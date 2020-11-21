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


// The master_agents argument is a set of agents that we should not
// configure because they were already configured by a master, and masters take 
// higher priority for configuration (so when an agent is reached from multiple
// engines, it's the most masterish one that "owns" it).
SCREngine::SCREngine( bool is_search_,
                      const CompareReplace *overall_master,
                      TreePtr<Node> cp,
                      TreePtr<Node> rp,
                      const unordered_set<Agent *> &master_agents,
                      const SCREngine *master ) :
    plan(this, is_search_, overall_master, cp, rp, master_agents, master),
    depth( 0 )
{
}

    
SCREngine::Plan::Plan( SCREngine *algo_,
                       bool is_search_,
                       const CompareReplace *overall_master,
                       TreePtr<Node> cp,
                       TreePtr<Node> rp,
                       const unordered_set<Agent *> &master_agents,
                       const SCREngine *master ) :
    algo( algo_ ),
    is_search( is_search_ ),
    master_ptr( nullptr )
{
    TRACE(GetName());
    INDENT("P");
    ASSERT(!master_ptr)("Calling configure on already-configured ")(*this);
    //TRACE("Entering SCREngine::Configure on ")(*this)("\n");
    overall_master_ptr = overall_master;
    master_ptr = master;
    
    InstallRootAgents(cp, rp);
            
    // For closure under full arrowhead model, we need a link to root
    root_plink = PatternLink::CreateDistinct( root_pattern );
    
    set<AgentCommonNeedSCREngine *> my_agents_needing_engines;   
    CategoriseSubs( master_agents, my_agents_needing_engines );    
    CreateMyEngines( master_agents, my_agents_needing_engines );    
    ConfigureAgents();
    
    and_rule_engine = make_shared<AndRuleEngine>(root_plink, master_agents);
} 


void SCREngine::Plan::InstallRootAgents( TreePtr<Node> cp,
						     		     TreePtr<Node> rp )
{
    ASSERT( cp )("Compare pattern must always be provided\n");
    
    // If only a search pattern is supplied, make the replace pattern the same
    // so they couple and then an overlay node can split them apart again.
    if( !rp )
        rp = cp;

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
}
    

void SCREngine::Plan::CategoriseSubs( const unordered_set<Agent *> &master_agents, 
                                      set<AgentCommonNeedSCREngine *> &my_agents_needing_engines )
{
    // Walkers for compare and replace patterns that do not recurse beyond slaves (except via "through")
    // So that the compare and replace subtrees of slaves are "obsucured" and not visible
    VisibleWalk tp(root_pattern); 
    unordered_set<Agent *> visible_agents;
    for( VisibleWalk::iterator it = tp.begin(); it != tp.end(); ++it )
    {
        auto n = (TreePtr<Node>)*it;
        visible_agents.insert( Agent::AsAgent(n) );
    //    visible_plinks.insert( FromWalkIterator( it, PatternLink root )
    }
    
    // Determine which ones really belong to us (some might be visible from one of our masters, 
    // in which case it should be in the supplied set.        
    my_agents = make_shared< unordered_set<Agent *> >();
    *my_agents = DifferenceOf( visible_agents, master_agents );         

    // Determine who our slaves are
    FOREACH( Agent *a, *my_agents )
        if( AgentCommonNeedSCREngine *sa = dynamic_cast<AgentCommonNeedSCREngine *>(a) )
            my_agents_needing_engines.insert( sa );
}


void SCREngine::Plan::CreateMyEngines( const unordered_set<Agent *> &master_agents, 
                                       const set<AgentCommonNeedSCREngine *> &my_agents_needing_engines )
{
    // Determine which agents our slaves should not configure
    unordered_set<Agent *> surrounding_agents = UnionOf( master_agents, *my_agents ); 
            
    FOREACH( AgentCommonNeedSCREngine *a, my_agents_needing_engines )
    {
        my_engines[a] = make_shared<SCREngine>( a->IsSearch(),
                                                overall_master_ptr, 
                                                a->GetSearchPattern(),
                                                a->GetReplacePattern(),
                                                surrounding_agents, 
                                                algo );
                            
        //TRACE("Recursing to configure slave ")(*p.first)("\n");
        a->AgentConfigure( algo, 
                           &*my_engines.at(a) );
    }
        
}


