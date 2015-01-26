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
        if( shared_ptr<SlaveBase> sb = dynamic_pointer_cast<SlaveBase>( n ) )
        {
            // it's a slave, so set up a container containing only "through", not "compare" or "replace"
            shared_ptr< Sequence<Node> > seq( new Sequence<Node> );
            seq->push_back( sb->GetThrough() );
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
																
								
void CompareReplace::ConfigureImpl()
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

    TRACE("Elaborating ")(string( *this ))(" at %p\n", this);

    // Walkers for compare and replace patterns that do not recurse beyond slaves
    UniqueWalkNoSlavePattern tsp(compare_pattern);
    UniqueWalkNoSlavePattern ss(replace_pattern);

	// Collect together all the first-level slaves
    Set< OOStd::SharedPtr<SlaveBase> > immediate_slaves;
    FOREACH( TreePtr<Node> n, tsp )
    {        
		if( shared_ptr<SlaveBase> sb = dynamic_pointer_cast<SlaveBase>(n) )
		{
		    immediate_slaves.insert( sb );
		}
    }
    FOREACH( TreePtr<Node> n, ss )
    {
		if( shared_ptr<SlaveBase> sb = dynamic_pointer_cast<SlaveBase>(n) )
		{
		    immediate_slaves.insert( sb );
		}
	}
	
	// Recurse into the slaves' configure
	FOREACH( shared_ptr<SlaveBase> s, immediate_slaves )
	{
	    TRACE("Recursing to configure slave ")(*s)("\n");
	    s->ConfigureImpl();
	}
	
	// Configure all the other nodes - do this last so if a node
	// is reachable from both master and slave, the master config
	// takes priority (overwriting the slave config).
    FOREACH( TreePtr<Node> n, tsp )
    {        
		// Give agents pointers to here and our coupling keys
		//TRACE("Configuring search pattern ")(*n)("\n");
        Agent::AsAgent(n)->Configure( this, &coupling_keys );		
        if( shared_ptr<StuffBase> sb = dynamic_pointer_cast<StuffBase>(n) )
        {
		    // Provide Stuff nodes with slave-access to our master coupling keys. They are not allowed to add keys,
			// only use the ones that are already there. Note that recurse restriction is done using a local CompareReplace object.
            TRACE("Found stuff@%p, rr@%p\n", sb.get(), sb->recurse_restriction.get());
            sb->recurse_comparer.coupling_keys.SetMaster( &coupling_keys ); 
            sb->recurse_comparer.compare_pattern = sb->recurse_restriction; // TODO could move into a Stuff node constructor if there was one
        }
		
        if( shared_ptr<CouplingSlave> cs = dynamic_pointer_cast<CouplingSlave>(n) )
        {
		    // Provide Slaves (and potentially anything else derived from CouplingSlave) with slave-access to our coupling keys
            TRACE("Found coupling slave in search pattern at %p\n", cs.get() );
            cs->SetCouplingsMaster( &coupling_keys ); 
        }
    }

    FOREACH( TreePtr<Node> n, ss )
    {
		// Give agents pointers to here and our coupling keys
		//TRACE("Configuring replace pattern ")(*n)("\n");		
        Agent::AsAgent(n)->Configure( this, &coupling_keys );		

        if( shared_ptr<CouplingSlave> cs = dynamic_pointer_cast<CouplingSlave>(n) )
        {
		    // As above, but in the replace pattern
            TRACE("Found coupling slave in replace pattern at %p\n", cs.get() );
            cs->SetCouplingsMaster( &coupling_keys ); 
        }
        if( shared_ptr<CompareReplace> cr = dynamic_pointer_cast<CompareReplace>(n) )
        {
		    // Provide a back pointer for slaves (not sure why)
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


// Helper for DecidedCompare that does the actual match testing work for the children and recurses.
// Also checks for soft matches.
bool CompareReplace::DecidedCompare( const TreePtrInterface &x,
									   TreePtr<Node> pattern,
									   bool can_key,
    								   Conjecture &conj ) const
{
    return Agent::AsAgent(pattern)->DecidedCompare( x, pattern, can_key, conj );
}


bool CompareReplace::MatchingDecidedCompare( const TreePtrInterface &x,
		                                     TreePtr<Node> pattern,
		                                     bool can_key,
		                                     Conjecture &conj ) const
{
    INDENT;
    bool r;

    if( can_key )
        coupling_keys.Clear();

    // Only key if the keys are already set to KEYING (which is 
    // the initial value). Keys could be RESTRICTING if we're under
    // a SoftNot node, in which case we only want to restrict.
    if( can_key )
    {
        // Do a two-pass matching process: first get the keys...
        TRACE("doing KEYING pass....\n");
        conj.PrepareForDecidedCompare();
        r = DecidedCompare( x, pattern, true, conj );
        TRACE("KEYING pass result %d\n", r );
        if( !r )
            return false;                  // Save time by giving up if no match found
    }
    
    // Now restrict the search according to the couplings
    TRACE("doing RESTRICTING pass....\n");
    conj.PrepareForDecidedCompare();
    r = DecidedCompare( x, pattern, false, conj );
    TRACE("RESTRICTING pass result %d\n", r );
    if( !r )
        return false;	               // Save time by giving up if no match found

    // Do not revert match keys if we were successful - keep them for replace
    // and any slave search and replace operations we might do.
    return true;
}


void CompareReplace::FlushSoftPatternCaches( TreePtr<Node> pattern ) const
{
    UniqueWalk t(pattern);
    FOREACH( TreePtr<Node> n, t )
        if( shared_ptr<Flushable> ssp = dynamic_pointer_cast<Flushable>(n) )
            ssp->FlushCache();      
}


bool CompareReplace::Compare( const TreePtrInterface &x,
		                        TreePtr<Node> pattern,
								bool can_key ) const
{
    INDENT("C");
    ASSERT( x );
    ASSERT( pattern );
	TRACE("Compare @%p x=", this)(*x);
    TRACE(" pattern=")(*pattern);
    TRACE(" can_key=%d \n", (int)can_key);
    //TRACE(**pcontext)(" @%p\n", pcontext);
    
    // easy optimisation - also important because couplings do this a lot
    if( x == pattern )
        return true;
    
    // Reset caches for this search run - the input tree will not change until 
    // the search is complete so stuff may be cached.
    FlushSoftPatternCaches( pattern );
    
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
		r = MatchingDecidedCompare( x, pattern, can_key, conj );

		// If we got a match, we're done. If we didn't, and we've run out of choices, we're done.
		if( r )
		    break; // Success
		    
		if( !conj.Increment() )
		    break; // Failure
	}
	return r;
}


bool CompareReplace::IsMatch( TreePtr<Node> context,       
                              TreePtr<Node> root )
{
    pcontext = &context;
    ASSERT( compare_pattern );
    bool r = Compare( root, compare_pattern, false );
    pcontext = NULL;
    return r == true;
}


TreePtr<Node> CompareReplace::DuplicateNode( TreePtr<Node> source,
    		                                  bool force_dirty ) const
{
    INDENT;

	// Make the new node (destination node)
    shared_ptr<Cloner> dup_dest = source->Duplicate(source);
    TreePtr<Node> dest = dynamic_pointer_cast<Node>( dup_dest );
    ASSERT(dest);

    bool source_dirty = GetOverallMaster()->dirty_grass.find( source ) != GetOverallMaster()->dirty_grass.end();
    if( force_dirty || // requested by caller
        source_dirty ) // source was dirty
    {
        //TRACE("dirtying ")(*dest)(" force=%d source=%d (")(*source)(")\n", force_dirty, source_dirty);        
        GetOverallMaster()->dirty_grass.insert( dest );
    }
    
    return dest;    
}    		                                          


TreePtr<Node> CompareReplace::DuplicateSubtree( TreePtr<Node> source,
		                                        TreePtr<Node> source_terminus,
												 TreePtr<Node> dest_terminus ) const
{
	INDENT;
	ASSERT( source );
    if( source_terminus )
		ASSERT( dest_terminus );
     // Under substitution, we should be duplicating a subtree of the input
    // program, which should not contain any special nodes
    ASSERT( !(dynamic_pointer_cast<SpecialBase>(source)) )
          ("Cannot duplicate special node ")(*source);
	
	// If source_terminus and dest_terminus are supplied, substitute dest_terminus node
    // in place of all copes of source terminus (directly, without duplicating).
	if( source_terminus && source == source_terminus ) 
		return dest_terminus;

	// Make a new node, since we're substituting, preserve dirtyness	    
    TreePtr<Node> dest = DuplicateNode( source, false );

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of source, and itemises over that type. dest must
    // be dynamic_castable to source's type.
    vector< Itemiser::Element * > keynode_memb = source->Itemise();
    vector< Itemiser::Element * > dest_memb = dest->Itemise(); 

    TRACE("Duplicating %d members source=", dest_memb.size())(*source)(" dest=")(*dest)("\n");
    // Loop over all the members of source (which can be a subset of dest)
    // and for non-NULL members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<dest_memb.size(); i++ )
    {
    	//TRACE("Duplicating member %d\n", i );
        ASSERT( keynode_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        
        if( ContainerInterface *keynode_con = dynamic_cast<ContainerInterface *>(keynode_memb[i]) )                
        {
            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_memb[i]);

            dest_con->clear();

            //TRACE("Duplicating container size %d\n", keynode_con->size() );
	        FOREACH( const TreePtrInterface &p, *keynode_con )
	        {
		        ASSERT( p ); // present simplified scheme disallows NULL
		        //TRACE("Duplicating ")(*p)("\n");
		        TreePtr<Node> n = DuplicateSubtree( p, source_terminus, dest_terminus );
  	            //TRACE("Normal element, inserting ")(*n)(" directly\n");
		        dest_con->insert( n );
	        }
        }            
        else if( TreePtrInterface *keynode_ptr = dynamic_cast<TreePtrInterface *>(keynode_memb[i]) )
        {
            //TRACE("Duplicating node ")(*keynode_ptr)("\n");
            TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( *keynode_ptr )("source should be non-NULL");
            *dest_ptr = DuplicateSubtree( *keynode_ptr, source_terminus, dest_terminus );
            ASSERT( *dest_ptr );
            ASSERT( TreePtr<Node>(*dest_ptr)->IsFinal() );            
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    
    return dest;
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
	{
	    //TRACE(*pattern)("\n");
	    TreePtr<Node> key = pattern;
	    if( shared_ptr<SoftReplacePattern> srp = dynamic_pointer_cast<SoftReplacePattern>( pattern ) )
	    {
            //TRACE("Soft replace pattern not keyed, ")(*pattern)("\n");

            // Call the soft pattern impl 
            key = srp->DuplicateSubtree( this );
            if( key )
            {            
                // Allow this to key a coupling. 
                coupling_keys.DoKey( key, pattern );
            } 
	    }
    }
}


TreePtr<Node> CompareReplace::BuildReplace( TreePtr<Node> pattern ) const
{	
    return Agent::AsAgent(pattern)->BuildReplace( pattern );
}


TreePtr<Node> CompareReplace::ReplacePhase( TreePtr<Node> pattern ) const
{
    INDENT("R");
    
    // Do a two-pass process: first get the keys...
    TRACE("doing replace KEYING pass....\n");
    //(void)DuplicateSubtree( pattern, true );
    KeyReplaceNodes( pattern );
    TRACE("replace KEYING pass\n" );

    // Now replace according to the couplings
    TRACE("doing replace SUBSTITUTING pass....\n");
    TreePtr<Node> r = BuildReplace( pattern );

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
    
	TRACE("%p Begin search\n", this);
	bool r = Compare( *proot, compare_pattern, true );
	if( !r )
		return false;

    if( r == true && replace_pattern )
    {
    	TRACE("%p Search successful, now replacing\n", this);
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

    TRACE("begin RCR %p\n", this);
        
    bool r=false;
    int i=0;
    for(i=0; i<repetitions; i++) 
    {
    	r = SingleCompareReplace( proot );
    	TRACE("%p SCR result %d\n", this, r);        
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
         
    TRACE("%p exiting\n", this);
    return i;
}


// Do a search and replace based on patterns stored in our members
void CompareReplace::operator()( TreePtr<Node> c, TreePtr<Node> *proot )
{
    ASSERT( is_configured )(*this)(" has not been configured");
    INDENT("");
    TRACE("Enter S&R instance ")(*this)(" at %p\n", this);
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


void SearchReplace::ConfigureImpl()
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

	CompareReplace::ConfigureImpl();	
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
    
    
shared_ptr<ContainerInterface> StuffBase::GetContainerInterface( TreePtr<Node> x )

{
    Filter *rf = NULL;
    if( recurse_restriction )
    {
        ASSERT( recurse_comparer.compare_pattern )("Stuff node in slave must be initialised before slave (at %p)\n", this);     
        rf = &recurse_comparer;
    }
    
    if( one_level )
        return shared_ptr<ContainerInterface>( new FlattenNode( x ) );
    else
        return shared_ptr<ContainerInterface>( new Walk( x, NULL, rf ) );
}


bool StarBase::MatchRange( const CompareReplace *sr,
                           ContainerInterface &range,
                           bool can_key )
{
    INDENT;
    // this is an abnormal context (which of the program nodes
    // in the range should key the pattern?) so just wave keying
    // pass right on through.
    if( can_key )
        return true;
                
    TreePtr<Node> p = GetPattern();
    if( p )
    {
        TRACE("MatchRange pattern\n");
        // Apply pattern restriction - will be at least as strict as pre-restriction
        FOREACH( TreePtr<Node> x, range )
        {
            bool r = sr->Compare( x, p, false );
            if( !r )
                return false;
        }
    }
    else
    {
        TRACE("MatchRange pre-res\n");
        // No pattern, so just use pre-restrictions
        FOREACH( TreePtr<Node> x, range )
        {
            if( !IsLocalMatch( x.get()) )
                return false;
        }
    }     
    TRACE("done\n");
    return true;   
}                       



shared_ptr<ContainerInterface> AnyNodeBase::GetContainerInterface( TreePtr<Node> x )
{ 
    TRACE("FlattenNodeing an AnyNode at ")(*x)(": { ");
    FlattenNode f( x );
    FOREACH( TreePtr<Node> pn, f )
        {TRACE(*pn)(" ");}
    TRACE("}\n");
        
    return shared_ptr<ContainerInterface>( new FlattenNode( x ) );
}


void CompareReplace::Test()
{
#if 0
    CompareReplace sr = CompareReplace( MakeTreePtr<Nop>(), MakeTreePtr<Nop>() ); // TODO we're only using compare side

    {
        // single node with topological wildcarding
        TreePtr<Void> v(new Void);
        ASSERT( sr.Compare( v, v ) == true );
        TreePtr<Boolean> b(new Boolean);
        ASSERT( sr.Compare( v, b ) == false );
        ASSERT( sr.Compare( b, v ) == false );
        TreePtr<Type> t(new Type);
        ASSERT( sr.Compare( v, t ) == true );
        ASSERT( sr.Compare( t, v ) == false );
        ASSERT( sr.Compare( b, t ) == true );
        ASSERT( sr.Compare( t, b ) == false );
        
        // node points directly to another with TC
        TreePtr<Pointer> p1(new Pointer);
        p1->destination = v;
        ASSERT( sr.Compare( p1, b ) == false );
        ASSERT( sr.Compare( p1, p1 ) == true );
        TreePtr<Pointer> p2(new Pointer);
        p2->destination = b;
        ASSERT( sr.Compare( p1, p2 ) == false );
        p2->destination = t;
        ASSERT( sr.Compare( p1, p2 ) == true );
        ASSERT( sr.Compare( p2, p1 ) == false );
    }
    
    {
        // string property
        TreePtr<SpecificString> s1( new SpecificString("here") );
        TreePtr<SpecificString> s2( new SpecificString("there") );
        ASSERT( sr.Compare( s1, s1 ) == true );
    //    ASSERT( sr.Compare( s1, s2 ) == false ); 
    //TODO seems to get a crash freeing s1 or s2 if uncommented, latent problem in SpecificString?
    }    
    
    {
        // int property
        TreePtr<SpecificInteger> i1( new SpecificInteger(3) );
        TreePtr<SpecificInteger> i2( new SpecificInteger(5) );
        TRACE("  %s %s\n", ((llvm::APSInt)*i1).toString(10).c_str(), ((llvm::APSInt)*i2).toString(10).c_str() );
        ASSERT( sr.Compare( i1, i1 ) == true );
        ASSERT( sr.Compare( i1, i2 ) == false );
    }    

    
    {
        // node with sequence, check lengths 
        TreePtr<Compound> c1( new Compound );
        ASSERT( sr.Compare( c1, c1 ) == true );
        TreePtr<Nop> n1( new Nop );
        c1->statements.push_back( n1 );
        ASSERT( sr.Compare( c1, c1 ) == true );
        TreePtr<Nop> n2( new Nop );
        c1->statements.push_back( n2 );
        ASSERT( sr.Compare( c1, c1 ) == true );
        TreePtr<Compound> c2( new Compound );
        ASSERT( sr.Compare( c1, c2 ) == false );
        TreePtr<Nop> n3( new Nop );
        c2->statements.push_back( n3 );
        ASSERT( sr.Compare( c1, c2 ) == false );
        TreePtr<Nop> n4( new Nop );
        c2->statements.push_back( n4 );
        ASSERT( sr.Compare( c1, c2 ) == true );
        TreePtr<Nop> n5( new Nop );
        c2->statements.push_back( n5 );
        ASSERT( sr.Compare( c1, c2 ) == false );
    }

    {
        // node with sequence, TW 
        TreePtr<Compound> c1( new Compound );
        TreePtr<Nop> n1( new Nop );
        c1->statements.push_back( n1 );
        TreePtr<Compound> c2( new Compound );
        TreePtr<Statement> s( new Statement );
        c2->statements.push_back( s );
        ASSERT( sr.Compare( c1, c2 ) == true );
        ASSERT( sr.Compare( c2, c1 ) == false );
    }

    {
        // topological with extra member in target node
        /* gone obsolete with tree changes TODO un-obsolete
        TreePtr<Label> l( new Label );
        TreePtr<Public> p1( new Public );
        l->access = p1;

        TreePtr<LabelIdentifier> li( new LabelIdentifier );
        li->name = "mylabel";
        l->identifier = li;
        TreePtr<Declaration> d( new Declaration );
        TreePtr<Public> p2( new Public );
        d->access = p2;
        ASSERT( sr.Compare( l, d ) == true );
        ASSERT( sr.Compare( d, l ) == false );
        TreePtr<Private> p3( new Private );
        d->access = p3;
        ASSERT( sr.Compare( l, d ) == false );
        ASSERT( sr.Compare( d, l ) == false );
        TreePtr<AccessSpec> p4( new AccessSpec );
        d->access = p4;
        ASSERT( sr.Compare( l, d ) == true );
        ASSERT( sr.Compare( d, l ) == false );
        */
    }
#endif
}
/*
#ifdef STRACE
    TRACE("DuplicateSubtree pattern={");
	    Walk w(pattern);
	    bool first=true;
	    FOREACH( TreePtr<Node> n, w )
	    {
	    	if( !first )
	    		TRACE(", ");
	    	if( n )
                TRACE( *n )(":%p", n.get());
            else
                TRACE("NULL");
	    	first=false;
	    }
	    TRACE("}\n"); // TODO put this in as a common utility somewhere
#endif
*/
