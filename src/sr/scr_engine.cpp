#include "scr_engine.hpp"
#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "agents/standard_agent.hpp"
#include "agents/overlay_agent.hpp"
#include "agents/search_container_agent.hpp"
#include "common/common.hpp"
#include "agents/agent.hpp"
#include "and_rule_engine.hpp"
#include "link.hpp"
#include "equivalence.hpp"

#include <list>

//#define CANNONICALISE

#define BUILD_THE_KNOWLEDGE

#define ENABLE_UNIQUIFY_DOMAIN_EXTENSION

using namespace SR;
using namespace std;

int SCREngine::repetitions;
bool SCREngine::rep_error;


// The master_plinks argument is a set of plinks to agents that we should not
// configure because they were already configured by a master, and masters take 
// higher priority for configuration (so when an agent is reached from multiple
// engines, it's the most masterish one that "owns" it).
SCREngine::SCREngine( bool is_search_,
                      const CompareReplace *overall_master,
                      CompareReplace::AgentPhases &agent_phases,
                      TreePtr<Node> cp,
                      TreePtr<Node> rp,
                      const unordered_set<PatternLink> &master_plinks,
                      const SCREngine *master ) :
    SerialNumber( false ),
    plan(this, is_search_, overall_master, agent_phases, cp, rp, master_plinks, master),
    depth( 0 )
{
}

    
SCREngine::Plan::Plan( SCREngine *algo_,
                       bool is_search_,
                       const CompareReplace *overall_master,
                       CompareReplace::AgentPhases &agent_phases,
                       TreePtr<Node> cp,
                       TreePtr<Node> rp,
                       const unordered_set<PatternLink> &master_plinks_,
                       const SCREngine *master ) :
    algo( algo_ ),
    is_search( is_search_ ),
    master_ptr( nullptr ),
    master_plinks( master_plinks_ )
{
    INDENT("}");
    TRACE(*this)(" planning part one\n");
    ASSERT(!master_ptr)("Calling configure on already-configured ")(*this);
    //TRACE("Entering SCREngine::Configure on ")(*this)("\n");
    overall_master_ptr = overall_master;
    master_ptr = master;
    
    InstallRootAgents(cp, rp);
            
    set<RequiresSubordinateSCREngine *> my_agents_needing_engines;   
    CategoriseSubs( master_plinks, my_agents_needing_engines, agent_phases );    

    // This recurses SCR engine planning
    CreateMyEngines( my_agents_needing_engines, agent_phases );    
}

    
void SCREngine::Plan::PlanningStageTwo(const CompareReplace::AgentPhases &agent_phases)
{
    INDENT("}");
    TRACE(*this)(" planning part two\n");
    
    // Recurse into subordinate SCREngines
    for( pair< RequiresSubordinateSCREngine *, shared_ptr<SCREngine> > p : my_engines )
        p.second->PlanningStageTwo(agent_phases);                                      

    // Configure agents on the way out
    ConfigureAgents(agent_phases);

    // Make and-rule engines on the way out - by now, hopefully all
    // the agents this and-rule engine sees have been configured.
    and_rule_engine = make_shared<AndRuleEngine>(root_plink, master_plinks, algo);
} 


void SCREngine::Plan::InstallRootAgents( TreePtr<Node> cp,
						     		     TreePtr<Node> rp )
{
    ASSERT( cp )("Compare pattern must always be provided\n");
    
    // If only a search pattern is supplied, make the replace pattern the same
    // so they couple and then an overlay node can split them apart again.
    if( !rp )
    {
        rp = cp;
    }

    if( rp != cp ) 
    {
        // Classic compare and replace with separate replace pattern, we can use
        // an Overlay node to overwrite the replace pattern at replace time.
        MakePatternPtr< Overlay<Node> > overlay; 
        overlay->through = cp;
        overlay->overlay = rp;
        cp = rp = overlay; 
    }

    if( is_search )
    {
        // Obtain search and replace semaintics from a compare and replace engine
        // by inserting a stuff node at root
        ASSERT( cp==rp );
        MakePatternPtr< Stuff<Node> > stuff;        
        stuff->terminus = cp;
        cp = rp = stuff;   
    }
    
    ASSERT( cp==rp ); // Should have managed to reduce to a single pattern by now
    root_pattern = cp; 
    root_agent = Agent::AsAgent(root_pattern);
    // For closure under full arrowhead model, we need a link to root
    root_plink = PatternLink::CreateDistinct( root_pattern );   
}
    

