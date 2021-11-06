#include "scr_engine.hpp"
#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "agents/standard_agent.hpp"
#include "agents/delta_agent.hpp"
#include "agents/search_container_agent.hpp"
#include "common/common.hpp"
#include "agents/agent.hpp"
#include "and_rule_engine.hpp"
#include "link.hpp"
#include "coupling.hpp"
#include "equivalence.hpp"

#include <list>

#define ENABLE_UNIQUIFY_DOMAIN_EXTENSION

//#define RUN_SLAVES_LATER

using namespace SR;
using namespace std;

int SCREngine::repetitions;
bool SCREngine::rep_error;


// The master_plinks argument is a set of plinks to agents that we should not
// configure because they were already configured by a master, and masters take 
// higher priority for configuration (so when an agent is reached from multiple
// engines, it's the most masterish one that "owns" it).
SCREngine::SCREngine( const CompareReplace *overall_master,
                      CompareReplace::AgentPhases &in_progress_agent_phases,
                      TreePtr<Node> cp,
                      TreePtr<Node> rp,
                      const unordered_set<PatternLink> &master_plinks,
                      const SCREngine *master ) :
    SerialNumber( false ),
    plan(this, overall_master, in_progress_agent_phases, cp, rp, master_plinks, master),
    depth( 0 )
{
}

    
SCREngine::Plan::Plan( SCREngine *algo_,
                       const CompareReplace *overall_master,
                       CompareReplace::AgentPhases &in_progress_agent_phases,
                       TreePtr<Node> cp,
                       TreePtr<Node> rp,
                       const unordered_set<PatternLink> &master_plinks_,
                       const SCREngine *master ) : // Note: Is planning stage one
    algo( algo_ ),
    master_ptr( nullptr ),
    master_plinks( master_plinks_ )
{
    INDENT("}");
    TRACE("Planning stage one\n");
    ASSERT(!master_ptr)("Calling configure on already-configured");
    overall_master_ptr = overall_master;
    master_ptr = master;
    
    ASSERT( cp )("Compare pattern must always be provided\n");
    ASSERT( cp==rp ); // Should have managed to reduce to a single pattern by now
    root_pattern = cp; 
    root_agent = Agent::AsAgent(root_pattern);
    // For closure under full arrowhead model, we need a link to root
    root_plink = PatternLink::CreateDistinct( root_pattern );   
            
    CategoriseAgents( master_plinks, in_progress_agent_phases );    

    // This recurses SCR engine planning stage 1
    CreateMyEngines( in_progress_agent_phases );    
}

    
void SCREngine::Plan::CategoriseAgents( const unordered_set<PatternLink> &master_plinks, 
                                        CompareReplace::AgentPhases &in_progress_agent_phases )
{
    // Walkers for compare and replace patterns that do not recurse beyond slaves (except via "through")
    // So that the compare and replace subtrees of slaves are "obsucured" and not visible. Determine 
    // compare and replace sets separately.
    unordered_set<PatternLink> visible_compare_plinks, visible_replace_plinks;
    list<PatternLink> visible_replace_plinks_postorder;
    WalkVisible( visible_compare_plinks, nullptr, root_plink, Agent::COMPARE_PATH );
    WalkVisible( visible_replace_plinks, &visible_replace_plinks_postorder, root_plink, Agent::REPLACE_PATH );
    
    // Determine all the agents we can see (can only see though slave "through", 
    // not into the slave's pattern)
    unordered_set<PatternLink> visible_plinks = UnionOf( visible_compare_plinks, visible_replace_plinks );
    
    for( PatternLink plink : visible_plinks )
    {
        unsigned int phase = (unsigned int)in_progress_agent_phases[plink.GetChildAgent()];
        if( visible_compare_plinks.count(plink) == 0 )
            phase |= Agent::IN_REPLACE_ONLY;
        else if( visible_replace_plinks.count(plink) == 0 )
            phase |= Agent::IN_COMPARE_ONLY;
        else
            phase |= Agent::IN_COMPARE_AND_REPLACE;
        in_progress_agent_phases[plink.GetChildAgent()] = (Agent::Phase)phase;
    }
        
    master_agents.clear();
    for( PatternLink plink : master_plinks )
        master_agents.insert( plink.GetChildAgent() );

    // Determine which ones really belong to us (some might be visible from one of our masters, 
    // in which case it should be in the supplied set.      
    my_plinks.clear();
    //my_plinks = DifferenceOf( visible_plinks, master_plinks );
    for( PatternLink plink : visible_plinks )
        if( master_agents.count( plink.GetChildAgent() ) == 0 ) // exclude by agent
            my_plinks.insert( plink );

    // Need the replace plinks in the same order that BuildReplace() walks the tree
    TRACE("Making ordered replace plinks\n");
    for( PatternLink plink : visible_replace_plinks_postorder )
        if( master_plinks.count(plink) == 0 ) // exclude by plink
        {
            TRACEC(plink)("\n");
            my_replace_plinks_postorder.push_back( plink );
        }

    my_agents.clear();
    for( PatternLink plink : my_plinks )
        my_agents.insert( plink.GetChildAgent() );

    // Determine who our slaves are
    my_agents_needing_engines.clear();
    my_overlay_starter_engines.clear();
    for( Agent *a : my_agents )
    {
        if( auto ae = dynamic_cast<RequiresSubordinateSCREngine *>(a) )
            my_agents_needing_engines.insert( ae );
        if( auto ao = dynamic_cast<StartsOverlay *>(a) )
            my_overlay_starter_engines.insert( ao );
    }
}


