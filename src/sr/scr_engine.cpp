#include "scr_engine.hpp"
#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include "standard_agent.hpp"
#include "overlay_agent.hpp"
#include "search_container_agent.hpp"
#include "common/common.hpp"
#include <list>

using namespace SR;
using namespace std;

int SCREngine::repetitions;
bool SCREngine::rep_error;


SCREngine::SCREngine( bool is_s ) :
    is_search( is_s ),
    master_ptr( NULL ),
    depth( 0 )
{
}    
    
    
// The master_agents argument is a set of agents that we should not
// configure because they were already configured by a master, and masters take 
// higher priority for configuration (so when an agent is reached from multiple
// engines, it's the most masterish one that "owns" it).
void SCREngine::Configure( const CompareReplace *overall_master,
                           TreePtr<Node> cp,
                           TreePtr<Node> rp,
                           const Set<Agent *> &master_agents,
                           const SCREngine *master )
{
    INDENT(" ");
    ASSERT(!master_ptr)("Calling configure on already-configured ")(*this);
    TRACE("Entering SCREngine::Configure on ")(*this)("\n");
    overall_master_ptr = overall_master;
    master_ptr = master;
    
    ConfigInstallRootAgents(cp, rp);
            
    TRACE("Elaborating ")(*this );    

    Set<AgentCommonNeedSCREngine *> my_agents_needing_engines;   
    ConfigCategoriseSubs( master_agents, my_agents_needing_engines );    
    ConfigCreateMyEngines( my_agents_needing_engines );    
    ConfigConfigureSubs( master_agents );
    
    and_rule_engine.Configure(root_agent, master_agents);    
} 


void SCREngine::Configure( const Set<Agent *> &master_agents,
                           const SCREngine *master )
{
    ASSERTFAIL("SCREngine::Configure(already, master) Must be overridden by a subclass");
}


void SCREngine::ConfigInstallRootAgents( TreePtr<Node> cp,
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
    pattern = cp; 
    root_agent = Agent::AsAgent(pattern);
}
    

void SCREngine::ConfigCategoriseSubs( const Set<Agent *> &master_agents, Set<AgentCommonNeedSCREngine *> &my_agents_needing_engines )
{
    // Walkers for compare and replace patterns that do not recurse beyond slaves (except via "through")
    // So that the compare and replace subtrees of slaves are "obsucured" and not visible
    VisibleWalk tp(pattern); 
    Set<Agent *> visible_agents;
    FOREACH( TreePtr<Node> n, tp )
        visible_agents.insert( Agent::AsAgent(n) );
    
    // Determine which ones really belong to us (some might be visible from one of our masters, 
    // in which case it should be in the supplied set.        
    my_agents = make_shared< Set<Agent *> >();
    *my_agents = SetDifference( visible_agents, master_agents );         

    // Determine who our slaves are
    FOREACH( Agent *a, *my_agents )
        if( AgentCommonNeedSCREngine *sa = dynamic_cast<AgentCommonNeedSCREngine *>(a) )
            my_agents_needing_engines.insert( sa );
}


void SCREngine::ConfigCreateMyEngines( const Set<AgentCommonNeedSCREngine *> &my_agents_needing_engines )
{
    FOREACH( AgentCommonNeedSCREngine *a, my_agents_needing_engines )
    {
        my_engines.emplace( std::piecewise_construct,
                            std::forward_as_tuple( a ),
                            std::forward_as_tuple( a->IsSearch() ) );
    }
        
}


void SCREngine::ConfigConfigureSubs( const Set<Agent *> &master_agents )
{
    // Determine which agents our slaves should not configure
    Set<Agent *> surrounding_agents = SetUnion( master_agents, *my_agents ); 
            
    // Recurse into the slaves' configure
    for( pair<AgentCommonNeedSCREngine * const, SCREngine> &p : my_engines )
    {
        TRACE("Recursing to configure slave ")(*p.first)("\n");
        p.second.Configure( overall_master_ptr, 
                            p.first->GetSearchPattern(),
                            p.first->GetReplacePattern(),
                            surrounding_agents, 
                            this );
        p.first->AgentConfigure( this, 
                                 &p.second );
    }
    // Give agents pointers to here and our coupling keys
    FOREACH( Agent *a, *my_agents )
    {        
        if( dynamic_cast<AgentCommonNeedSCREngine *>(a) )
            continue; // TODO yuck, should determine this in ConfigCategoriseSubs()
        TRACE("Configuring agent ")(*a)("\n");
        a->AgentConfigure( this );             
    }    
}


const CompareReplace * SCREngine::GetOverallMaster() const
{
    return overall_master_ptr;
}


