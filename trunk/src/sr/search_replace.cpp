#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"
#include "helpers/simple_compare.hpp"
#include <list>

using namespace SR;

//#define STRACE
 
int CompareReplace::repetitions;
bool CompareReplace::rep_error;


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


CompareReplace::CompareReplace( TreePtr<Node> cp,
                                TreePtr<Node> rp,
                                bool im ) :
    is_configured( false ),								 
    is_master( im ),                                                 
    compare_pattern( cp ),
    replace_pattern( rp ),
    master_ptr( NULL )
{
}    
    
    
void CompareReplace::Configure( TreePtr<Node> cp,
                                TreePtr<Node> rp )
{
    INDENT;
    ASSERT(!is_configured)("Calling configure on already-configured ")(*this);
    TRACE("Entering SR::Configure on ")(*this)("\n");
    compare_pattern = cp;
    replace_pattern = rp;
	ConfigureImpl();
}
																
								
// The agents_already_configured argument is a set of agents that we should not
// configure because they were already configured by a master, and masters take 
// higher priority for configuration (so when an agent is reached from multiple
// engines, it's the most masterish one that "owns" it).
void CompareReplace::ConfigureImpl( const Set<Agent *> &agents_already_configured )
{
    INDENT;
    TRACE("Entering CR::ConfigureImpl on ")(*this)("\n");

    ASSERT(!is_configured)("Should not directly configure slave ")(*this);
    // TODO now that this operates per-slave instead of recursing through everything from the 
    // master, we need to obey the rule that slave patterns are complete before Configure, as
    // with master. Maybe an optional check on first invocation? And change all existing 
    // steps to comply.
    ASSERT( compare_pattern );
    
    // If only a search pattern is supplied, make the replace pattern the same
    // so they couple and then an overlay node can split them apart again.
    if( !replace_pattern )
        replace_pattern = compare_pattern;

    TRACE("Elaborating ")(string( *this ));

    // Walkers for compare and replace patterns that do not recurse beyond slaves (except via "through")
    UniqueWalkNoSlavePattern tsp(compare_pattern);
    UniqueWalkNoSlavePattern ss(replace_pattern);
    Set<Agent *> immediate_agents;
    FOREACH( TreePtr<Node> n, tsp )
        immediate_agents.insert( Agent::AsAgent(n) );
    FOREACH( TreePtr<Node> n, ss )
        immediate_agents.insert( Agent::AsAgent(n) );
	
    // Now configure all the ones we are allowed to configure        
    Set<Agent *> agents_to_configure = SetDifference( immediate_agents, agents_already_configured );         
    FOREACH( Agent *a, agents_to_configure )
    {
        TRACE("Configuring agent ")(*a)("\n");
        a->Configure( this, &coupling_keys );       
    }

    // These are the ones our slaves should not configure
    Set<Agent *> agents_configured = SetUnion( immediate_agents, agents_already_configured ); 
    
    // Recurse into the slaves' configure
	FOREACH( Agent *a, agents_to_configure )
	{
        // Give agents pointers to here and our coupling keys
        if( SlaveAgent *sa = dynamic_cast<SlaveAgent *>(a) )
        {            
            TRACE("Recursing to configure slave ")(*sa)("\n");
            sa->ConfigureImpl(agents_configured);
        }
    }
	
	// Configure all the other nodes - do this last so if a node
	// is reachable from both master and slave, the master config
	// takes priority (overwriting the slave config).
    FOREACH( Agent *a, agents_to_configure )
    {        
		TRACE("Checking agent ")(*a)("\n");
        AgentCommon *ac = dynamic_cast<AgentCommon *>(a);
        ASSERT(ac);
        ASSERT(ac->coupling_keys == &coupling_keys);
        ASSERT(ac->sr == this);
        if( CouplingSlave *csa = dynamic_cast<CouplingSlave *>(a) )
        {
		    // Provide Slaves (and potentially anything else derived from CouplingSlave) with slave-access to our coupling keys
            //TRACE("Found coupling slave in search pattern at %p\n", cs.get() );
            csa->SetCouplingsMaster( &coupling_keys ); 
        }
    }

    // Provide a back pointer for slaves (not sure why) TODO find out!
    FOREACH( TreePtr<Node> n, ss )
    {
		// Give agents pointers to here and our coupling keys
		TRACE("Configuring replace pattern ")(*n)("\n");		
        if( shared_ptr<CompareReplace> cr = dynamic_pointer_cast<CompareReplace>(n) )
        {		    
            cr->master_ptr = this; 
        }
	}

	is_configured = true;
} 