void SCREngine::Plan::ConfigureAgents()
{
    // Give agents pointers to here and our coupling keys
    FOREACH( Agent *a, *my_agents )
    {        
        if( dynamic_cast<AgentCommonNeedSCREngine *>(a) )
            continue; // TODO yuck, should determine this in CategoriseSubs()
        //TRACE("Configuring agent ")(*a)("\n");
        a->AgentConfigure( algo );             
    }    
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


void SCREngine::GetGraphInfo( vector<string> *labels, 
                              vector< TreePtr<Node> > *blocks ) const
{
    // TODO pretty sure this can "suck in" explicitly placed stuff and overlay 
    // nodes under the SR, CR or slave. These are obviously unnecessary, maybe I
    // should error on them?
    TreePtr<Node> original_pattern = plan.root_pattern;
    if( plan.is_search )
    {
        TreePtr< Stuff<Node> > stuff = dynamic_pointer_cast< Stuff<Node> >(original_pattern);
        ASSERT( stuff );
        original_pattern = stuff->terminus;
    }
    TreePtr< Overlay<Node> > overlay = dynamic_pointer_cast< Overlay<Node> >(original_pattern);
    if( overlay )
    {        
        labels->push_back(plan.is_search?"search":"compare");    
        blocks->push_back(overlay->through);
        labels->push_back("replace");
        blocks->push_back(overlay->overlay);
    }
    else
    {
        labels->push_back(plan.is_search?"search_replace":"compare_replace");    
        blocks->push_back(original_pattern);
    }
}


void SCREngine::KeyReplaceNodes( const CouplingKeysMap *coupling_keys ) const
{
    INDENT("K");   
        
    FOREACH( Agent *a, *plan.my_agents )
    {
        TRACE(*a)(coupling_keys->count( a )?" is in coupling_keys ":" is not in coupling_keys")
             (a->GetKey()?" and is self-coupled\n":" and is not self-coupled\n");
        if( coupling_keys->count( a ) > 0 && !a->GetKey() )
            a->KeyReplace(coupling_keys);
    }
}


TreePtr<Node> SCREngine::Replace() const
{
    INDENT("R");
    
    // Now replace according to the couplings
    TreePtr<Node> rnode = plan.root_agent->BuildReplace();
    return plan.root_agent->DuplicateSubtree(rnode);
}


void SCREngine::GatherCouplings( CouplingKeysMap *coupling_keys ) const
{
	// Get couplings from agents into the supplied map if not there already
    FOREACH( Agent *a, *plan.my_agents )
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
           
    TRACE("Search successful, now keying replace nodes\n");
    plan.and_rule_engine->EnsureChoicesHaveIterators(); // Replace can't deal with hard BEGINs
    KeyReplaceNodes( &plan.and_rule_engine->GetCouplingKeys() );

    if( !plan.my_engines.empty() )
    {
		CouplingKeysMap coupling_keys = UnionOfSolo( *master_keys, 
                                                     plan.and_rule_engine->GetCouplingKeys() );    
		
        for( const pair< AgentCommonNeedSCREngine *, shared_ptr<SCREngine> > &p : plan.my_engines )
            p.first->SetMasterCouplingKeys( coupling_keys );
	}

    TRACE("Now replacing\n");
    *p_root_xnode = Replace();
    
    // Clear out all the replace keys (the ones inside the agents) now that replace is done
    FOREACH( Agent *a, *plan.my_agents )
        a->ResetKey();
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
            for( const pair< AgentCommonNeedSCREngine * const, shared_ptr<SCREngine> > &p : plan.my_engines )
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


shared_ptr<ContainerInterface::iterator_interface> SCREngine::VisibleWalk_iterator::Clone() const
{
	return shared_ptr<VisibleWalk_iterator>( new VisibleWalk_iterator(*this) );
}      


shared_ptr<ContainerInterface> SCREngine::VisibleWalk_iterator::GetChildContainer( TreePtr<Node> n ) const
{
	return Agent::AsAgent(n)->GetVisibleChildren(); 
}

