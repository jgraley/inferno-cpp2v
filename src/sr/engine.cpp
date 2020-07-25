#include "engine.hpp"
#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include "slave_agent.hpp"
#include "standard_agent.hpp"
#include "overlay_agent.hpp"
#include "search_container_agent.hpp"
#include "common/common.hpp"
#include <list>

using namespace SR;
 
int Engine::repetitions;
bool Engine::rep_error;


Engine::Engine( bool is_s ) :
    is_search( is_s ),
    master_ptr( NULL ),
    depth( 0 )
{
}    
    
    
// The master_agents argument is a set of agents that we should not
// configure because they were already configured by a master, and masters take 
// higher priority for configuration (so when an agent is reached from multiple
// engines, it's the most masterish one that "owns" it).
void Engine::Configure( TreePtr<Node> cp,
                        TreePtr<Node> rp,
                        const Set<Agent *> &master_agents,
                        const Engine *master )
{
    INDENT(" ");
    ASSERT(!master_ptr)("Calling configure on already-configured ")(*this);
    TRACE("Entering Engine::Configure on ")(*this)("\n");
    master_ptr = master;
    
    ConfigInstallRootAgents(cp, rp);
            
    TRACE("Elaborating ")(*this );    
    ConfigCategoriseSubs( master_agents );    
    ConfigConfigureSubs( master_agents );
} 


void Engine::Configure( const Set<Agent *> &master_agents,
                        const Engine *master )
{
    ASSERTFAIL("Engine::Configure(already, master) Must be overridden by a subclass");
}