void CompareReplace::GetGraphInfo( vector<string> *labels, 
                                   vector< TreePtr<Node> > *links ) const
{
    labels->push_back("compare");
    links->push_back(compare_pattern);
    labels->push_back("replace");
    links->push_back(replace_pattern);
}


bool CompareReplace::Compare( const TreePtrInterface &x,
                              TreePtr<Node> pattern,
                              bool can_key ) const
{
    return Agent::AsAgent(pattern)->Compare( x, can_key );
}


void CompareReplace::FlushSoftPatternCaches( TreePtr<Node> pattern ) const
{
    UniqueWalk t(pattern);
    FOREACH( TreePtr<Node> n, t )
        if( shared_ptr<Flushable> ssp = dynamic_pointer_cast<Flushable>(n) )
            ssp->FlushCache();      
}


bool CompareReplace::IsMatch( TreePtr<Node> context,       
                              TreePtr<Node> root )
{
    pcontext = &context;
    ASSERT( compare_pattern );
    bool r = Agent::AsAgent(compare_pattern)->Compare( root, false );
    pcontext = NULL;
    return r == true;
}


// Key for replace by just walking the tree (uniquised walk, not recursing into 
// the compare, search or replace fields of slaves) activating soft nodes and keying
// them.
void CompareReplace::KeyReplaceNodes( TreePtr<Node> pattern ) const
{
    INDENT;
    TRACE("Walking replace pattern to key the soft nodes\n");
    
    UniqueWalkNoSlavePattern e(pattern);
    FOREACH( TreePtr<Node> pattern, e )	
	    Agent::AsAgent(pattern)->KeyReplace();
}


TreePtr<Node> CompareReplace::BuildReplace( TreePtr<Node> pattern ) const
{	
    return Agent::AsAgent(pattern)->BuildReplace();
}


TreePtr<Node> CompareReplace::ReplacePhase( TreePtr<Node> pattern ) const
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


bool CompareReplace::SingleCompareReplace( TreePtr<Node> *proot ) 
{
    INDENT;

    // Explicitly preserve the coupling keys structure - we do this instead
    // of clearing the keys in case the keys were set up in advance, as will
    // be the case if this is a slave.
    
        
    // Reset caches for this search run - the input tree will not change until 
    // the search is complete so stuff may be cached.
    FlushSoftPatternCaches( compare_pattern );
    
    TRACE("Begin search\n");
	bool r = Agent::AsAgent(compare_pattern)->Compare( *proot, true );
	if( !r )
		return false;

    if( r == true && replace_pattern )
    {
    	TRACE("Search successful, now replacing\n");
        *proot = ReplacePhase( replace_pattern );
    }

    // Clean up, to allow dead nodes to be deleted
    coupling_keys.Clear(); 

    return r;
}


// Perform search and replace on supplied program based
// on supplied patterns and couplings. Does search and replace
// operations repeatedly until there are no more matches. Returns how
// many hits we got.
int CompareReplace::RepeatingCompareReplace( TreePtr<Node> *proot )
{
    INDENT;
    if( !master_ptr )
        dirty_grass.clear(); 

    TRACE("begin RCR\n");
        
    bool r=false;
    int i=0;
    for(i=0; i<repetitions; i++) 
    {
    	r = SingleCompareReplace( proot );
    	TRACE("SCR result %d\n", r);        
    	if( !r )
            break; // when the compare fails, we're done
        TRACE("Dirty grass:");
        FOREACH( TreePtr<Node> n, dirty_grass )
            TRACE(" ")(*n);
        TRACE("\n");     
       // Validate()( *pcontext, proot );
    }
    
    if( r==true )
    {
        TRACE("Over %d reps\n",i); 
        if(rep_error)
            ASSERT(i<repetitions)
            ("Still getting matches after %d repetitions, may be repeating forever.\n"
             "Try using -rn%d to suppress this error\n", i, i);
    }
         
    if( !master_ptr )
        dirty_grass.clear(); 
         
    TRACE("exiting\n");
    return i;
}


