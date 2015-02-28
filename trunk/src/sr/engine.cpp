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


/** Walks the tree, avoiding the "search"/"compare" and "replace" members of slaves
    but still recurses through the "through" member. Therefore, it visits all the
    nodes at the same slave level as the root. Based on UniqueWalk, so each node only
    visited once. */
class UniqueWalkNoSlavePattern_iterator : public UniqueWalk::iterator
{
public:
    UniqueWalkNoSlavePattern_iterator( TreePtr<Node> &root ) : UniqueWalk::iterator(root) {}        
    UniqueWalkNoSlavePattern_iterator() : UniqueWalk::iterator() {}
    virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const
    {
        return shared_ptr<UniqueWalkNoSlavePattern_iterator>( new UniqueWalkNoSlavePattern_iterator(*this) );
    }      
private:
    virtual shared_ptr<ContainerInterface> GetChildContainer( TreePtr<Node> n ) const
    {
        // We need to create a container of elements of the child.
        if( SlaveAgent *sa = dynamic_cast<SlaveAgent *>( Agent::AsAgent(n) ) )
        {
            // it's a slave, so set up a container containing only "through", not "compare" or "replace"
            shared_ptr< Sequence<Node> > seq( new Sequence<Node> );
            seq->push_back( sa->GetThrough() );
            return seq;
        }
        else
        {
            // it's not a slave, so proceed as for UniqueWalk
            return UniqueWalk::iterator::GetChildContainer(n);
        }
    }
};


typedef ContainerFromIterator< UniqueWalkNoSlavePattern_iterator, TreePtr<Node> > UniqueWalkNoSlavePattern;


Engine::Engine( bool is_s ) :
    is_search( is_s ),
    master_ptr( NULL )
{
}    
    
    
// The agents_already_configured argument is a set of agents that we should not
// configure because they were already configured by a master, and masters take 
// higher priority for configuration (so when an agent is reached from multiple
// engines, it's the most masterish one that "owns" it).
void Engine::Configure( TreePtr<Node> cp,
                        TreePtr<Node> rp,
                        const Set<Agent *> &agents_already_configured )
{
    INDENT;
    ASSERT(!pattern)("Calling configure on already-configured ")(*this);
    TRACE("Entering CR::Configure on ")(*this)("\n");

    // TODO now that this operates per-slave instead of recursing through everything from the 
    // master, we need to obey the rule that slave patterns are complete before Configure, as
    // with master. Maybe an optional check on first invocation? And change all existing 
    // steps to comply.
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
            
    TRACE("Elaborating ")(string( *this ));

    // Walkers for compare and replace patterns that do not recurse beyond slaves (except via "through")
    UniqueWalkNoSlavePattern tp(pattern); // TODO remove duplication here when just have pattern, as mentioned above
    Set<Agent *> immediate_agents;
    FOREACH( TreePtr<Node> n, tp )
        immediate_agents.insert( Agent::AsAgent(n) );
    
    // Now configure all the ones we are allowed to configure        
    my_agents = SetDifference( immediate_agents, agents_already_configured );         
    FOREACH( Agent *a, my_agents )
    {
        // Give agents pointers to here and our coupling keys
        TRACE("Configuring agent ")(*a)("\n");
        a->AgentConfigure( this );             
    }

    // These are the ones our slaves should not configure
    Set<Agent *> agents_now_configured = SetUnion( agents_already_configured, my_agents ); 
    
    // Recurse into the slaves' configure
    FOREACH( Agent *a, my_agents )
    {
        if( SlaveAgent *sa = dynamic_cast<SlaveAgent *>(a) )
        {                        
            TRACE("Recursing to configure slave ")(*sa)("\n");
            sa->Configure(agents_now_configured);
        }
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
                           vector< TreePtr<Node> > *links ) const
{
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


bool Engine::Compare( const TreePtrInterface &x,
                              TreePtr<Node> pattern ) const
{
    INDENT("C");
    ASSERT( x );
    TRACE("Compare x=")(*x);
    TRACE(" pattern=")(*pattern);
    Agent *root_agent = Agent::AsAgent(pattern);
    //TRACE(**pcontext)(" @%p\n", pcontext);
           
    // Create the conjecture object we will use for this compare, and keep iterating
    // though different conjectures trying to find one that allows a match.
    Conjecture conj;
    bool r;
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
        TRACE("doing KEYING pass....\n");
        conj.PrepareForDecidedCompare();
        r = root_agent->DecidedCompare( x, true, conj );
        TRACE("KEYING pass result %d\n", r );
               
        if( r )
        {
            // ...now restrict the search according to the couplings
            TRACE("doing RESTRICTING pass....\n");
            conj.PrepareForDecidedCompare();
            r = root_agent->DecidedCompare( x, false, conj );
            TRACE("RESTRICTING pass result %d\n", r );
        }
        
        // If we got a match, we're done. If we didn't, and we've run out of choices, we're done.
        if( r )
            break; // Success
            
        if( !conj.Increment() )
            break; // Failure            
    }
    return r;
}


// Key for replace by just walking the tree (uniquised walk, not recursing into 
// the compare, search or replace fields of slaves) activating soft nodes and keying
// them.
void Engine::KeyReplaceNodes( TreePtr<Node> pattern ) const
{
    INDENT;
    TRACE("Walking replace pattern to key the soft nodes\n");
    
    FOREACH( Agent *a, my_agents )
        a->KeyReplace();
}


TreePtr<Node> Engine::BuildReplace( TreePtr<Node> pattern ) const
{   
    return Agent::AsAgent(pattern)->BuildReplace();
}


TreePtr<Node> Engine::ReplacePhase( TreePtr<Node> pattern ) const
{
    INDENT("R");
    
    // Do a two-pass process: first get the keys...
    TRACE("doing replace KEYING pass....\n");
    KeyReplaceNodes( pattern );
    TRACE("replace KEYING pass\n" );

    // Now replace according to the couplings
    TRACE("doing replace SUBSTITUTING pass....\n");
    TreePtr<Node> r = Agent::AsAgent(pattern)->BuildReplace();

    // TODO do an overlay, means *proot needs passing in here and this fn should be renamed.
    TRACE("replace SUBSTITUTING pass\n" );
    return r;
}


bool Engine::SingleCompareReplace( TreePtr<Node> *proot ) 
{
    INDENT;

    // Explicitly preserve the coupling keys structure - we do this instead
    // of clearing the keys in case the keys were set up in advance, as will
    // be the case if this is a slave.
    TRACE("Begin search\n");
    bool r = Compare( *proot, pattern );
    if( !r )
        return false;

    if( r == true )
    {
        TRACE("Search successful, now replacing\n");
        *proot = ReplacePhase( pattern );
    }

    return r;
}


// Perform search and replace on supplied program based
// on supplied patterns and couplings. Does search and replace
// operations repeatedly until there are no more matches. Returns how
// many hits we got.
int Engine::RepeatingCompareReplace( TreePtr<Node> *proot )
{
    INDENT;
    TRACE("begin RCR\n");
        
    bool r=false;
    int i=0;
    for(i=0; i<repetitions; i++) 
    {
        r = SingleCompareReplace( proot );
        TRACE("SCR result %d\n", r);        
        if( !r )
            break; // when the compare fails, we're done
    }
    
    if( r==true )
    {
        TRACE("Over %d reps\n",i); 
        if(rep_error)
            ASSERT(i<repetitions)
            ("Still getting matches after %d repetitions, may be repeating forever.\n"
             "Try using -rn%d to suppress this error\n", i, i);
    }
         
    TRACE("exiting\n");
    return i;
}

