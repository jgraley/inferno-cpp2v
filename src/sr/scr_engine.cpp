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
                       const SCREngine *master ) :
    algo( algo_ ),
    master_ptr( nullptr ),
    master_plinks( master_plinks_ )
{
    INDENT("}");
    TRACE(*this)(" planning part one\n");
    ASSERT(!master_ptr)("Calling configure on already-configured ")(*this);
    //TRACE("Entering SCREngine::Configure on ")(*this)("\n");
    overall_master_ptr = overall_master;
    master_ptr = master;
    
    ASSERT( cp )("Compare pattern must always be provided\n");
    ASSERT( cp==rp ); // Should have managed to reduce to a single pattern by now
    root_pattern = cp; 
    root_agent = Agent::AsAgent(root_pattern);
    // For closure under full arrowhead model, we need a link to root
    root_plink = PatternLink::CreateDistinct( root_pattern );   
            
    CategoriseSubs( master_plinks, in_progress_agent_phases );    

    // This recurses SCR engine planning
    CreateMyEngines( in_progress_agent_phases );    
}

    
void SCREngine::Plan::PlanningStageTwo(const CompareReplace::AgentPhases &final_agent_phases_)
{
    INDENT("}");
    TRACE(*this)(" planning part two\n");
    final_agent_phases = final_agent_phases_;
    
    // Recurse into subordinate SCREngines
    for( pair< RequiresSubordinateSCREngine *, shared_ptr<SCREngine> > p : my_engines )
        p.second->PlanningStageTwo(final_agent_phases);                                      

    // Configure agents on the way out
    ConfigureAgents();
}


void SCREngine::Plan::PlanningStageThree()
{
    INDENT("}");
    TRACE(*this)(" planning part three\n");
    
    // Recurse into subordinate SCREngines
    for( pair< RequiresSubordinateSCREngine *, shared_ptr<SCREngine> > p : my_engines )
        p.second->PlanningStageThree();                                      

    // Make and-rule engines on the way out - by now, hopefully all
    // the agents this and-rule engine sees have been configured.
    and_rule_engine = make_shared<AndRuleEngine>(root_plink, master_plinks, algo);
} 