void SCREngine::Plan::WalkVisible( unordered_set<PatternLink> &visible,
                                   list<PatternLink> *visible_postorder, // optional
                                   PatternLink base_plink, 
                                   Agent::Path path ) const
{
    visible.insert( base_plink );    
    
    // Gee, I sure hope recovers children in the same order as BuildReplaceImpl()    
    list<PatternLink> plinks = base_plink.GetChildAgent()->GetVisibleChildren(path); 
    
    for( PatternLink plink : plinks )
        WalkVisible( visible, visible_postorder, plink, path );    

    if( visible_postorder )
        visible_postorder->push_back( base_plink );
}


void SCREngine::Plan::CreateMyEngines( CompareReplace::AgentPhases &in_progress_agent_phases )
{
    // Determine which agents our slaves should not configure
    unordered_set<PatternLink> surrounding_plinks = UnionOf( master_plinks, my_plinks ); 
            
    for( RequiresSubordinateSCREngine *ae : my_agents_needing_engines )
    {
        my_engines[ae] = make_shared<SCREngine>( overall_master_ptr, 
                                                 in_progress_agent_phases,
                                                 ae->GetSearchPattern(),
                                                 ae->GetReplacePattern(),
                                                 surrounding_plinks, 
                                                 algo );       
    }        
}


void SCREngine::Plan::PlanningStageTwo(const CompareReplace::AgentPhases &final_agent_phases_)
{
    INDENT("}");
    TRACE("Planning stage two\n");
    final_agent_phases = final_agent_phases_;
    
    // Recurse into subordinate SCREngines
    for( pair< RequiresSubordinateSCREngine *, shared_ptr<SCREngine> > p : my_engines )
        p.second->PlanningStageTwo(final_agent_phases);                                      

    // Configure agents on the way out
    ConfigureAgents();
}


void SCREngine::Plan::ConfigureAgents()
{
    // Give agents pointers to here and our coupling keys
    for( Agent *agent : my_agents )
    {        
        agent->SCRConfigure( algo,
                             final_agent_phases.at(agent) );                                                 
    }

/* Not sure if needed
    for( PatternLink plink : my_plinks )
    {
        Agent *agent = plink.GetChildAgent();
        // Replace-only nodes are self-keying
        if( in_progress_agent_phases.at(plink.GetChildAgent()) == Agent::IN_REPLACE_ONLY )
            agent->AndRuleConfigure( nullptr, PatternLink(), {plink} );
    }    
*/
}


void SCREngine::Plan::PlanningStageThree(unordered_set<PatternLink> master_keyers)
{    
    INDENT("}");
    // Stage three mirrors the sequence of events taken at run time i.e.
    // COMPARE, REPLACE, RECURSE, RECURSE
    TRACE("Planning stage three\n");
 
    all_keyer_plinks = master_keyers;
    
    // COMPARE
    PlanCompare();
    
    // REPLACE
    PlanReplace();
    
    // RECURSE RECURSE
    // Recurse into subordinate SCREngines
    for( pair< RequiresSubordinateSCREngine *, shared_ptr<SCREngine> > p : my_engines )
        p.second->PlanningStageThree(all_keyer_plinks);                                      
} 


