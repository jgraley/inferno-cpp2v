#include "engine.hpp"
#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include "slave_agent.hpp"
#include "normal_agent.hpp"
#include "overlay_agent.hpp"
#include "search_container_agent.hpp"
#include "common/common.hpp"
#include <list>

using namespace SR;

//#define STRACE
 
int Engine::repetitions;
bool Engine::rep_error;


Engine::Engine( bool is_s ) :
    is_search( is_s ),
    master_ptr( NULL ),
    depth( 0 )
{
}    
    
    
// The agents_already_configured argument is a set of agents that we should not
// configure because they were already configured by a master, and masters take 
// higher priority for configuration (so when an agent is reached from multiple
// engines, it's the most masterish one that "owns" it).
void Engine::Configure( TreePtr<Node> cp,
                        TreePtr<Node> rp,
                        const Set<Agent *> &agents_already_configured,
                        const Engine *master )
{
    INDENT(" ");
    ASSERT(!pattern)("Calling configure on already-configured ")(*this);
    TRACE("Entering Engine::Configure on ")(*this)("\n");
    master_ptr = master;

    ASSERT( cp );
    
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
            
    TRACE("Elaborating ")(*this );

    // Walkers for compare and replace patterns that do not recurse beyond slaves (except via "through")
    SlaveAgent::UniqueWalkNoSlavePattern tp(pattern); 
    Set<Agent *> immediate_agents;
    FOREACH( TreePtr<Node> n, tp )
        immediate_agents.insert( Agent::AsAgent(n) );
    
    // Determine which ones really belong to us (some might be visible from one of our masters, 
    // in which case it should be in the supplied set.        
    my_agents = SetDifference( immediate_agents, agents_already_configured );         

    // Determine who our slaves are
    my_slaves = Set<SlaveAgent *>();
    FOREACH( Agent *a, my_agents )
        if( SlaveAgent *sa = dynamic_cast<SlaveAgent *>(a) )
            my_slaves.insert( sa );
        else
            ASSERT( !dynamic_cast<Engine *>(a) ); // not expecting to see any engine other than a slave

    // Determine which agents our slaves should not configure
    Set<Agent *> agents_now_configured = SetUnion( agents_already_configured, my_agents ); 
            
    // Recurse into the slaves' configure
    FOREACH( Engine *e, my_slaves )
    {
        TRACE("Recursing to configure slave ")(*e)("\n");
        e->Configure(agents_now_configured, this);
    }
    
    // Give agents pointers to here and our coupling keys
    FOREACH( Agent *a, my_agents )
    {        
        TRACE("Configuring agent ")(*a)("\n");
        a->AgentConfigure( this );             
    }    
} 


void Engine::Configure( const Set<Agent *> &agents_already_configured,
                        const Engine *master )
{
    ASSERTFAIL("Engine::Configure(already, master) Must be overridden by a subclass");
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
                           vector< TreePtr<Node> > *links ) const
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
        links->push_back(overlay->through);
        labels->push_back("replace");
        links->push_back(overlay->overlay);
    }
    else
    {
        labels->push_back(is_search?"search_replace":"compare_replace");    
        links->push_back(original_pattern);
    }
}


bool Engine::CompareLinks( const Links &mylinks,
                           bool can_key,
                           Conjecture &conj,
                           CouplingMap &local_keys,      
                           const CouplingMap &master_keys,
                           Set<Agent *> &reached ) const
{
	ASSERT( !mylinks.evaluator );
    // Follow up on any links that were noted by the agent impl
    bool made_coupling_keys = false;  
    CouplingMap coupling_keys;
    int i=0;
    FOREACH( const Links::Link &l, mylinks.links )
    {
        TRACE("Comparing link %d\n", i);
 
        // Get pattern for linked node
        const TreePtrInterface *px = l.GetX();
           
        // Recurse now       
        if( l.abnormal )
        {
			// Non-evaluator abnormal cases only.
			if( can_key )
			    continue; // Only check abnormals in restricting pass
			    
			if( !made_coupling_keys ) // optimisation: only make them once, if needed at all.
			{
			    coupling_keys = MapUnion( master_keys, local_keys ); 
     	        made_coupling_keys = true;
			}
			
			if( !Compare( l.agent, *px, coupling_keys ) )
			    return false;
        }    
        else
        {
			// Check for a coupling match to one of our agents we reached earlier in this pass
			SimpleCompare sc;
			if( reached.IsExist(l.agent) )
			{
				if( !sc( *px, local_keys.At(l.agent) ) )
				    return false;
			}
			else
			{
				// Recurse normally
				if( !DecidedCompare(l.agent, *px, can_key, conj, local_keys, master_keys, reached) )
					return false;
			}
 		}
 		
        i++;
    }
      
    return true;
}