void Engine::ConfigInstallRootAgents( TreePtr<Node> cp,
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
    

void Engine::ConfigCategoriseSubs( const Set<Agent *> &master_agents )
{
    // Walkers for compare and replace patterns that do not recurse beyond slaves (except via "through")
    // So that the compare and replace subtrees of slaves are "obsucured" and not visible
    VisibleWalk tp(pattern); 
    Set<Agent *> visible_agents;
    FOREACH( TreePtr<Node> n, tp )
        visible_agents.insert( Agent::AsAgent(n) );
    
    // Determine which ones really belong to us (some might be visible from one of our masters, 
    // in which case it should be in the supplied set.        
    my_agents = SetDifference( visible_agents, master_agents );         

    // Determine who our slaves are
    my_slaves = Set<SlaveAgent *>();
    FOREACH( Agent *a, my_agents )
        if( SlaveAgent *sa = dynamic_cast<SlaveAgent *>(a) )
            my_slaves.insert( sa );
        else
            ASSERT( !dynamic_cast<Engine *>(a) ); // not expecting to see any engine other than a slave
}


void Engine::ConfigConfigureSubs( const Set<Agent *> &master_agents )
{
    // Determine which agents our slaves should not configure
    Set<Agent *> surrounding_agents = SetUnion( master_agents, my_agents ); 
            
    // Recurse into the slaves' configure
    FOREACH( Engine *e, my_slaves )
    {
        TRACE("Recursing to configure slave ")(*e)("\n");
        e->Configure(surrounding_agents, this);
    }
    
    // Give agents pointers to here and our coupling keys
    FOREACH( Agent *a, my_agents )
    {        
        TRACE("Configuring agent ")(*a)("\n");
        a->AgentConfigure( this );             
    }    
}


const CompareReplace * Engine::GetOverallMaster() const
{
    const Engine *m = this;
    while( m->master_ptr )
        m = m->master_ptr;
    const CompareReplace *cr = dynamic_cast<const CompareReplace *>(m);
    ASSERT(cr);
    return cr;
}


void Engine::GetGraphInfo( vector<string> *labels, 
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


bool Engine::CompareLinks( const AgentQuery &query,
                           bool can_key,
                           Conjecture &conj,
                           CouplingMap &local_keys,      
                           const CouplingMap &master_keys,
                           Set<Agent *> &reached ) const
{
	ASSERT( !query.GetEvaluator() );
    // Follow up on any blocks that were noted by the agent impl
    bool made_coupling_keys = false;  
    CouplingMap coupling_keys;
    int i=0;
    FOREACH( const AgentQuery::Block &b, *query.GetBlocks() )
    {
        if( b.is_link ) // skip decisions    
		{
			TRACE("Comparing block %d\n", i);
	 
			// Get x for linked node
			const TreePtrInterface *px = b.GetPX();
			ASSERT( *px );
			   
			// Recurse now       
			if( b.abnormal )
			{
				// Non-evaluator abnormal cases only.
				if( can_key )
					continue; // Only check abnormals in restricting pass
					
				if( !made_coupling_keys ) // optimisation: only make them once, if needed at all. See #43
				{
					coupling_keys = MapUnion( master_keys, local_keys ); 
					made_coupling_keys = true;
				}
				
				if( !Compare( b.agent, px, coupling_keys ) )
					return false;
			}    
			else
			{
				// Check for a coupling match to a master engine's agent. 
				SimpleCompare sc;
				if( master_keys.IsExist(b.agent) )
				{
                    if( can_key && !sc( *px, master_keys.At(b.agent) ) ) // only in first pass
                        return false;
				}
				// Check for a coupling match to one of our agents we reached earlier in this pass.
				else if( reached.IsExist(b.agent) )
				{
                    if( can_key && !sc( *px, local_keys.At(b.agent) ) ) // only in first pass
					    return false;
				}
				else
				{
					// Recurse normally
					if( !DecidedCompare(b.agent, px, can_key, conj, local_keys, master_keys, reached) )
						return false;
				}
			}
			
			i++;
		}
    }
      
    return true;
}


// Only to be called in the restricting pass
bool Engine::CompareEvaluatorLinks( const AgentQuery &query,
									Conjecture &conj,
									CouplingMap &local_keys,      
									const CouplingMap &master_keys,
									Set<Agent *> &reached ) const
{
	ASSERT( query.GetEvaluator() );
    CouplingMap coupling_keys = MapUnion( master_keys, local_keys ); // see #43

    // Follow up on any blocks that were noted by the agent impl    
    int i=0;
    list<bool> compare_results;
    FOREACH( const AgentQuery::Block &b, *query.GetBlocks() )
    {
        if( b.is_link ) // skip decisions
		{
			TRACE("Comparing block %d\n", i);
			ASSERT( b.abnormal )("When an evaluator is used, all blocks must be into abnormal contexts");
	 
			// Get x for linked node
			const TreePtrInterface *px = b.GetPX();
								
			compare_results.push_back( Compare( b.agent, px, coupling_keys ) );
			i++;
		}
    }
    
    shared_ptr<BooleanEvaluator> evaluator = query.GetEvaluator();
	bool match = (*evaluator)( compare_results );
	TRACE(" Evaluating ");
	FOREACH(bool b, compare_results)
	    TRACE(b)(" ");
	TRACE("got ")(match)("\n");
	return match;
}


bool Engine::DecidedCompare( Agent *agent,
                             const TreePtrInterface *px,
                             bool can_key,
                             Conjecture &conj,
                             CouplingMap &local_keys,      // applies ACROSS PASSES
                             const CouplingMap &master_keys,
                             Set<Agent *> &reached ) const // applies to CURRENT PASS only
{
    INDENT(" ");
    ASSERT( px ); // Ref to target must not be NULL (i.e. corrupted ref)
    ASSERT( *px ); // Target must not be NULL
    ASSERT( !reached.IsExist(agent) ); // Only call this once per agent in a given pass

    // Remember we reached this agent in this pass
    reached.insert( agent );

    // Obtain the query state from the conjecture
    shared_ptr<AgentQuery> query = conj.GetQuery(agent);

    if( can_key ) // only in first pass...
    {
        // Run the compare implementation to get the blocks based on the choices
        TRACE(*agent)("?=")(**px)(" Gathering blocks\n");    
        agent->DecidedQuery( *query, px );
        TRACE(*agent)("?=")(**px)(" local match ")(query->IsLocalMatch())("\n");
                
        // Stop if the node itself mismatched (can be for any reason depending on agent)
        if(!query->IsLocalMatch())
            return false;

        // Remember the coupling before recursing, as we can hit the same node 
        // (eg identifier) and we need to have coupled it. 
        if( !local_keys.IsExist(agent) )
            local_keys[agent] = *px;
    }
          
    // Use worker functions to go through the blocks, special case if there is evaluator
    if( !query->GetEvaluator() )
    {
		TRACE(*agent)("?=")(**px)(" Comparing blocks\n");
        return CompareLinks( *query, can_key, conj, local_keys, master_keys, reached );
	}
    else if( !can_key ) // only in second pass...
    {
		TRACE(*agent)("?=")(**px)(" Comparing evaluator blocks\n");
        return CompareEvaluatorLinks( *query, conj, local_keys, master_keys, reached );
	}
    else
        return true;
}


// This one operates from root for a stand-alone compare operation (side API)
bool Engine::Compare( const TreePtrInterface *p_start_x,
                      const CouplingMap &master_keys ) const
{
	ASSERT( root_agent );
    return Compare( root_agent, p_start_x, master_keys );
}


// This one if you don't want the resulting keys and conj (ie not doing a replace)
bool Engine::Compare( Agent *start_agent,
                      const TreePtrInterface *p_start_x,
                      const CouplingMap &master_keys ) const
{
    Conjecture conj(my_agents);
    CouplingMap local_keys; 
    return Compare( start_agent, p_start_x, conj, local_keys, master_keys );
}


// This one if you want the resulting couplings and conj (ie doing a replace imminently)
bool Engine::Compare( Agent *start_agent,
                      const TreePtrInterface *p_start_x,
                      Conjecture &conj,
                      CouplingMap &local_keys,
                      const CouplingMap &master_keys ) const
{
    INDENT("C");
    ASSERT( p_start_x );
    ASSERT( *p_start_x );
    TRACE("Compare x=")(**p_start_x);
    TRACE(" pattern=")(*start_agent);
    ASSERT( &local_keys != &master_keys );
    //TRACE(**pcontext)(" @%p\n", pcontext);
           
    SimpleCompare sc;
    if( master_keys.IsExist(start_agent) )
	{
		return sc( *p_start_x, master_keys.At(start_agent) );
	}      
           
    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    bool r;
    //int i=0;
    while(1)
    {
        // Try out the current conjecture. This will call RememberDecision() once for each decision;
        // RememberDecision() will return the current choice for that decision, if absent it will
        // add the decision and choose the first choice, if the decision reaches the end it
        // will remove the decision.

        // Only key if the keys are already set to KEYING (which is 
        // the initial value). Keys could be RESTRICTING if we're under
        // a SoftNot node, in which case we only want to restrict.
        
        // Initialise keys to the ones inherited from master, keeping 
        // none of our own from any previous unsuccessful attempt.
        local_keys = CouplingMap();

        // Do a two-pass matching process: first get the keys...
        {
			Set<Agent *> reached;
            r = DecidedCompare( start_agent, p_start_x, true, conj, local_keys, master_keys, reached );
        }
               
        if( r )
        {
            // ...now restrict the search according to the couplings. This 
            // allows a coupling keyed late in the walk to restrict something 
            // seen earlier (eg in an abnormal context where keying is impossible)
            Set<Agent *> reached;
            r = DecidedCompare( start_agent, p_start_x, false, conj, local_keys, master_keys, reached );
        }
        
        // If we got a match, we're done. If we didn't, and we've run out of choices, we're done.
        if( r )
        {
            TRACE("Engine hit\n");
            break; // Success
        }
        TRACE("Engine miss, trying increment conjecture\n");
        if( !conj.Increment() )
            break; // Failure                   

        //assert(i<1000);
        //i++;
    }
    // by now, we succeeded and local_keys is the right set of keys
    return r;
}


void Engine::KeyReplaceNodes( Conjecture &conj,
                              const CouplingMap &coupling_keys ) const
{
    INDENT("K");   
    // NO! coupling_keys now contains the master's couplings - we must not touch the 
    // master's agents 
    //typedef pair< Agent *, TreePtr<Node> > keypair;
    //FOREACH( keypair c, coupling_keys )
    //    c.first->KeyReplace(c.second, conj.GetChoices(c.first));
        
    FOREACH( Agent *a, my_agents )
    {
		TRACE(*a)(coupling_keys.IsExist( a )?" is in coupling_keys ":" is not in coupling_keys")
		     (a->GetCoupled()?" and is self-coupled\n":" and is not self-coupled\n");
        if( coupling_keys.IsExist( a ) && !a->GetCoupled() )
            a->KeyReplace(coupling_keys.at(a), conj.GetChoices(a));
	}
}


TreePtr<Node> Engine::Replace() const
{
    INDENT("R");
    
    // Now replace according to the couplings
    return root_agent->BuildReplace();
}


void Engine::GatherCouplings( CouplingMap &coupling_keys ) const
{
	// Get couplings from agents into the supplied map if not there already
    FOREACH( Agent *a, my_agents )
        if( a->GetCoupled() && !coupling_keys.IsExist( a ) )
            coupling_keys[a] = a->GetCoupled();	
}


bool Engine::SingleCompareReplace( TreePtr<Node> *p_root,
                                   const CouplingMap &master_keys ) 
{
    INDENT(">");

    CouplingMap local_keys;
    Conjecture conj(my_agents);

    TRACE("Begin search\n");
    bool r = Compare( root_agent, p_root, conj, local_keys, master_keys );
    if( !r )
        return false;
       
    TRACE("Search successful, now keying replace nodes\n");
    KeyReplaceNodes( conj, local_keys );

    if( !my_slaves.empty() )
    {
		CouplingMap coupling_keys = MapUnion( master_keys, local_keys );    
		
        FOREACH( SlaveAgent *sa, my_slaves )
            sa->SetMasterCouplingKeys( coupling_keys );
	}

    TRACE("Now replacing\n");
    *p_root = Replace();
    
    // Clear out all the replace keys (the ones inside the agents) now that replace is done
    FOREACH( Agent *a, my_agents )
        a->ResetKey();

    return true;
}


// Perform search and replace on supplied program based
// on supplied patterns and couplings. Does search and replace
// operations repeatedly until there are no more matches. Returns how
// many hits we got.
int Engine::RepeatingCompareReplace( TreePtr<Node> *proot,
                                     const CouplingMap &master_keys )
{
    INDENT("}");
    TRACE("begin RCR\n");
        
    ASSERT( pattern )("Engine object was not configured before invocation.\n"
                      "Either call Configure() or supply pattern arguments to constructor.\n"
                      "Thank you for taking the time to read this message.\n");
    
    for(int i=0; i<repetitions; i++) 
    {
        bool stop = depth < stop_after.size() && stop_after[depth]==i;
        if( stop )
            FOREACH( Engine *e, my_slaves )
                e->SetStopAfter(stop_after, depth+1); // and propagate the remaining ones
                
        bool r = SingleCompareReplace( proot, master_keys );
        TRACE("SCR result %d\n", r);        
        
        if( !r )
        {
            if( depth < stop_after.size() )
                ASSERT(stop_after[depth]<i)("Stop requested after hit that doesn't happen, there are only %d", i);
            return i; // when the compare fails, we're done
        }
        if( stop )
        {
            TRACE("Stopping after hit %d\n", stop_after[depth]);
            return i;
        }    
    }
    
    TRACE("Over %d reps\n", repetitions); 
    ASSERT(!rep_error)
          ("Still getting matches after %d repetitions, may be repeating forever.\n"
           "Try using -rn%d to suppress this error\n", repetitions, repetitions);
       
    TRACE("exiting\n");
    return repetitions;
}


shared_ptr<ContainerInterface::iterator_interface> Engine::VisibleWalk_iterator::Clone() const
{
	return shared_ptr<VisibleWalk_iterator>( new VisibleWalk_iterator(*this) );
}      


shared_ptr<ContainerInterface> Engine::VisibleWalk_iterator::GetChildContainer( TreePtr<Node> n ) const
{
	return Agent::AsAgent(n)->GetVisibleChildren(); 
}