void SCREngine::Plan::PlanCompare()
{
    // All agents this AndRuleEngine see must have been configured 
    and_rule_engine = make_shared<AndRuleEngine>(root_plink, master_plinks, algo);
    
    all_keyer_plinks = UnionOfSolo( all_keyer_plinks, 
                                    and_rule_engine->GetKeyerPatternLinks() );
}

void SCREngine::Plan::PlanReplace()
{
    // Plan the keyers for couplings 
    for( StartsOverlay *ao : my_overlay_starter_engines )
        ao->StartPlanOverlay();        
        
    for( PatternLink plink : my_replace_plinks_postorder )
        if( plink.GetChildAgent()->PlanReplaceKeying(plink, all_keyer_plinks) )
            all_keyer_plinks.insert( plink );
}


string SCREngine::Plan::GetTrace() const 
{
    return algo->GetName() + ".plan" + algo->GetSerialString();
}

    
void SCREngine::PostSlaveFixup( TreePtr<Node> through_subtree, TreePtr<Node> new_subtree ) const 
{
    INDENT("F");
    
    // Fix up for remaining slaves if required.
    for( auto &p : slave_though_subtrees ) // ref important - we're modifying!
    {
        TRACE("Trying fixup of ")(through_subtree)(": ")(p.first)(", ")(p.second)("\n");
        if( p.second == through_subtree )
        {
            TRACEC("Fixup for ")(*(Agent *)p.first)(": ")(through_subtree)(" becomes ")(new_subtree)("\n");
            p.second = new_subtree;
        }
    }
    
    if( plan.master_ptr ) // recurse down to overall master
        plan.master_ptr->PostSlaveFixup( through_subtree, new_subtree );
}


void SCREngine::RunSlave( RequiresSubordinateSCREngine *slave_agent, TreePtr<Node> *p_root_x )
{         
    shared_ptr<SCREngine> slave_engine = plan.my_engines.at(slave_agent);
    ASSERT( slave_engine );
   
    TRACE("Going to run slave on ")(*slave_engine)
         (" agent ")(slave_agent)
         (" and slave_though_subtrees are\n")(slave_though_subtrees)("\n");
   
    // Recall the base node of the subtree under through (after master replace)
    TreePtr<Node> through_subtree = slave_though_subtrees.at(slave_agent);
    slave_though_subtrees.erase(slave_agent); // not needed any more
    ASSERT( through_subtree );
    
    // Run the slaves engine on this subtree
    TreePtr<Node> new_subtree = through_subtree;
    int hits = slave_engine->RepeatingCompareReplace( &new_subtree, &replace_keys );
    if( !hits )
        return;
        
    TRACE("Slave ")(slave_engine)(" succeeded, need to implant new subtree ")(new_subtree)("\n");
    // Special case when slave is at root of my SCR region: switch the whole tree
    if( through_subtree == *p_root_x )
    {
        TRACEC("Implanting at root over ")(*p_root_x)("\n");
        *p_root_x = new_subtree;
    }
    else
    {
        // Search for links to the though subtree and stitch in the new one
        int hits = 0;
        Walk e( *p_root_x ); 
        for( Walk::iterator wit=e.begin(); wit!=e.end(); ++wit )
        {
            if( *wit == through_subtree ) // found the though subtree
            {            
                // Get the pointer that points to the though subtree
                const TreePtrInterface *px = wit.GetNodePointerInParent();    
                // Update it to point to the new subtree
                if( px ) // ps is NULL at root 
                {
                    TRACEC("Implanting at non-root over ")(*const_cast<TreePtrInterface *>(px))("\n");
                    *const_cast<TreePtrInterface *>(px) = new_subtree;
                    hits++;
                }
            }
        }
        ASSERT( hits==1 )("Trying to implant ")(new_subtree)(" into ")(*p_root_x)(" at ")(through_subtree)(" got %d hits, expecting one", hits);
    }

    PostSlaveFixup( through_subtree, new_subtree );
}