void SCREngine::Plan::CategoriseSubs( const unordered_set<PatternLink> &master_plinks, 
                                      CompareReplace::AgentPhases &in_progress_agent_phases )
{
    // Walkers for compare and replace patterns that do not recurse beyond slaves (except via "through")
    // So that the compare and replace subtrees of slaves are "obsucured" and not visible. Determine 
    // compare and replace sets separately.
    unordered_set<PatternLink> visible_plinks_compare, visible_plinks_replace;
    WalkVisible( visible_plinks_compare, root_plink, Agent::COMPARE_PATH );
    WalkVisible( visible_plinks_replace, root_plink, Agent::REPLACE_PATH );
    
    // Determine all the agents we can see (can only see though slave "through", 
    // not into the slave's pattern)
    unordered_set<PatternLink> visible_plinks = UnionOf( visible_plinks_compare, visible_plinks_replace );
    
    for( PatternLink plink : visible_plinks )
    {
        unsigned int phase = (unsigned int)in_progress_agent_phases[plink.GetChildAgent()];
        if( visible_plinks_compare.count(plink) == 0 )
            phase |= Agent::IN_REPLACE_ONLY;
        else if( visible_plinks_replace.count(plink) == 0 )
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

    my_agents.clear();
    for( PatternLink plink : my_plinks )
        my_agents.insert( plink.GetChildAgent() );

    // Determine who our slaves are
    my_agents_needing_engines.clear();
    my_overlay_starter_engines.clear();
    for( PatternLink plink : my_plinks )
    {
        Agent *a = plink.GetChildAgent();
        if( auto ae = dynamic_cast<RequiresSubordinateSCREngine *>(a) )
            my_agents_needing_engines.insert( ae );
        if( auto ao = dynamic_cast<StartsOverlay *>(a) )
            my_overlay_starter_engines.insert( ao );
    }
}


void SCREngine::Plan::WalkVisible( unordered_set<PatternLink> &visible, 
                                   PatternLink base_plink, 
                                   Agent::Path path ) const
{
    visible.insert( base_plink );    
    list<PatternLink> plinks = base_plink.GetChildAgent()->GetVisibleChildren(path); 
    for( PatternLink plink : plinks )
        WalkVisible( visible, plink, path );    
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
        ae->ConfigureMyEngine( &*my_engines.at(ae) );
    }        
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


string SCREngine::Plan::GetTrace() const 
{
    return algo->GetName() + "::Plan" + algo->GetSerialString();
}


const CompareReplace * SCREngine::GetOverallMaster() const
{
    return plan.overall_master_ptr;
}


void SCREngine::SetStopAfter( vector<int> ssa, int d )
{
    stop_after = ssa;
    depth = d;
}


TreePtr<Node> SCREngine::Replace( const CouplingKeysMap *master_keys )
{
    INDENT("R");
        
    replace_keys = UnionOfSolo( *master_keys,
                                plan.and_rule_engine->GetCouplingKeys() );
    keys_available = true;

    for( StartsOverlay *ao : plan.my_overlay_starter_engines )
        ao->StartKeyForOverlay();
  
    // Now replace according to the couplings
    TreePtr<Node> rnode = plan.root_agent->BuildReplace();
    
    keys_available = false;
    replace_keys.clear();
    
    // Need a duplicate here in case we're a slave replacing an identifier
    // with a non-identifier. Otherwise our subtree would look fine, but 
    // global X tree would be incorrect (multiple links to non-identifier)
    // and that would break knowledge building. See #217
    return plan.root_agent->DuplicateSubtree(rnode);
}


void SCREngine::RecurseInto( SCREngine *slave_engine, 
                             TreePtr<Node> *p_root_xnode ) const
{
    ASSERT( keys_available );
    
    // Run the slave engine        
    slave_engine->RepeatingCompareReplace( p_root_xnode, &replace_keys );
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
           
    knowledge.Clear();

    TRACE("Now replacing\n");
    *p_root_xnode = Replace(master_keys);
    
    // Clear out all the replace keys (the ones inside the agents) now that replace is done
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
    TRACE("begin RCR\n");
        
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
            TRACE("OK\n");
            return i; // when the compare fails, we're done
        }
        if( stop )
        {
            TRACE("Stopping after hit %d\n", stop_after[depth]);
            TRACE("OK\n");
            return i;
        }    
    }
    
    TRACE("Over %d reps\n", repetitions); 
    ASSERT(!rep_error)
          ("Still getting matches after %d repetitions, may be repeating forever.\n"
           "Try using -rn%d to suppress this error\n", repetitions, repetitions);
       
    return repetitions;
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


void SCREngine::SetReplaceKey( const Agent *agent, CouplingKey key ) const
{
    ASSERT( keys_available );
    
    ASSERT(key);
    if( plan.final_agent_phases.at(agent) != IN_COMPARE_ONLY )
        ASSERT( key.IsFinal() )(*this)(" trying to key with non-final ")(key)("\n"); 

    InsertSolo( replace_keys, make_pair(agent, key) );
}


CouplingKey SCREngine::GetReplaceKey( const Agent *agent ) const
{
    ASSERT( keys_available );
    if( replace_keys.count(agent) == 1 )
        return replace_keys.at(agent);
    else
        return CouplingKey();
}


void SCREngine::CopyReplaceKey( const Agent *dest_agent, const Agent *src_agent ) const
{
    ASSERT( keys_available );
    ASSERT( replace_keys.count(src_agent) == 1 );
    InsertSolo( replace_keys, make_pair(dest_agent, replace_keys.at(src_agent)) );
}
