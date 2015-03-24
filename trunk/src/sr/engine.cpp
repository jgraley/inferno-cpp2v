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
    my_engines = Set<Engine *>();
    FOREACH( Agent *a, my_agents )
        if( Engine *e = dynamic_cast<Engine *>(a) )
            my_engines.insert( e );

    // Determine which agents our slaves should not configure
    Set<Agent *> agents_now_configured = SetUnion( agents_already_configured, my_agents ); 
            
    // Recurse into the slaves' configure
    FOREACH( Engine *e, my_engines )
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


bool Engine::Compare( const TreePtrInterface &x ) const
{
    INDENT("C");
    ASSERT( x );
    TRACE("Compare x=")(*x);
    TRACE(" pattern=")(*root_agent);
    //TRACE(**pcontext)(" @%p\n", pcontext);
           
    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    Conjecture conj;
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
            // Unkey 
        FOREACH( Agent *a, my_agents )
            a->ResetKey();

        // Do a two-pass matching process: first get the keys...
        conj.PrepareForDecidedCompare();
        r = root_agent->DecidedCompare( x, true, conj );
               
        if( r )
        {
            // ...now restrict the search according to the couplings
            conj.PrepareForDecidedCompare();
            r = root_agent->DecidedCompare( x, false, conj );
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
    return r;
}


void Engine::KeyReplaceNodes() const
{
    INDENT("K");    
    FOREACH( Agent *a, my_agents )
        a->KeyReplace();
}


TreePtr<Node> Engine::Replace() const
{
    INDENT("R");
    
    // Now replace according to the couplings
    return root_agent->BuildReplace();
}


bool Engine::SingleCompareReplace( TreePtr<Node> *proot ) 
{
    INDENT(">");

    TRACE("Begin search\n");
    bool r = Compare( *proot );
    if( !r )
        return false;

    TRACE("Search successful, now keying replace nodes\n");
    KeyReplaceNodes();

    TRACE("Now replacing\n");
    *proot = Replace();

    return true;
}


// Perform search and replace on supplied program based
// on supplied patterns and couplings. Does search and replace
// operations repeatedly until there are no more matches. Returns how
// many hits we got.
int Engine::RepeatingCompareReplace( TreePtr<Node> *proot )
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
            FOREACH( Engine *e, my_engines )
                e->SetStopAfter(stop_after, depth+1); // and propagate the remaining ones
        bool r = SingleCompareReplace( proot );
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