void SCREngine::GetGraphInfo( vector<string> *labels, 
                              vector< TreePtr<Node> > *blocks ) const
{
    // TODO pretty sure this can "suck in" explicitly placed stuff and overlay 
    // nodes under the SR, CR or slave. These are obviously unnecessary, maybe I
    // should error on them?
    TreePtr<Node> original_pattern = pattern;
    if( is_search )
    {
        TreePtr< Stuff<Node> > stuff = dynamic_pointer_cast< Stuff<Node> >(original_pattern);
        ASSERT( stuff );
        original_pattern = stuff->terminus;
    }
    TreePtr< Overlay<Node> > overlay = dynamic_pointer_cast< Overlay<Node> >(original_pattern);
    if( overlay )
    {        
        labels->push_back(is_search?"search":"compare");    
        blocks->push_back(overlay->through);
        labels->push_back("replace");
        blocks->push_back(overlay->overlay);
    }
    else
    {
        labels->push_back(is_search?"search_replace":"compare_replace");    
        blocks->push_back(original_pattern);
    }
}

// This one operates from root for a stand-alone compare operation and
// no master keys.
void SCREngine::Compare( const TreePtrInterface *p_start_x ) const
{
    and_rule_engine.Compare( p_start_x );
}


// This one operates from root for a stand-alone compare operation (side API)
//void SCREngine::Compare( const TreePtrInterface *p_start_x,
//                         const CouplingMap *master_keys ) const
//{
//    and_rule_engine.Compare( p_start_x, master_keys );
//}


void SCREngine::KeyReplaceNodes( const Conjecture &conj,
                                 const CouplingMap *coupling_keys ) const
{
    INDENT("K");   
    // NO! coupling_keys now contains the master's couplings - we must not touch the 
    // master's agents 
    //typedef pair< Agent *, TreePtr<Node> > keypair;
    //FOREACH( keypair c, coupling_keys )
    //    c.first->KeyReplace(c.second, conj.GetChoices(c.first));
        
    FOREACH( Agent *a, *my_agents )
    {
		TRACE(*a)(coupling_keys->IsExist( a )?" is in coupling_keys ":" is not in coupling_keys")
		     (a->GetCoupled()?" and is self-coupled\n":" and is not self-coupled\n");
        if( coupling_keys->IsExist( a ) && !a->GetCoupled() )
            a->KeyReplace(coupling_keys->at(a), conj.GetChoices(a));
	}
    TRACE("OK\n");
}


TreePtr<Node> SCREngine::Replace() const
{
    INDENT("R");
    
    // Now replace according to the couplings
    return root_agent->BuildReplace();
}


void SCREngine::GatherCouplings( CouplingMap *coupling_keys ) const
{
	// Get couplings from agents into the supplied map if not there already
    FOREACH( Agent *a, *my_agents )
        if( a->GetCoupled() && !coupling_keys->IsExist( a ) )
            (*coupling_keys)[a] = a->GetCoupled();	
}


void SCREngine::SingleCompareReplace( TreePtr<Node> *p_root,
                                      const CouplingMap *master_keys ) 
{
    INDENT(">");

    TRACE("Begin search\n");
    and_rule_engine.Compare( p_root, master_keys );
           
    TRACE("Search successful, now keying replace nodes\n");
    KeyReplaceNodes( and_rule_engine.GetConjecture(), 
                     &and_rule_engine.GetCouplingKeys() );

    if( !my_engines.empty() )
    {
		CouplingMap coupling_keys = MapUnion( *master_keys, 
                                              and_rule_engine.GetCouplingKeys() );    
		
        for( pair<AgentCommonNeedSCREngine * const, SCREngine> &p : my_engines )
            p.first->SetMasterCouplingKeys( coupling_keys );
	}

    TRACE("Now replacing\n");
    *p_root = Replace();
    
    // Clear out all the replace keys (the ones inside the agents) now that replace is done
    FOREACH( Agent *a, *my_agents )
        a->ResetKey();

    TRACE("OK\n");
}


// Perform search and replace on supplied program based
// on supplied patterns and couplings. Does search and replace
// operations repeatedly until there are no more matches. Returns how
// many hits we got.
int SCREngine::RepeatingCompareReplace( TreePtr<Node> *proot,
                                        const CouplingMap *master_keys )
{
    INDENT("}");
    TRACE("begin RCR\n");
        
    ASSERT( pattern )("SCREngine object was not configured before invocation.\n"
                      "Either call Configure() or supply pattern arguments to constructor.\n"
                      "Thank you for taking the time to read this message.\n");
    
    for(int i=0; i<repetitions; i++) 
    {
        bool stop = depth < stop_after.size() && stop_after[depth]==i;
        if( stop )
            for( pair<AgentCommonNeedSCREngine * const, SCREngine> &p : my_engines )
                p.second.SetStopAfter(stop_after, depth+1); // and propagate the remaining ones
        try
        {
            SingleCompareReplace( proot, master_keys );
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
       
    TRACE("exiting\n");
    TRACE("OK\n");
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