TreePtr<Node> SCREngine::Replace( const CouplingKeysMap *master_keys )
{
    INDENT("R");
        
    replace_keys = UnionOfSolo( *master_keys,
                                plan.and_rule_engine->GetCouplingKeys() );
    my_keyer_plinks_measured.clear();
    slave_though_subtrees.clear();
    keys_available = true;

    // Now replace according to the couplings
    TRACE("Now replacing, root agent=")(plan.root_agent)("\n");
    TreePtr<Node> new_root_x;
    new_root_x = plan.root_agent->BuildReplace(plan.root_plink);
    TRACE("Replace done\n");
    
    if( ReadArgs::new_slave_sequence )
    {
        for( PatternLink plink : plan.my_replace_plinks_postorder )
        {
            if( auto slave_agent = dynamic_cast<RequiresSubordinateSCREngine *>(plink.GetChildAgent()) )
            {
                TRACE("Running slave ")(*(Agent *)slave_agent)(" root x=")(new_root_x)("\n");
                RunSlave(slave_agent, &new_root_x);
            }
        }        
    }
    TRACE("Slaves done\n");
    
    keys_available = false;
    replace_keys.clear();
    //FTRACE(my_keyer_plinks_measured);
    
    // Need a duplicate here in case we're a slave replacing an identifier
    // with a non-identifier. Otherwise our subtree would look fine, but 
    // global X tree would be incorrect (multiple links to non-identifier)
    // and that would break knowledge building. See #217
    {Tracer::RAIIEnable silencer( false );return plan.root_agent->DuplicateSubtree(new_root_x);}
}


void SCREngine::SingleCompareReplace( TreePtr<Node> *p_root_xnode,
                                      const CouplingKeysMap *master_keys ) 
{
    INDENT(">");

    // Cannonicalise could change root
    XLink root_xlink = XLink::CreateDistinct(*p_root_xnode);

    // Global domain of possible xlink values
    knowledge.Build( plan.root_plink, root_xlink );

    TRACE("Begin search\n");
    // Note: comparing doesn't require double pointer any more, but
    // replace does so it can change the root node.
    plan.and_rule_engine->Compare( root_xlink, master_keys, &knowledge );
    TRACE("Search got a match\n");
           
    knowledge.Clear();

    *p_root_xnode = Replace(master_keys);
    
    // Clear out anything cached in agents now that replace is done
    FOREACH( Agent *a, plan.my_agents )
        a->Reset();
    plan.and_rule_engine->ClearCouplingKeys(); // save memory
}


// Perform search and replace on supplied program based
// on supplied patterns and couplings. Does search and replace
// operations repeatedly until there are no more matches. Returns how
// many hits we got.
int SCREngine::RepeatingCompareReplace( TreePtr<Node> *p_root_xnode,
                                        const CouplingKeysMap *master_keys )
{
    INDENT("}");
    TRACE("Begin RCR\n");
        
    ASSERT( plan.root_pattern )("SCREngine object was not configured before invocation.\n"
                                "Either call Configure() or supply pattern arguments to constructor.\n"
                                "Thank you for taking the time to read this message.\n");
    
    for(int i=0; i<repetitions; i++) 
    {
        bool stop = depth < stop_after.size() && stop_after[depth]==i;
        if( stop )
            for( const pair< RequiresSubordinateSCREngine * const, shared_ptr<SCREngine> > &p : plan.my_engines )
                p.second->SetStopAfter(stop_after, depth+1); // and propagate the remaining ones
        try
        {
            SingleCompareReplace( p_root_xnode, master_keys );
        }
        catch( ::Mismatch )
        {
            if( depth < stop_after.size() )
                ASSERT(stop_after[depth]<i)("Stop requested after hit that doesn't happen, there are only %d", i);
            TRACE("Mismatched; stopping\n");
            return i; // when the compare fails, we're done
        }
        if( stop )
        {
            TRACE("Stopping as requested after hit %d\n", stop_after[depth]);
            TRACEC("OK\n");
            return i;
        }    
    }
    
    TRACE("Over the limit of %d reps\n", repetitions); 
    ASSERT(!rep_error)
          ("Still getting matches after %d repetitions, may be repeating forever.\n"
           "Try using -rn%d to suppress this error\n", repetitions, repetitions);
       
    return repetitions;
}