// Only to be called in the restricting pass
bool Engine::CompareEvaluatorLinks( const Links &mylinks,
									Conjecture &conj,
									CouplingMap &local_keys,      
									const CouplingMap &master_keys,
									Set<Agent *> &reached ) const
{
	ASSERT( mylinks.evaluator );
    CouplingMap coupling_keys = MapUnion( master_keys, local_keys );

    // Follow up on any links that were noted by the agent impl    
    int i=0;
    deque<bool> results;
    FOREACH( const Links::Link &l, mylinks.links )
    {
        TRACE("Comparing link %d\n", i);
 		ASSERT( l.abnormal )("When an evaluator is used, all links must be into abnormal contexts");
 
        // Get pattern for linked node
        const TreePtrInterface *px = l.GetX();
                     		
		results.push_back( Compare( l.agent, *px, coupling_keys ) );
        i++;
    }

	bool match = (*mylinks.evaluator)( results );
	TRACE(" Evaluating ");
	FOREACH(bool b, results)
	    TRACE(b)(" ");
	TRACE("got ")(match)("\n");
	return match;
}


bool Engine::DecidedCompare( Agent *agent,
                             const TreePtrInterface &x,
                             bool can_key,
                             Conjecture &conj,
                             CouplingMap &local_keys,      // applies ACROSS PASSES
                             const CouplingMap &master_keys,
                             Set<Agent *> &reached ) const // applies to CURRENT PASS only
{
    INDENT(" ");
    ASSERT( &x ); // Ref to target must not be NULL (i.e. corrupted ref)
    ASSERT( x ); // Target must not be NULL
            
    // Check for a coupling match to a master engine's agent
	SimpleCompare sc;
    if( master_keys.IsExist(agent) )
        return sc( x, master_keys.At(agent) );
	
    // Obtain the choices from the conjecture
    deque<ContainerInterface::iterator> choices = conj.GetChoices(agent);

    // Run the compare implementation to get the links based on the choices
    TRACE(*agent)("?=")(*x)(" Gathering links\n");    
    Links mylinks = agent->DecidedQuery( x, choices );
    TRACE(*agent)("?=")(*x)(" local match ")(mylinks.local_match)("\n");
    
    // Feed the decisions info in the links structure back to the conjecture
    conj.RegisterDecisions( agent, mylinks.local_match, mylinks.decisions );
        
    // Stop if the node itself mismatched (can be for any reason depending on agent)
    if(!mylinks.local_match)
        return false;

    // Remember the coupling before recursing, as we can hit the same node 
    // (eg identifier) and we need to have coupled it. 
    if( can_key && !local_keys.IsExist(agent) )
        local_keys[agent] = x;
        
    // Remember we reached this agent in this pass
    reached.insert( agent );
      
    // Use worker function to go through the links, special case if there is evaluator
    if( !mylinks.evaluator )
    {
		TRACE(*agent)("?=")(*x)(" Comparing links\n");
        return CompareLinks( mylinks, can_key, conj, local_keys, master_keys, reached );
	}
    else if( !can_key )
    {
		TRACE(*agent)("?=")(*x)(" Comparing evaluator links\n");
        return CompareEvaluatorLinks( mylinks, conj, local_keys, master_keys, reached );
	}
    else
        return true;
}


// This one operates from root for a stand-alone compare operation (side API)
bool Engine::Compare( const TreePtrInterface &start_x ) const
{
    const Map< Agent *, TreePtr<Node> > master_keys; // always empty
    return Compare( root_agent, start_x, master_keys );
}


// This one if you don't want the resulting keys and conj (ie not doing a replace)
bool Engine::Compare( Agent *start_agent,
                      const TreePtrInterface &start_x,
                      const CouplingMap &master_keys ) const
{
    Conjecture conj(my_agents);
    CouplingMap local_keys; 
    return Compare( start_agent, start_x, conj, local_keys, master_keys );
}


// This one if you want the resulting couplings and conj (ie doing a replace imminently)
bool Engine::Compare( Agent *start_agent,
                      const TreePtrInterface &start_x,
                      Conjecture &conj,
                      CouplingMap &local_keys,
                      const CouplingMap &master_keys ) const
{
    INDENT("C");
    ASSERT( start_x );
    TRACE("Compare x=")(*start_x);
    TRACE(" pattern=")(*start_agent);
    ASSERT( &local_keys != &master_keys );
    //TRACE(**pcontext)(" @%p\n", pcontext);
           
    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    bool r;
    //int i=0;
    while(1)
    {
        // Try out the current conjecture. This will call HandlDecision() once for each decision;
        // HandleDecision() will return the current choice for that decision, if absent it will
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
            conj.PrepareForDecidedCompare(0);
			Set<Agent *> reached;
            r = DecidedCompare( start_agent, start_x, true, conj, local_keys, master_keys, reached );
        }
               
        if( r )
        {
            // ...now restrict the search according to the couplings. This 
            // allows a coupling keyed late in the walk to restrict something 
            // seen earlier (eg in an abnormal context where keying is impossible)
            conj.PrepareForDecidedCompare(1);
            Set<Agent *> reached;
            r = DecidedCompare( start_agent, start_x, false, conj, local_keys, master_keys, reached );
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


bool Engine::SingleCompareReplace( TreePtr<Node> *proot,
                                   const CouplingMap &master_keys ) 
{
    INDENT(">");

    CouplingMap local_keys;
    Conjecture conj(my_agents);

    TRACE("Begin search\n");
    bool r = Compare( root_agent, *proot, conj, local_keys, master_keys );
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
    *proot = Replace();
    
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