void SCREngine::Plan::CategoriseSubs( const unordered_set<PatternLink> &master_plinks, 
                                      set<RequiresSubordinateSCREngine *> &my_agents_needing_engines,
                                      CompareReplace::AgentPhases &agent_phases )
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
        unsigned int phase = (unsigned int)agent_phases[plink.GetChildAgent()];
        if( visible_plinks_compare.count(plink) == 0 )
            phase |= Agent::IN_REPLACE_ONLY;
        else if( visible_plinks_replace.count(plink) == 0 )
            phase |= Agent::IN_COMPARE_ONLY;
        else
            phase |= Agent::IN_COMPARE_AND_REPLACE;
        agent_phases[plink.GetChildAgent()] = (Agent::Phase)phase;
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
    for( PatternLink plink : my_plinks )
    {
        Agent *a = plink.GetChildAgent();
        if( auto ae = dynamic_cast<RequiresSubordinateSCREngine *>(a) )
            my_agents_needing_engines.insert( ae );
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


void SCREngine::Plan::CreateMyEngines( const set<RequiresSubordinateSCREngine *> &my_agents_needing_engines,
                                       CompareReplace::AgentPhases &agent_phases )
{
    // Determine which agents our slaves should not configure
    unordered_set<PatternLink> surrounding_plinks = UnionOf( master_plinks, my_plinks ); 
            
    for( RequiresSubordinateSCREngine *ae : my_agents_needing_engines )
    {
        my_engines[ae] = make_shared<SCREngine>( ae->IsSearch(),
                                                 overall_master_ptr, 
                                                 agent_phases,
                                                 ae->GetSearchPattern(),
                                                 ae->GetReplacePattern(),
                                                 surrounding_plinks, 
                                                 algo );       
        ae->ConfigureMyEngine( &*my_engines.at(ae) );
    }        
}


void SCREngine::Plan::ConfigureAgents(const CompareReplace::AgentPhases &agent_phases)
{
    // Give agents pointers to here and our coupling keys
    for( Agent *agent : my_agents )
    {        
        agent->AgentConfigure( agent_phases.at(agent),
                               algo );                                                 
    }
    
    for( PatternLink plink : my_plinks )
    {
        Agent *agent = plink.GetChildAgent();
        // Replace-only nodes are self-keying
        if( agent_phases.at(plink.GetChildAgent()) == Agent::IN_REPLACE_ONLY )
            agent->ConfigureParents( PatternLink(), {plink}, GetTrace() );
    }    
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


void SCREngine::KeyReplaceNodes( const CouplingKeysMap *coupling_keys ) const
{
    INDENT("K");   
        
    TRACE("My agents coupling status:\n");
    FOREACH( Agent *a, plan.my_agents )
    {
        TRACEC(*a);
        bool keyed = ( coupling_keys->count( a ) > 0 );
        if( keyed )
            TRACEC(" is in coupling_keys: ")(coupling_keys->at( a ));
        else
            TRACEC(" is not in coupling_keys");
        CouplingKey self_coupled = a->GetKey();
        if( self_coupled )
            TRACEC(" and is self-coupled: ")(self_coupled)("\n");
        else
            TRACEC(" and is not self-coupled\n");
            
        if( keyed && !self_coupled )
            a->KeyReplace(coupling_keys);
    }
}


TreePtr<Node> SCREngine::Replace() const
{
    INDENT("R");
    
    // Now replace according to the couplings
    TreePtr<Node> rnode = plan.root_agent->BuildReplace();
    
    // Need a duplicate here in case we're a slave replacing an identifier
    // with a non-identifier. Otherwise our subtree would look fine, but 
    // global X tree would be incorrect (multiple links to non-identifier)
    // and that would break knowledge building. See #217
    return plan.root_agent->DuplicateSubtree(rnode);
}


void SCREngine::GatherCouplings( CouplingKeysMap *coupling_keys ) const
{
	// Get couplings from agents into the supplied map if not there already
    FOREACH( Agent *a, plan.my_agents )
        if( a->GetKey() && !coupling_keys->count( a ) > 0 )
            (*coupling_keys)[a] = a->GetKey();	
}


XLink SCREngine::UniquifyDomainExtension( XLink xlink ) const
{
    // Don't worry about generated nodes that are already in 
    // the X tree (they had to have been found there after a
    // search). 
    if( knowledge.domain.count(xlink) > 0 )
        return xlink;
        
#ifdef ENABLE_UNIQUIFY_DOMAIN_EXTENSION    
    return knowledge.domain_extension_classes->Uniquify( xlink );
#else
    return xlink;
#endif    
}


void SCREngine::SingleCompareReplace( TreePtr<Node> *p_root_xnode,
                                      const CouplingKeysMap *master_keys ) 
{
    INDENT(">");

#ifdef CANNONICALISE
    Cannonicaliser cz;
    TRACE("Cannonicalise\n");
    cz( *p_root_xnode, p_root_xnode );
#endif

    // Cannonicalise could change root
    XLink root_xlink = XLink::CreateDistinct(*p_root_xnode);

    // Global domain of possible xlink values
#ifdef BUILD_THE_KNOWLEDGE
    knowledge.Build( plan.root_plink, root_xlink );
#endif

    TRACE("Begin search\n");
    // Note: comparing doesn't require double pointer any more, but
    // replace does so it can change the root node.
    plan.and_rule_engine->Compare( root_xlink, master_keys, &knowledge );
           
#ifdef BUILD_THE_KNOWLEDGE
    knowledge.Clear();
#endif

    TRACE("Search successful, now keying replace nodes\n");
    plan.and_rule_engine->EnsureChoicesHaveIterators(); // Replace can't deal with hard BEGINs
    KeyReplaceNodes( &plan.and_rule_engine->GetCouplingKeys() );

    if( !plan.my_engines.empty() )
    {
		CouplingKeysMap coupling_keys = UnionOfSolo( *master_keys, 
                                                     plan.and_rule_engine->GetCouplingKeys() );    
		
        for( const pair< RequiresSubordinateSCREngine *, shared_ptr<SCREngine> > &p : plan.my_engines )
            p.first->SetMasterCouplingKeys( coupling_keys );
	}

    TRACE("Now replacing\n");
    *p_root_xnode = Replace();
    
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


Graphable::Block SCREngine::GetGraphBlockInfo( const LinkNamingFunction &lnf ) const
{
	TRACE(*this)(" GetGraphBlockInfo()\n");
    list<SubBlock> sub_blocks;
    if( ReadArgs::graph_trace )
    {
        // Actually much simpler in graph trace mode - just show the root node and plink
        sub_blocks.push_back( { "root", 
                                "",
                                true,
                                { { &plan.root_pattern,
                                    SOLID, 
                                    {},
                                    {plan.root_plink.GetShortName()} } } } );
        sub_blocks.push_back( { GetSerialString(), 
                                "", 
                                false, 
                                {} }  );
        return { false, GetName(), "", "", CONTROL, THROUGH, sub_blocks };
    }
    
    // TODO pretty sure this can "suck in" explicitly placed stuff and overlay 
    // nodes under the SR, CR or slave. These are obviously unnecessary, maybe I
    // should error on them?
    const TreePtrInterface *original_ptr = &plan.root_pattern;
    if( plan.is_search )
    {
        TreePtr< Stuff<Node> > stuff = DynamicTreePtrCast< Stuff<Node> >(*original_ptr);
        ASSERT( stuff );
        original_ptr = stuff->GetTerminus();
    }
    TreePtr< Overlay<Node> > overlay = DynamicTreePtrCast< Overlay<Node> >(*original_ptr);
    if( overlay )
    {        
        sub_blocks.push_back( { plan.is_search?"search":"compare", 
                                "",
                                false,
                                { { overlay->GetThrough(),
                                    SOLID, 
                                    {},
                                    {} } } } );    
        sub_blocks.push_back( { "replace", 
                                "",
                                false,
                                { { overlay->GetOverlay(),
                                    DASHED, 
                                    {},
                                    {} } } } );
    }
    else
    {
        sub_blocks.push_back( { plan.is_search?"search_replace":"compare_replace", 
                                "",
                                true,
                                { { original_ptr,
                                    SOLID, 
                                    {},
                                    {} } } } );
    }
    return { false, GetName(), "", "", CONTROL, THROUGH, sub_blocks };
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
		engines.push_back(nullptr);
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