void SCREngine::SetStopAfter( vector<int> ssa, int d )
{
    stop_after = ssa;
    depth = d;
}


void SCREngine::SetMaxReps( int n, bool e ) 
{ 
    repetitions = n; 
    rep_error = e; 
}


list<const AndRuleEngine *> SCREngine::GetAndRuleEngines() const
{
	list<const AndRuleEngine *> engines;
	engines = plan.and_rule_engine->GetAndRuleEngines();
	for( auto p : plan.my_engines )
    {
		//engines.push_back(nullptr);
		engines = engines + p.second->GetAndRuleEngines();
	}
	return engines;
}


list<const SCREngine *> SCREngine::GetSCREngines() const
{
	list<const SCREngine *> engines;
	engines.push_back( this );
	for( auto p : plan.my_engines )
		engines = engines + p.second->GetSCREngines();
	return engines;
}


void SCREngine::GenerateGraphRegions( Graph &graph ) const
{
	plan.and_rule_engine->GenerateGraphRegions(graph, GetGraphId());
	for( auto p : plan.my_engines )
		p.second->GenerateGraphRegions(graph);	
}


void SCREngine::RecurseInto( RequiresSubordinateSCREngine *slave_agent,
                             TreePtr<Node> *p_slave_through_subtree ) const
{
    ASSERT( keys_available );
    //Tracer::RAIIEnable silencer( true );    
    
    if( ReadArgs::new_slave_sequence )
    {
        // p_root_xnode cannot be stored: it points to a local of our caller
        TreePtr<Node> slave_through_subtree = *p_slave_through_subtree; 
        
        InsertSolo( slave_though_subtrees, make_pair( slave_agent, slave_through_subtree ) );
    }
    else    
    {
        shared_ptr<SCREngine> slave_engine = plan.my_engines.at(slave_agent);
        ASSERT( slave_engine );
        
        // Run the slave engine        
        slave_engine->RepeatingCompareReplace( p_slave_through_subtree, &replace_keys );
    }
}


void SCREngine::SetReplaceKey( LocatedLink keyer_link, KeyProducer place ) const
{
    ASSERT( keys_available );
    Agent *keyer_agent = keyer_link.GetChildAgent();
    TreePtr<Node> keyer_x = keyer_link.GetChildX();
    
    ASSERT(keyer_x);
    if( plan.final_agent_phases.at(keyer_agent) != IN_COMPARE_ONLY )
        ASSERT( keyer_x->IsFinal() )("Trying to key with non-final ")(keyer_x)("\n"); 

    CouplingKey key( keyer_link, place, nullptr, this );
    InsertSolo( replace_keys, make_pair(keyer_agent, key) );
    InsertSolo( my_keyer_plinks_measured, (PatternLink)keyer_link );
}


CouplingKey SCREngine::GetReplaceKey( const Agent *agent ) const
{
    ASSERT( keys_available );
    if( replace_keys.count(agent) == 1 )
        return replace_keys.at(agent);
    else
        return CouplingKey();
}


bool SCREngine::IsKeyedByAndRuleEngine( Agent *agent ) const
{
    ASSERT( plan.and_rule_engine );
    return plan.and_rule_engine->GetKeyedAgents().count( agent );
}


bool SCREngine::IsKeyed( PatternLink plink ) const
{
    return plan.all_keyer_plinks.count(plink)==1;
}


bool SCREngine::IsKeyed( Agent *agent ) const
{
    for( PatternLink keyer_plink : plan.all_keyer_plinks )
    {
        Agent *keyer_agent = keyer_plink.GetChildAgent();
        if( keyer_agent == agent )
            return true; 
    }
    return false;
}


const CompareReplace * SCREngine::GetOverallMaster() const
{
    return plan.overall_master_ptr;
}


XLink SCREngine::UniquifyDomainExtension( XLink xlink ) const
{
    // Don't worry about generated nodes that are already in 
    // the X tree (they had to have been found there after a
    // search). 
    if( knowledge.domain.count(xlink) > 0 )
        return xlink;
        
    return knowledge.domain_extension_classes->Uniquify( xlink ); 
}


string SCREngine::GetTrace() const
{
    string s = Traceable::GetName() + GetSerialString();
    return s;
}


string SCREngine::GetGraphId() const
{
	return "SCR"+GetSerialString();
}
