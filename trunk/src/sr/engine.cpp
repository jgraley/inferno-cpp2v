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


bool Engine::DecidedCompare( Agent *agent,
                             const TreePtrInterface &x,
                             bool can_key,
                             Conjecture &conj,
                             CouplingMap &coupling_keys ) const
{
    INDENT(" ");
    ASSERT( &x ); // Ref to target must not be NULL (i.e. corrupted ref)
    ASSERT( x ); // Target must not be NULL
            
    // If the agent is coupled already, check for a coupling match
    if( coupling_keys.IsExist(agent) )
    {
        SimpleCompare sc;
        bool match = sc( x, coupling_keys[agent] );
        if( !my_agents.IsExist(agent) )
            return match; // do not recurse into master's agents
        if( !match )
        {
            conj.RegisterDecisions( agent, false );
            return false;
		}
    }
    
    // Obtain the choices from the conjecture
    deque<ContainerInterface::iterator> choices = conj.GetChoices(agent);

    TRACE(*agent)(" Gathering links\n");    
    // Run the compare implementation to get the links based on the choices
    Links mylinks = agent->DecidedQuery( x, choices );
    
    // The number of decisions reported should not shrink
    if( mylinks.local_match )
        ASSERT( mylinks.decisions.size()>=choices.size() )(*this)(" cs=%d ds=%d\n", choices.size(), mylinks.decisions.size());    
    
    // Feed the decisions info in the links structure back to the conjecture
    conj.RegisterDecisions( agent, mylinks.local_match, mylinks.decisions );
        
    // Stop if the node itself mismatched (can be for any reason depending on agent)
    if(!mylinks.local_match)
    {
        TRACE(*agent)(" local mismatch, aborting\n");
        return false;
    }
    
    // Remember the coupling before recursing, as we can hit the same node 
    // (eg identifier) and we need to have coupled it. 
    if( can_key && !coupling_keys.IsExist(agent) )
        coupling_keys[agent] = x;
      
    // Follow up on any links that were noted by the agent impl
    TRACE(*agent)(" Comparing links\n");    
    int i=0;
    FOREACH( const Links::Link &l, mylinks.links )
    {
        TRACE("ConjSpin Comparing link %d\n", i);
 
        // Get pattern for linked node
        const TreePtrInterface *px;
        if( l.px )
            px = l.px; // linked pattern is in input tree
        else
            px = &(l.local_x); // linked pattern is local, kept alive by local_x    
            
        // Recurse now       
        bool r;       
        if( l.abnormal )
            // Recurse into an abnormal context
            r = AbnormalCompare(l.agent, *px, can_key, coupling_keys);
        else
            // Recurse normally
            r = DecidedCompare(l.agent, *px, can_key, conj, coupling_keys);
            
        // sense swapping for "not" nodes, only apply during restricting pass 
        if( l.invert )
            r = !r || can_key; 
            
        // Early out on mismatch
        if( !r )
            return false;
        i++;
    }
      
    TRACE(*agent)(" Done\n");        
    return true;
}


bool Engine::AbnormalCompare( Agent *agent,
                              const TreePtrInterface &x, 
                              bool can_key,
                              CouplingMap &coupling_keys ) const
{
    INDENT("A");
    ASSERT( x );
    TRACE("Compare x=")(*x);
    TRACE(" pattern=")(*agent);

    // Only run during "restricting" pass
    if( can_key )
        return true;

    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    Conjecture conj(my_agents);
    bool r;
    //int i = 0;
    while(1)
    {
		// Prepare for a new tree walk
        conj.PrepareForDecidedCompare(0);
        
        // Walk into the abnormal subtree
        r = DecidedCompare( agent, x, false, conj, coupling_keys );
        
        // If we got a match, we're done. If we didn't, and we've run out of choices, we're done.
        if( r )
        {
            TRACE("ConjSpin Abnormal hit\n");            
            break; // Success
        }
            
        if( !conj.Increment() )
            break; // Failure        
            
        //assert(i<1000);
        //i++;
    }
    return r;
}


bool Engine::Compare( const TreePtrInterface &x ) const
{
    Map< Agent *, TreePtr<Node> > coupling_keys, empty;
    Conjecture conj(my_agents);
    return Compare( x, conj, coupling_keys, empty );
}


bool Engine::Compare( const TreePtrInterface &x,
                      Conjecture &conj,
                      CouplingMap &matching_coupling_keys,
                      const CouplingMap &initial_coupling_keys ) const
{
    INDENT("C");
    ASSERT( x );
    TRACE("Compare x=")(*x);
    TRACE(" pattern=")(*root_agent);
    ASSERT( &matching_coupling_keys != &initial_coupling_keys );
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
        r = true;

        // Only key if the keys are already set to KEYING (which is 
        // the initial value). Keys could be RESTRICTING if we're under
        // a SoftNot node, in which case we only want to restrict.
        
        // Initialise keys to the ones inherited from master, keeping 
        // none of our own from any previous unsuccessful attempt.
        matching_coupling_keys = initial_coupling_keys;

        // Do a two-pass matching process: first get the keys...
        conj.PrepareForDecidedCompare(0);
        r = DecidedCompare( root_agent, x, true, conj, matching_coupling_keys );
               
        if( r )
        {
            // ...now restrict the search according to the couplings. This 
            // allows a coupling keyed late in the walk to restrict something 
            // seen earlier (eg in an abnormal context where keying is impossible)
            conj.PrepareForDecidedCompare(1);
            r = DecidedCompare( root_agent, x, false, conj, matching_coupling_keys );
        }
        
        // If we got a match, we're done. If we didn't, and we've run out of choices, we're done.
        if( r )
        {
            TRACE("ConjSpin Engine hit\n");
            break; // Success
        }
            
        if( !conj.Increment() )
            break; // Failure                   

        //assert(i<1000);
        //i++;
    }
    // by now, we succeeded and matching_coupling_keys is the right set of keys
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
        if( coupling_keys.IsExist( a ) && !a->GetCoupled() )
            a->KeyReplace(coupling_keys.at(a), conj.GetChoices(a));
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
                                   const CouplingMap &initial_coupling_keys ) 
{
    INDENT(">");

    CouplingMap matching_coupling_keys;
    Conjecture conj(my_agents);

    TRACE("Begin search\n");
    bool r = Compare( *proot, conj, matching_coupling_keys, initial_coupling_keys );
    if( !r )
        return false;

    TRACE("Search successful, now keying replace nodes\n");
    KeyReplaceNodes( conj, matching_coupling_keys );

    FOREACH( SlaveAgent *sa, my_slaves )
        sa->SetMasterCouplingKeys( matching_coupling_keys );

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
                                     const CouplingMap &initial_coupling_keys )
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
                
        bool r = SingleCompareReplace( proot, initial_coupling_keys );
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