// Do a search and replace based on patterns stored in our members
void CompareReplace::operator()( TreePtr<Node> c, TreePtr<Node> *proot )
{
    ASSERT( is_configured )(*this)(" has not been configured");
    INDENT("");
    TRACE("Enter S&R instance ")(*this);
    ASSERT( compare_pattern )("CompareReplace (or SearchReplace) object was not configured before invocation.\n"
                              "Either call Configure() or supply pattern arguments to constructor.\n"
                              "Thank you for taking the time to read this message.\n");
    
    // If the initial root and context are the same node, then arrange for the context
    // to follow the root node as we modify it (in SingleSearchReplace()). This ensures
    // new declarations can be found in slave searches. 
    //
    // TODO but does not work for sub-slaves, because the first level slave's proot
    // is not the same as pcontext. When slave finishes a singe CR, only the locally-created
    // *proot is updated, not the top level *proot or *pcontext, so the updates do not appear 
    // in the context until the first level slave completes, the local *proot is copied over
    // the TL *proot (and hence *pcontext) and the mechanism described here kicks in
    //  
    // We could get the
    // same effect by taking the context as a reference, but leave it like this for now.
    // If *proot is under context, then we're OK as long as proot points to the actual
    // tree node - then the walk at context will follow the new *proot pointer and get
    // into the new subtree.
    if( c == *proot )
	pcontext = proot;
    else
	pcontext = &c;
        
    (void)RepeatingCompareReplace( proot );   

    pcontext = NULL; // just to avoid us relying on the context outside of a search+replace pass
}


SearchReplace::SearchReplace( TreePtr<Node> sp,
                              TreePtr<Node> rp,
                              bool im ) :
    CompareReplace( sp, rp, im )                              
{
}


void SearchReplace::ConfigureImpl( const Set<Agent *> &agents_already_configured )
{                    
    INDENT;
    ASSERT( compare_pattern ); // a search pattern is required to configure the engine
	TRACE("Entering SR::Configure on ")(*this)("\n");

    // Make a non-rooted search and replace (ie where the base of the search pattern
    // does not have to be the root of the whole program tree).
    // Insert a Stuff node as root of the search pattern
    // Needs to be Node, because we don't want pre-restriction action here (if we're a slave
    // we got pre-restricted already.
    MakePatternPtr< Stuff<Node> > stuff;

    if( !replace_pattern )
    {
        // Search and replace immediately coupled, insert Stuff, but don't bother
        // with the redundant Overlay.
        stuff->terminus = compare_pattern;
		compare_pattern = stuff;
    }
    else
    {
        // Classic search and replace with separate replace pattern, we can use
        // an Overlay node to overwrite the replace pattern at replace time.
        
        // Insert a Stuff node as root of the replace pattern
        MakePatternPtr< Overlay<Node> > overlay;
        stuff->terminus = overlay;
        overlay->through = compare_pattern;
        overlay->overlay = replace_pattern;
        compare_pattern = stuff;
		replace_pattern = stuff;
    }

	CompareReplace::ConfigureImpl( agents_already_configured );	
}


void SearchReplace::GetGraphInfo( vector<string> *labels, 
                                  vector< TreePtr<Node> > *links ) const
{
    // Find the original patterns
    TreePtr< Stuff<Node> > stuff = dynamic_pointer_cast< Stuff<Node> >(compare_pattern);
    ASSERT( stuff );
    TreePtr< Overlay<Node> > overlay = dynamic_pointer_cast< Overlay<Node> >(stuff->terminus);
    if( overlay )
    {        
        labels->push_back("search");    
        links->push_back(overlay->through);
        labels->push_back("replace");
        links->push_back(overlay->overlay);
    }
    else
    {
        labels->push_back("search_replace");    
        links->push_back(stuff->terminus);
    }
}


