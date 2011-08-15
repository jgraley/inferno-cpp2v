#include "search_replace.hpp"
#include "conjecture.hpp"
#include "common/hit_count.hpp"

//#define STRACE

int CompareReplace::repetitions;
bool CompareReplace::rep_error;


/** Walks the tree, avoiding the "search"/"compare" and "replace" members of slaves
    but still recurses through the "through" member. Therefore, it visits all the
    nodes at the same slave level as the root. Based on Traverse, so each node only
    visited once. */
class TraverseNoSlavePattern_iterator : public Traverse::iterator
{
public:
    TraverseNoSlavePattern_iterator( TreePtr<Node> &root ) : Traverse::iterator(root) {}        
    TraverseNoSlavePattern_iterator() : Traverse::iterator() {}
	virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const
	{
   	    return shared_ptr<TraverseNoSlavePattern_iterator>( new TraverseNoSlavePattern_iterator(*this) );
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
            // it's not a slave, so proceed as for Traverse
            return Traverse::iterator::GetChildContainer(n);
        }
    }
};

typedef ContainerFromIterator< TraverseNoSlavePattern_iterator, TreePtr<Node> > TraverseNoSlavePattern;

// Master constructor remembers search pattern, replace pattern and any supplied couplings as required
CompareReplace::CompareReplace( TreePtr<Node> cp,
                                TreePtr<Node> rp,
                                bool im ) :
    is_master( im ),                                                 
    compare_pattern( NULL ),
    replace_pattern( NULL )
{
    if( cp )
        Configure( cp, rp );
}    
    
    
void CompareReplace::Configure( TreePtr<Node> cp,
                                TreePtr<Node> rp )
{
    ASSERT( cp );
    
    // If only a search pattern is supplied, make the replace pattern the same
    // so they couple and then an overlay node can split them apart again.
    if( !rp )
        rp = cp;
        
    TRACE("Elaborating for instance at %p\n", this);
    // Fill in fields on the stuff nodes, but not in slaves
    TraverseNoSlavePattern tsp(cp);
    FOREACH( TreePtr<Node> n, tsp )
    {        
        if( TreePtr<StuffBase> sb = dynamic_pointer_cast<StuffBase>(n) )
        {
            TRACE("Elaborating Stuff@%p, rr@%p\n", sb.get(), sb->recurse_restriction.get());
            sb->recurse_comparer.coupling_keys.SetMaster( &coupling_keys ); 
            sb->recurse_comparer.compare_pattern = sb->recurse_restriction; // TODO could move into a Stuff node constructor if there was one
        }
    }

    // look for first-level slaves. Set their couplings master pointer to point
    // to our couplings. 
    TraverseNoSlavePattern ss(rp);
    FOREACH( TreePtr<Node> n, ss )
    {
        if( TreePtr<CouplingSlave> cs = dynamic_pointer_cast<CouplingSlave>(n) )
        {
            cs->SetCouplingsMaster( &coupling_keys ); 
        }
    }

    // Finally store the patterns in the object (we are now ready to transform)
    compare_pattern = cp;
    replace_pattern = rp;
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
bool CompareReplace::DecidedCompare( TreePtr<Node> x,
									   TreePtr<Node> pattern,
									   bool can_key,
    								   Conjecture &conj,
    								   Conjecture::Choice *gc,
    								   int go ) const
{
    INDENT;
	ASSERT( x ); // Target must not be NULL
	if( !pattern )    // NULL matches anything in search patterns (just to save typing)
		return true;
    TRACE("Comparing x=")
         (*x)
         (" with pattern=")
         (*pattern)
         ("\n");
    
	// Check whether the present node matches. Do this for all nodes: this will be the local
	// restriction for normal nodes and the pre-restriction for special nodes (based on
	// how IsLocalMatch() has been overridden.
	if( !pattern->IsLocalMatch(x.get()) )
	{
		return false;
    }
    
	if( shared_ptr<SoftSearchPattern> ssp = dynamic_pointer_cast<SoftSearchPattern>(pattern) )
    {
    	// Hand over to any soft search functionality in the search pattern node
    	bool r = ssp->DecidedCompare( this, x, can_key, conj );
    	if( !r )
    		return false;
    }
    else if( dynamic_pointer_cast<SoftReplacePattern>(pattern) )
    {
    	// No further restriction beyond the pre-restriction for these nodes when searching.
    }
    else if( TreePtr<SearchContainerBase> sc_pattern = dynamic_pointer_cast<SearchContainerBase>(pattern) )
    {
    	// Invoke stuff node compare
    	// Check whether the present node matches
    	bool r = DecidedCompare( x, sc_pattern, can_key, conj );
        if( !r )
            return false;
    }
    else if( TreePtr<GreenGrassBase> green_pattern = dynamic_pointer_cast<GreenGrassBase>(pattern) )
    {
        // Restrict so that everything in the input program under here must be "green grass"
        // ie unmodified by previous replaces in this RepeatingSearchReplace() run.
        Expand w( x );
        FOREACH( TreePtr<Node> p, w )
            if( dirty_grass.find( p ) != dirty_grass.end() )
                return false;
        // Normal matching for the through path
        bool r = DecidedCompare( x, green_pattern->GetThrough(), can_key, conj );
        if( !r )
            return false;
    }
    else if( TreePtr<OverlayBase> op = dynamic_pointer_cast<OverlayBase>(pattern) )
    {
        // When DoOverlay node seen duriung search, just forward through the "base" path
        bool r = DecidedCompare( x, op->GetThrough(), can_key, conj );
        if( !r )
            return false;
    }
    else if( TreePtr<SlaveBase> sp = dynamic_pointer_cast<SlaveBase>(pattern) )
    {
        // When a slave node seen duriung search, just forward through the "base" path
        bool r = DecidedCompare( x, sp->GetThrough(), can_key, conj );
        if( !r )
            return false;
    }
    else
    {
		// Recurse through the children. Note that the itemiser internally does a
		// dynamic_cast onto the type of pattern, and itemises over that type. x must
		// be dynamic_castable to pattern's type.
		vector< Itemiser::Element * > pattern_memb = pattern->Itemise();
		vector< Itemiser::Element * > x_memb = pattern->Itemise( x.get() );   // Get the members of x corresponding to pattern's class
		ASSERT( pattern_memb.size() == x_memb.size() );
		for( int i=0; i<pattern_memb.size(); i++ )
		{
			bool r;
			ASSERT( pattern_memb[i] )( "itemise returned null element");
			ASSERT( x_memb[i] )( "itemise returned null element");

			if( SequenceInterface *pattern_seq = dynamic_cast<SequenceInterface *>(pattern_memb[i]) )
			{
				SequenceInterface *x_seq = dynamic_cast<SequenceInterface *>(x_memb[i]);
				ASSERT( x_seq )( "itemise for x didn't match itemise for pattern");
				TRACE("Member %d is Sequence, x %d elts, pattern %d elts\n", i, x_seq->size(), pattern_seq->size() );
				r = DecidedCompare( *x_seq, *pattern_seq, can_key, conj );
			}
			else if( CollectionInterface *pattern_col = dynamic_cast<CollectionInterface *>(pattern_memb[i]) )
			{
				CollectionInterface *x_col = dynamic_cast<CollectionInterface *>(x_memb[i]);
				ASSERT( x_col )( "itemise for x didn't match itemise for pattern");
				TRACE("Member %d is Collection, x %d elts, pattern %d elts\n", i, x_col->size(), pattern_col->size() );
				r = DecidedCompare( *x_col, *pattern_col, can_key, conj );
			}
			else if( TreePtrInterface *pattern_ptr = dynamic_cast<TreePtrInterface *>(pattern_memb[i]) )
			{
				TreePtrInterface *x_ptr = dynamic_cast<TreePtrInterface *>(x_memb[i]);
				ASSERT( x_ptr )( "itemise for x didn't match itemise for pattern");
				TRACE("Member %d is TreePtr, pattern ptr=%p\n", i, pattern_ptr->get());
				r = DecidedCompare( *x_ptr, TreePtr<Node>(*pattern_ptr), can_key, conj );
			}
			else
			{
				ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
			}

			if( !r )
				return false;
		}
    }
   
    // Check couplings after everything else because they can get keyed during keying pass
    // and then seem to match during restricting pass. A key match is not a guarantee 
    // because when it keyed some coupled node below it may not have been keyed yet 
    // and so their checks would not have occured and hte check would not be strict 
    // enough. Perhaps keys can be "concrete" when all the couplings below them have
    // been checked as matching?
    if( TreePtr<Node> keynode = coupling_keys.GetCoupled( pattern ) )
        if( Compare( x, keynode ) == false )
            return false;
	
	if( can_key )
        coupling_keys.DoKey( x, pattern, gc, go );	

    return true;
}

                                                                                                                        
// TODO support SearchContainerBase(ie Stuff nodes) here and below inside the container.
// Behaviour would be to try the container at each of the nodes matched by the star, and if
// one match is found we have a hit (ie OR behaviour). I think this results in 3 decisions
// for sequences as opposed to Star and Stuff, which are one decision each. They are:
// first: How many elements to match (as with Star)
// second: Which of the above to try for container match
// third: Which element of the SearchContainer to try 
bool CompareReplace::DecidedCompare( SequenceInterface &x,
		                               SequenceInterface &pattern,
		                               bool can_key,
		                               Conjecture &conj ) const
{
    INDENT;
	// Attempt to match all the elements between start and the end of the sequence; stop
	// if either pattern or subject runs out.
	ContainerInterface::iterator xit = x.begin();
	ContainerInterface::iterator pit, npit, nnpit;
	Conjecture::Choice *governing_choice = NULL;
	int governing_offset;

	for( pit = pattern.begin(); pit != pattern.end(); ++pit )
	{
		ASSERT( xit == x.end() || *xit );

		// Get the next element of the pattern
		TreePtr<Node> pe( *pit );
		ASSERT( pe );
		npit=pit;
		++npit;
	    if( TreePtr<StarBase> ps = dynamic_pointer_cast<StarBase>(pe) )
	    {
			// We have a Star type wildcard that can match multiple elements.
			// Remember where we are - this is the beginning of the subsequence that
	    	// potentially matches the Star.
	    	ContainerInterface::iterator xit_begin_star = xit;

	    	// Star always matches at the end of a sequence, so we only need a 
	    	// decision when there are more elements left
	    	if( npit == pattern.end() )
	    	{
	    		xit = x.end(); // match all remaining members of x; jump to end
	    	}
	    	else
	    	{
	    		TRACE("Pattern continues after star\n");

	    		// Star not at end so there is more stuff to match; ensure not another star
	    	//	ASSERT( !dynamic_pointer_cast<StarBase>(TreePtr<Node>(*npit)) )
	    	//	      ( "Not allowed to have two neighbouring Star elements in search pattern Sequence");

		    	// Decide how many elements the current * should match, using conjecture. Jump forward
	    		// that many elements, to the element after the star. We need to use the special 
	    		// interface to Conjecture because the iterator we get is itself an end to the 
	    		// range matched by the star, and so x.end() is a legitimate choice for ss.end()
	    		// So allow the Conjecture class to give us two ends, and only give up on the second.
		    	Conjecture::Choice *current_choice = conj.GetChoicePtr();
#if 0
		    	// Optimisation for couplings in sequences: is some future pattern node coupled?
		    	bool do_optimise = false;
		    	int lad=0;
		    	shared_ptr<Key> lakey;
		    	for( ContainerInterface::iterator lapit=npit; lapit!=pattern.end(); ++lapit )
		    	{
		    	    lad++;
		        	if( dynamic_pointer_cast<StarBase>(TreePtr<Node>(*lapit)) )
		        	    break; // Stop at stars, do not use them or go past them (TODO use stars, need to uncomment some code here and give them governing_choice */
		        	lakey = coupling_keys.GetKey( *lapit );
		        	// Did we find a node that has been keyed?
		        	if( lakey && lakey->root )
		        	{
		        	    TRACE("Maybe optimise lad=%d *pit=", lad)(**pit)(" *lapit=")(**lapit)
		        	         (" lakey->governing_choice=%p current_choice=%p", lakey->governing_choice, current_choice)
		        	         (" lakey->pattern=")(*lakey->replace_pattern)(" lakey->root=")(*lakey->root)("\n");
		        	    // Did the node have a governing_choice set on it (i.e. was it goverend by a Star node decision?
		        	    // AND is it not the current decision (we leave the original decision unoptimised)
		        	    if( lakey->governing_choice &&
		        	        lakey->governing_choice != current_choice )
		        	    {
		        	        TRACE("governing_offset=%d\n", governing_offset);
                    	    do_optimise=true;
		                }
		            }
		    	}		    			    	

		    	if( do_optimise )
		    	{
	        	    TreePtr<Node> lakeynode = lakey->root;
	        	    // Is it a SubCollection? then use the first node out of subcollection as the key
		    	    if( TreePtr<SubCollection> lakeysc = dynamic_pointer_cast<SubCollection>(lakey->root) )
    	    	        lakeynode = *(lakeysc->begin());
	        	    TRACE("Optimising coupled node %d after star in sequence, looking for ", lad)(*lakeynode)("\n");
		        	        
		        	    // Search for the key in the input program sequence    
#if 0 // TODO Very tenuous argument for the Choice pointed to by lakey->governing_choice not having been 
      // deleted, maybe put them under shared_ptr for peace of mind. Argument is: Key points to 
      // Choice. Keys all forgotten at top of MDC(). Conjecture class "live" thoughout both passes
      // in MCD (PFDC() does not wipe the Choices). Choices only wiped after earlier or same decision 
      // runs out of choices. Pointed-to decision earlier than current one(*). Ergo, if current Choice 
      // is live then earlier Choice is live and current too. Like a stack. 
      // (*) is it? Could have been reached late in the keying pass and then seen early in restricting pass
      // - but keying pass was a match so all decisions were hit, and now we only dicard decisions
      // during Increment, which isn't called between keying and restricting passes
      // NOTE: we can have xit_begin_star actually beyond the target lakeynode due to earlier
      // Star<> trynig a big number of nodes. We must reject in this case or we'll create an invalid
      // subsequence. The only way I can see is to enumerate with integers.
	        	    xit = lakey->governing_choice->it;
	        	    ASSERT( lakey );
	        	    ASSERT( lakey->governing_choice );
	        	    ASSERT( lakey->governing_choice->it != lakey->governing_choice->end );
	        	    ASSERT( lakey->governing_choice->it != x.end() );
#else		        	    
	        	    bool found = false;
	        	    for( xit = xit_begin_star; xit != x.end(); ++xit )
	        	    {
	        	        if( (*xit) == lakeynode )
	        	        {
	        	            found = true;
	        	            break;
	        	        }
	        	    }    
	        	    if( !found )
	        	    {
	        	        TRACE("we have already overshot the target node, so fail\n");
	        	        return false;
	        	    }
#endif		        	    
                    // Step back or forward in the input tree nodes, so that we don't include anything that corresponds to the patterns we skipped above
	        	    int offset = lakey->governing_offset - lad; 
                    TRACE("Need to go forward (backward) %d places\n", offset);
	        	    // TODO += on iterators, this is just messy here
	        	    for( ; offset < 0; offset++ )  
	        	    {
	            	    if( xit == x.begin() )
	            	    {
	            	        TRACE("At beginning, cannot back up\n");
			                return false;
			            }
	            	    --xit;
	            	}
	        	    for( ; offset > 0; offset-- ) 
	        	    {
	            	    if( xit == x.end() )
	            	    {
	            	        TRACE("At end, cannot go forward\n");
			                return false;
			            }
	            	    ++xit;
	            	}
	        	    TRACE("Moved to ")(**xit)(", forcing decision\n");
		    	    xit = conj.HandleDecision( xit );
		        }
		        else
#endif
		        {
		    	    xit = conj.HandleDecision( xit_begin_star, x.end(), 1 );
		        }		        		        
            }
            
			// Star matched [xit_begin_star, xit) i.e. xit-xit_begin_star elements
		    // Now make a copy of the elements that matched the star and apply couplings
	    	TreePtr<SubSequence> ss( new SubSequence( xit_begin_star, xit ) );
	    	//for( ContainerInterface::iterator it=xit_begin_star; it != xit; ++it ) // TODO FOREACH?
	    	//{
	    	//	ss->push_back( *it );
	    	//}
	    	
			// Apply couplings to this Star and matched range
            if( TreePtr<Node> keynode = coupling_keys.GetCoupled( pe ) )
                if( Compare( TreePtr<Node>(ss), keynode ) == false )
                    return false;

            // Restrict to pre-restriction or pattern
            bool r = ps->MatchRange( this, *ss );
            if( !r )
                return false;

	        if( can_key )
                coupling_keys.DoKey( TreePtr<Node>(ss), pe, governing_choice, governing_offset );	

	    	if( npit != pattern.end() )
	    	{
		    	governing_choice = conj.GetPrevChoicePtr();
		    	governing_offset = 0;           
		    }
	    }
	    else // not a Star so match singly...
	    {
			// If there is one more element in x, see if it matches the pattern
			//TreePtr<Node> xe( x[xit] );
			if( xit != x.end() && DecidedCompare( *xit, pe, can_key, conj, governing_choice, governing_offset ) == true )
			{
				++xit;
			}
			else
			{
				TRACE("Element mismatched\n");
				return false;
			}
	    }
	    
	    if( governing_choice )
	        governing_offset++; // Gap between the last decision and affected nodes gets bigger as we go along...
	}

    // If we finished the job and pattern and subject are still aligned, then it was a match
	TRACE("Finishing compare sequence %d %d\n", xit==x.end(), pit==pattern.end() );
    return (xit==x.end() && pit==pattern.end()) ? true : false;
}


bool CompareReplace::DecidedCompare( CollectionInterface &x,
			              			   CollectionInterface &pattern,
									   bool can_key,
									   Conjecture &conj ) const
{
    INDENT;
    // Make a copy of the elements in the tree. As we go though the pattern, we'll erase them from
	// here so that (a) we can tell which ones we've done so far and (b) we can get the remainder
	// after decisions.
	// TODO is there some stl algorithm for this?
    TreePtr<SubCollection> xremaining( new SubCollection );
    FOREACH( const TreePtrInterface &xe, x )
        xremaining->insert( xe );
    
    TreePtr<StarBase> star;
    bool seen_star = false;

    for( CollectionInterface::iterator pit = pattern.begin(); pit != pattern.end(); ++pit )
    {
    	TRACE("Collection compare %d remain out of %d; looking at %s in pattern\n",
    			xremaining->size(),
    			pattern.size(),
    			TypeInfo( TreePtr<Node>(*pit) ).name().c_str() );
    	TreePtr<StarBase> maybe_star = dynamic_pointer_cast<StarBase>( TreePtr<Node>(*pit) );

        if( maybe_star ) // Star in pattern collection?
        {
        	ASSERT(!seen_star)("Only one Star node (or NULL ptr) allowed in a search pattern Collection");
        	// TODO remove this restriction - I might want to match one star and leave another unmatched.
            star = maybe_star; // remember for later and skip to next pattern
            seen_star = true; // TODO do we need?
        }
	    else // not a Star so match singly...
	    {
	    	// We have to decide which node in the tree to match, so use the present conjecture
	    	ContainerInterface::iterator xit = conj.HandleDecision( x.begin(), x.end() );
			if( xit == x.end() )
				return false;

	    	// Remove the chosen element from the remaineder collection. If it is not there (ret val==0)
	    	// then the present chosen iterator has been chosen before and the choices are conflicting.
	    	// We'll just return false so we do not stop trying further choices (xit+1 may be legal).
	    	if( xremaining->erase( *xit ) == 0 )
	    		return false;

	    	// Recurse into comparison function for the chosen node
			if( !DecidedCompare( *xit, TreePtr<Node>(*pit), can_key, conj ) )
			    return false;
	    }
    }

    // Now handle the star if there was one; all the non-star matches have been erased from
    // the collection, leaving only the star matches.

    if( !xremaining->empty() && !seen_star )
    	return false; // there were elements left over and no star to match them against

    TRACE("seen_star %d star %p size of xremaining %d\n", seen_star, star.get(), xremaining->size() );

    // Apply pre-restriction to the star
    if( seen_star )
    {
        if( TreePtr<Node> keynode = coupling_keys.GetCoupled( star ) )
            if( Compare( TreePtr<Node>(xremaining), keynode ) == false )
                return false;

        bool r = star->MatchRange( this, *xremaining );
        if( !r )
            return false;
    
        if( can_key )
            coupling_keys.DoKey( TreePtr<Node>(xremaining), star );	
    }
    TRACE("matched\n");
	return true;
}


// Helper for DecidedCompare that does the actual match testing work for the children and recurses.
// Also checks for soft matches.
bool CompareReplace::DecidedCompare( TreePtr<Node> x,
									   TreePtr<SearchContainerBase> pattern,
									   bool can_key,
									   Conjecture &conj ) const
{
    INDENT;
	ASSERT( pattern->terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");

    TRACE("SearchContainer pattern ")(*pattern)(" terminus pattern is ")(*(pattern->terminus))(" at ")(*x)("\n");

    // Get an interface to the container we will search
    shared_ptr<ContainerInterface> pwx = pattern->GetContainerInterface( x );
    
	// Get choice from conjecture about where we are in the walk
	ContainerInterface::iterator thistime = conj.HandleDecision( pwx->begin(), pwx->end() );
	if( thistime == (ContainerInterface::iterator)(pwx->end()) )
		return false; // ran out of choices

	// Try out comparison at this position
	TRACE("Trying terminus ")(**thistime);
	bool r = DecidedCompare( *thistime, pattern->terminus, can_key, conj );
    if( !r )
        return false;
        
    if( TreePtr<Node> keynode = coupling_keys.GetCoupled( pattern ) )
        if( Compare( x, keynode ) == false )
            return false;
    
    // If we got this far, do the couplings
    if( can_key )
    {
    	shared_ptr<SearchContainerKey> key( new SearchContainerKey );
    	key->root = x;
    	key->terminus = *thistime;
    	shared_ptr<Key> sckey( key );
    	TRACE("Matched, so keying search container type ")(*pattern)(" for ")(*x)(" at %p\n", key.get());
        coupling_keys.DoKey( sckey, pattern );	
    }
	return r;
}


bool CompareReplace::MatchingDecidedCompare( TreePtr<Node> x,
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
    Traverse t(pattern);
    FOREACH( TreePtr<Node> n, t )
        if( shared_ptr<SoftSearchPattern> ssp = dynamic_pointer_cast<SoftSearchPattern>(n) )
            ssp->FlushCache();      
}


bool CompareReplace::Compare( TreePtr<Node> x,
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


// Clear all pointer members in supplied dest to NULL
void CompareReplace::ClearPtrs( TreePtr<Node> dest ) const
{
    vector< Itemiser::Element * > dest_memb = dest->Itemise();
    for( int i=0; i<dest_memb.size(); i++ )
    {
        if( TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]) )
        {
            *dest_ptr = TreePtr<Node>();
        }
    }       
}


TreePtr<Node> CompareReplace::DoOverlayOrOverwriteSubstitutionPattern( TreePtr<Node> keynode,
                		                                               shared_ptr<Key> current_key,
		                                                               TreePtr<Node> source ) const
{
    INDENT;
    ASSERT( source ); 
    ASSERT( keynode );
    TRACE("key=")(*keynode)(" source=")(*source)("\n");
    HIT;
    if( ReadArgs::assert_pedigree )
    {
        ASSERT( keyed_pedigree.IsExist(keynode) )(*keynode);
        ASSERT( pattern_pedigree.IsExist(source) )(*source);
    }
    
    TreePtr<Node> dest = ApplySpecialAndCouplingPattern( source );
    if( dest )
        return ApplySlave( source, dest ); // if this produced a result then we're done (effectively, always overwrite)  
    
    if( source->IsLocalMatch(keynode.get()) ) 
    {
        HIT;
        dest = DoOverlaySubstitutionPattern( keynode, current_key, source );
    }
    else
    {
        HIT;    
        dest = DuplicateSubtreePattern( source ); // Overwriting pattern over dest, need to make a duplicate 
    }
    
    dest = ApplySlave( source, dest );
    return dest;
}


TreePtr<Node> CompareReplace::DoOverlaySubstitutionPattern( TreePtr<Node> keynode,
                  	         	                            shared_ptr<Key> current_key,
		                                                    TreePtr<Node> source ) const // under substitution if not NULL
{
	INDENT;
    ASSERT( source );
    ASSERT( keynode );
    HIT;
    if( ReadArgs::assert_pedigree )
    {
        ASSERT( keyed_pedigree.IsExist(keynode) )(*keynode);
        ASSERT( pattern_pedigree.IsExist(source) )(*source);
    }
#ifdef STRACE
    TRACE("DoOverlayPattern dest={");
   {    Expand w(dest);
        bool first=true;
        FOREACH( TreePtr<Node> n, w )
        {
            if( !first )
                TRACE(", ");
            TRACE( n ? *n : string("NULL"));
            first=false;
        }
        TRACE("}\n"); // TODO put this in as a common utility somewhere
   }
   TRACE("source={");
   {    Expand w(source);
        bool first=true;
        FOREACH( TreePtr<Node> n, w )
        {
            if( !first )
                TRACE(", ");
            TRACE( n ? *n : string("NULL"));
            first=false;
        }
        TRACE("}\n"); // TODO put this in as a common utility somewhere
   }
#endif   
    
    ASSERT( source->IsLocalMatch(keynode.get()) )
    ("source=")
    (*source)
    (" must be a non-strict superclass of keynode=")
    (*keynode)
    (", so that it does not have more members");
    TreePtr<Node> dest;
    
    ASSERT( !dynamic_pointer_cast<SearchContainerKey>(current_key)  ); // we only get here when ordinary nodes are coupled

    // Make a new node	    
    dest = DuplicateNode( keynode, current_key );

    // Loop over the elements of source, keynode and dest, limited to elements
    // present in source, which is a non-strict subclass of keynode and dest.
    // Overlay or overwrite source over a duplicate of dest. Keep track of 
    // corresponding elements of dest. 
    vector< Itemiser::Element * > source_memb = source->Itemise();
    vector< Itemiser::Element * > keynode_memb = keynode->Itemise( dest.get() ); // Get the members of keynode corresponding to source's class
    vector< Itemiser::Element * >  dest_memb = source->Itemise( dest.get() ); // Get the members of dest corresponding to source's class
    ASSERT( source_memb.size() == dest_memb.size() );
    Set< Itemiser::Element * > present_in_source;
    
    TRACE("Overlaying %d members source=%s dest=%s\n", dest_memb.size(), TypeInfo(source).name().c_str(), TypeInfo(dest).name().c_str());
    for( int i=0; i<dest_memb.size(); i++ )
    {
    	TRACE("Overlaying member %d\n", i );
        ASSERT( source_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        ASSERT( keynode_memb[i] )( "itemise returned null element" );                
        if( ContainerInterface *source_con = dynamic_cast<ContainerInterface *>(source_memb[i]) )                
        {
            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_memb[i]);
            ASSERT( dest_con )( "itemise for dest didn't match itemise for source");
            dest_con->clear();

            TRACE("Overwriting container size %d\n", source_con->size() );
	        FOREACH( const TreePtrInterface &p, *source_con )
	        {
		        ASSERT( p ); // present simplified scheme disallows NULL
		        TRACE("Got ")(*p)("\n");
		        if( TreePtr<StarBase> ps = dynamic_pointer_cast<StarBase>(TreePtr<Node>(p)) )
		        {
		            shared_ptr<Key> key = coupling_keys.GetKey( ps );
		            ASSERT( key )("Replacing ")(*p)(" but it was not keyed from the search pattern as required\n");
		            TRACE("Got ")(*key->root)("\n");
		            ContainerInterface *psc = dynamic_cast<ContainerInterface *>(key->root.get());
		            ASSERT( psc );
			        TRACE("Expanding SubContainer length %d\n", psc->size() );
		            FOREACH( const TreePtrInterface &pp, *psc )
		            {
		                TreePtr<Node> nn = DuplicateSubtreeSubstitution( pp, key );
                        if( ReadArgs::assert_pedigree )
                            ASSERT( duplicated_pedigree.IsExist(nn) )(*nn);
			            dest_con->insert( nn );
			        }
           		}
		        else
		        {
		            TreePtr<Node> n = DuplicateSubtreePattern( p );
                    if( ReadArgs::assert_pedigree )
                        ASSERT( duplicated_pedigree.IsExist(n) )(*n);
			        TRACE("Normal element, inserting %s directly\n", TypeInfo(n).name().c_str());
			        dest_con->insert( n );
		        }
	        }
	        present_in_source.insert( dest_memb[i] );
        }            
        else if( TreePtrInterface *source_ptr = dynamic_cast<TreePtrInterface *>(source_memb[i]) )
        {
        	TRACE();
            TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            TreePtrInterface *keynode_ptr = dynamic_cast<TreePtrInterface *>(keynode_memb[i]);
            ASSERT( dest_ptr )( "itemise for target didn't match itemise for source");
            TreePtr<Node> source_child = *source_ptr;
            TreePtr<Node> dest_child = *dest_ptr;
                       
            if( source_child )
            {                             
                dest_child = DoOverlayOrOverwriteSubstitutionPattern( *keynode_ptr, current_key, source_child );
                ASSERT( dest_child );
                ASSERT( dest_child->IsFinal() );
                present_in_source.insert( dest_memb[i] );
            }
            
            *dest_ptr = dest_child;
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }        
    }
    
    // Loop over all the elements of keynode and dest that do not appear in source or
    // appear in source but are NULL TreePtr<>s. Duplicate from keynode into dest.
    keynode_memb = keynode->Itemise();
    dest_memb = dest->Itemise(); 
    
    TRACE("Overlaying %d members keynode=%s dest=%s\n", dest_memb.size(), TypeInfo(keynode).name().c_str(), TypeInfo(dest).name().c_str());
    // Loop over all the members of keynode (which can be a subset of dest)
    // and for non-NULL members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<dest_memb.size(); i++ )
    {
    	TRACE("Overlaying member %d\n", i );
        ASSERT( keynode_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        
        if( present_in_source.IsExist(dest_memb[i]) )
            continue; // already did this one in the above loop

        if( ContainerInterface *keynode_con = dynamic_cast<ContainerInterface *>(keynode_memb[i]) )                
        {
            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_memb[i]);
            dest_con->clear();

            TRACE("Copying container size %d\n", keynode_con->size() );
	        FOREACH( const TreePtrInterface &p, *keynode_con )
	        {
		        ASSERT( p ); // present simplified scheme disallows NULL
		        HIT;
		        TreePtr<Node> n = DuplicateSubtreeSubstitution( p, current_key );
		        if( ContainerInterface *sc = dynamic_cast<ContainerInterface *>(n.get()) )
		        {
			        TRACE("Expanding SubContainer length %d\n", sc->size() );
		            FOREACH( const TreePtrInterface &xx, *sc )
			            dest_con->insert( xx );
           		}
		        else
		        {
			        TRACE("Normal element, inserting %s directly\n", TypeInfo(n).name().c_str());
			        dest_con->insert( n );
		        }
	        }
        }            
        else if( TreePtrInterface *keynode_ptr = dynamic_cast<TreePtrInterface *>(keynode_memb[i]) )
        {
            TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( *keynode_ptr )("source should be non-NULL");
            HIT;
            *dest_ptr = DuplicateSubtreeSubstitution( *keynode_ptr, current_key );
            ASSERT( *dest_ptr );
            ASSERT( TreePtr<Node>(*dest_ptr)->IsFinal() );            
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    
#ifdef STRACE
    TRACE("DoOverlayPattern result={");
   {    Expand w(dest);
        bool first=true;
        FOREACH( TreePtr<Node> n, w )
        {
            if( !first )
                TRACE(", ");
            TRACE( n ? *n : string("NULL"));
            first=false;
        }
        TRACE("}\n"); // TODO put this in as a common utility somewhere
   }
#endif    
    ASSERT( dest );
    return dest;
}


TreePtr<Node> CompareReplace::DuplicateNode( TreePtr<Node> source,
    		                                 shared_ptr<Key> current_key ) const
{
    INDENT;
    if( ReadArgs::assert_pedigree )
        ASSERT( keyed_pedigree.IsExist(source) || pattern_pedigree.IsExist(source) )(*source);

    // Make the new node (destination node)
    shared_ptr<Cloner> dup_dest = source->Duplicate(source);
    TreePtr<Node> dest = dynamic_pointer_cast<Node>( dup_dest );
    ASSERT(dest);

    // If not substituting a Stuff/AnyNode node, remember this node is dirty for GreenGrass restriction
    // Also dirty the dest if the source was dirty when we are substituting Stuff
    // TODO try removing StuffKey clause, so never acts during substitution
    // TODO move this into callers so this fn doesn't need a current_key parameter
    if( !current_key || !dynamic_pointer_cast<SearchContainerKey>(current_key) || dirty_grass.find( source ) != dirty_grass.end() )
        dirty_grass.insert( dest );

    TRACE("Duplicated pedigree: ")(*dest)("\n");
    if( ReadArgs::assert_pedigree )
        duplicated_pedigree.insert( dest );    
       
    return dest;    
}    		                                          


TreePtr<Node> CompareReplace::ApplySpecialAndCouplingPattern( TreePtr<Node> source ) const
{
    INDENT;
    ASSERT( source );
    HIT;
    if( ReadArgs::assert_pedigree )
        ASSERT( pattern_pedigree.IsExist(source) )(*source);
    // TODO source must be pattern?    

    // See if the source node is coupled to anything
    shared_ptr<Key> key = coupling_keys.GetKey( source );
    TreePtr<Node> overlay;
        
    if( shared_ptr<SpecialBase> sbs = dynamic_pointer_cast<SpecialBase>(source) )
    {   
        if( shared_ptr<SoftReplacePattern> srp = dynamic_pointer_cast<SoftReplacePattern>( source ) )
        {
            ASSERT(key)(*source)(" not keyed\n");
            overlay = srp->GetOverlayPattern(); // only strong modifiers use this
        }
        else if( shared_ptr<OverlayBase> ob = dynamic_pointer_cast<OverlayBase>( source ) )
        {
            ASSERT(key)(*source)(" not keyed\n");
            TRACE("Overlay node through=")(*(ob->GetThrough()))(" overlay=")(*(ob->GetOverlay()))("\n");
            ASSERT( ob->GetOverlay() );          
            overlay = ob->GetOverlay(); 
        }
        
        if( TreePtr<SlaveBase> sb = dynamic_pointer_cast<SlaveBase>(source) )
        {   
            ASSERT(!key)("slave should not be coupled; should be in replace pattern only\n");
            HIT;
            return DuplicateSubtreePattern( sb->GetThrough() ); 
        } 
        else if( overlay )
        {
            return DoOverlayOrOverwriteSubstitutionPattern(key->root, key, overlay);
        }
        else
        {
            // SearchContainer, Star, Not, TransformOf etc. Also MatchAll with no overlay pattern falls thru to here
            ASSERT(key)(*source)(" not keyed\n");     
            // this call is justified: we really do want to go all the way to leaves for eg Star, NotMatch because
            // we have nothing to overlay (Star pattern is only restriction, NotMatch pattern is abnormal, 
            // TransformOf pattern is not type-correct to overlay etc).
            // DuplicateSubtreeSubstitution() will spot the terminus of SearchContainers (Stuff, AnyNode)
            HIT;
            return DuplicateSubtreeSubstitution(key->root, key);   
        }     
    }
    else
    {
        if( key )
        {
            // If we're here we keyed a coupling on a normal node.
            return DoOverlaySubstitutionPattern( key->root, key, source );
        }
        else
        {
            return TreePtr<Node>();
        }
    }
}


TreePtr<Node> CompareReplace::ApplySlave( TreePtr<Node> source, TreePtr<Node> dest ) const 
{
    INDENT;
    if( ReadArgs::assert_pedigree )
    {
        ASSERT( pattern_pedigree.IsExist(source) )(*source);
        ASSERT( duplicated_pedigree.IsExist(dest) )(*dest);
    }

    if( TreePtr<SlaveBase> sb = dynamic_pointer_cast<SlaveBase>(source) )
    {
        TreePtr<Node> input_tree_node = coupling_keys.GetCoupled(source);
        (*sb)( *pcontext, &dest );

        if( ReadArgs::assert_pedigree )
        {
            // The slave's output will be seen by the master when for example
            // the master overlays something over it.
            TRACE("Duplicated pedigree (from slave): ");
            Traverse t1(dest);
            FOREACH( TreePtr<Node> n, t1 )
            {
                duplicated_pedigree.insert( n );    
                TRACE(*n)(" ");
            }
            TRACE("\n");
        }
    } 
    return dest;
}


TreePtr<Node> CompareReplace::DuplicateSubtreePattern( TreePtr<Node> source ) const
{
	INDENT;

    ASSERT( source );
 	TRACE("Duplicating ")(*source)("\n");
    HIT;
    if( ReadArgs::assert_pedigree )
        ASSERT( pattern_pedigree.IsExist(source) )(*source);
    TreePtr<Node> dest = ApplySpecialAndCouplingPattern( source );
    if( dest )
        return ApplySlave( source, dest ); // if this produced a result then we're done   
    
    // Make a new node
    dest = DuplicateNode( source );

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of source, and itemises over that type. dest must
    // be dynamic_castable to source's type.
    vector< Itemiser::Element * > source_memb = source->Itemise();
    vector< Itemiser::Element * > dest_memb = dest->Itemise(); 

    TRACE("Copying %d members source=", dest_memb.size())(*source)(" dest=")(*dest)("\n");
    // Loop over all the members of source (which can be a subset of dest)
    // and for non-NULL members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<dest_memb.size(); i++ )
    {
    	TRACE("Copying member %d\n", i );
        ASSERT( source_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        
        if( ContainerInterface *source_con = dynamic_cast<ContainerInterface *>(source_memb[i]) )                
        {
            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_memb[i]);
            dest_con->clear();

            TRACE("Copying container size %d\n", source_con->size() );
	        FOREACH( const TreePtrInterface &p, *source_con )
	        {
		        ASSERT( p )("Some element of member %d (", i)(*source_con)(") of ")(*source)(" was NULL\n");
		        TRACE("Got ")(*p)("\n");
		        if( TreePtr<StarBase> ps = dynamic_pointer_cast<StarBase>(TreePtr<Node>(p)) )
		        {
		            shared_ptr<Key> key = coupling_keys.GetKey( ps );
		            ASSERT( key )("Replacing ")(*p)(" but it was not keyed from the search pattern as required\n");
		            TRACE("Got ")(*key->root)("\n");
		            ContainerInterface *psc = dynamic_cast<ContainerInterface *>(key->root.get());
		            ASSERT( psc );
			        TRACE("Expanding SubContainer length %d\n", psc->size() );
		            FOREACH( const TreePtrInterface &pp, *psc )
		            {
		                TreePtr<Node> nn = DuplicateSubtreeSubstitution( pp, key );
                        if( ReadArgs::assert_pedigree )
                            ASSERT( duplicated_pedigree.IsExist(nn) )(*nn);
			            dest_con->insert( nn );
			        }
           		}
		        else
		        {
		            TreePtr<Node> n = DuplicateSubtreePattern( p );
                    if( ReadArgs::assert_pedigree )
                        ASSERT( duplicated_pedigree.IsExist(n) )(*n);
			        TRACE("Normal element, inserting %s directly\n", TypeInfo(n).name().c_str());
			        dest_con->insert( n );
		        }
	        }
        }            
        else if( TreePtrInterface *source_ptr = dynamic_cast<TreePtrInterface *>(source_memb[i]) )
        {
            TRACE("Copying single element\n");
            TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( *source_ptr )("Member %d (", i)(*source_ptr)(") of ")(*source)(" was NULL when not overlaying\n");
            *dest_ptr = DuplicateSubtreePattern( *source_ptr );
            ASSERT( *dest_ptr );
            ASSERT( TreePtr<Node>(*dest_ptr)->IsFinal() );            
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    
    dest = ApplySlave( source, dest );
    
    return dest;
}


TreePtr<Node> CompareReplace::DuplicateSubtreeSubstitution( TreePtr<Node> keynode,
		                                                    shared_ptr<Key> current_key ) const
{
	INDENT;
	ASSERT( current_key );
	ASSERT(keynode);
    HIT;
    if( ReadArgs::assert_pedigree )
        ASSERT( keyed_pedigree.IsExist(keynode) )(*keynode);
    
    TRACE("Duplicating %s current_key=%p\n", ((string)*keynode).c_str(), current_key.get());
    // Under substitution, we should be duplicating a subtree of the input
    // program, which should not contain any special nodes
    ASSERT( !(dynamic_pointer_cast<SpecialBase>(keynode)) )
          ("Duplicating special node ")
          (*keynode)
          (" while under substitution\n" );

    // Are we substituting a stuff node? If so, see if we reached the terminus, and if
	// so come out of substitution. Done as tail recursion so that we already duplicated
	// the terminus key, and can just overlay the terminus replace pattern.
	if( shared_ptr<SearchContainerKey> stuff_key = dynamic_pointer_cast<SearchContainerKey>(current_key) )
	{
		TRACE( "Substituting stuff: keynode=%s:%p, term=%s:%p\n",
				TypeInfo(keynode).name().c_str(), keynode.get(),
				TypeInfo(stuff_key->terminus).name().c_str(), stuff_key->terminus.get() );
		ASSERT( stuff_key->replace_pattern );
		TreePtr<SearchContainerBase> replace_stuff = dynamic_pointer_cast<SearchContainerBase>( stuff_key->replace_pattern );
		ASSERT( replace_stuff );
		if( replace_stuff->terminus &&      // There is a terminus in replace pattern
			keynode == stuff_key->terminus ) // and the present node is the terminus in the keynode pattern
		{
		    // go to the coupling of the terminus so we don't keep exiting stuff substitution over and over
		    shared_ptr<Key> terminus_key = coupling_keys.GetKey( replace_stuff->terminus );
			TRACE( "Leaving substitution to duplicate terminus replace pattern at ")(*(replace_stuff->terminus))("\n" );
			TreePtr<Node> term = DoOverlayOrOverwriteSubstitutionPattern( keynode, terminus_key, replace_stuff->terminus );
			//DuplicateSubtree( replace_stuff->terminus ); // not in substitution any more
			TRACE( "Returning to substitution for rest of stuff\n" );
			return term;
		}
	}

    // Make a new node	    
    TreePtr<Node> dest = DuplicateNode( keynode, current_key );

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of keynode, and itemises over that type. dest must
    // be dynamic_castable to keynode's type.
    vector< Itemiser::Element * > keynode_memb = keynode->Itemise();
    vector< Itemiser::Element * > dest_memb = dest->Itemise(); 

    TRACE("Overlaying %d members keynode=%s dest=%s\n", dest_memb.size(), TypeInfo(keynode).name().c_str(), TypeInfo(dest).name().c_str());
    // Loop over all the members of keynode (which can be a subset of dest)
    // and for non-NULL members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<dest_memb.size(); i++ )
    {
    	TRACE("Overlaying member %d\n", i );
        ASSERT( keynode_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        
        if( ContainerInterface *keynode_con = dynamic_cast<ContainerInterface *>(keynode_memb[i]) )                
        {
            ContainerInterface *dest_con = dynamic_cast<ContainerInterface *>(dest_memb[i]);
            dest_con->clear();

            TRACE("Copying container size %d\n", keynode_con->size() );
	        FOREACH( const TreePtrInterface &p, *keynode_con )
	        {
		        ASSERT( p ); // present simplified scheme disallows NULL
		        HIT;
		        TreePtr<Node> n = DuplicateSubtreeSubstitution( p, current_key );
		        if( ContainerInterface *sc = dynamic_cast<ContainerInterface *>(n.get()) )
		        {
			        TRACE("Expanding SubContainer length %d\n", sc->size() );
		            FOREACH( const TreePtrInterface &xx, *sc )
			            dest_con->insert( xx );
			            ASSERT(0);
           		}
		        else
		        {
			        TRACE("Normal element, inserting %s directly\n", TypeInfo(n).name().c_str());
			        dest_con->insert( n );
		        }
	        }
        }            
        else if( TreePtrInterface *keynode_ptr = dynamic_cast<TreePtrInterface *>(keynode_memb[i]) )
        {
            TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( *keynode_ptr )("keynode should be non-NULL");
            HIT;
            *dest_ptr = DuplicateSubtreeSubstitution( *keynode_ptr, current_key );
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
void CompareReplace::KeyReplaceNodes( TreePtr<Node> source ) const
{
    INDENT;
    TRACE("Walking replace pattern to key the soft nodes\n");
    
    TraverseNoSlavePattern e(source);
    FOREACH( TreePtr<Node> pattern, e )
	{
	    TRACE(*pattern)("\n");
	    TreePtr<Node> key = pattern;
	    if( shared_ptr<SoftReplacePattern> srp = dynamic_pointer_cast<SoftReplacePattern>( pattern ) )
	    {
            TRACE("Soft replace pattern not keyed, ")(*pattern)("\n");

            // Call the soft pattern impl 
            key = srp->DuplicateSubtree( this );
            if( key )
            {            
                if( ReadArgs::assert_pedigree )
                {
                    TRACE("Keyed pedigree: ");
                    Traverse t1(key);
                    FOREACH( TreePtr<Node> n, t1 )
                    {
                        keyed_pedigree.insert( n );    
                        TRACE(*n)(" ");
                    }
                    TRACE("\n");
                }

                // Allow this to key a coupling. 
                coupling_keys.DoKey( key, pattern );
            } 
	    }
    }
}

/*
void RunSlaves( TreePtr<Node> pattern )
{
    Flatten f( pattern );
    FOREACH( TreePtr<Node> n, f )
    {
        if( shared_ptr<SlaveBase> sb = dynamic_pointer_cast<SlaveBase>( n ) )
        {
            RunSlaves( sb->GetThrough() ); // run more-nested slaves first
            pdest = // the address of the TreePtr to the node in the generated output tree corresponding to the slave's position
            (*sb)( *pcontext, pdest );
        }
        else
        {
            RunSlaves( n );
        }
    }
}
*/

TreePtr<Node> CompareReplace::MatchingDuplicateSubtree( TreePtr<Node> source ) const
{
    INDENT("R");
    if( ReadArgs::assert_pedigree )
    {
        // Populate backing sets for checking node usage
        keyed_pedigree = coupling_keys.GetAllKeys();
    }
    
    // Do a two-pass process: first get the keys...
    TRACE("doing replace KEYING pass....\n");
    //(void)DuplicateSubtree( source, true );
    KeyReplaceNodes( source );
    TRACE("replace KEYING pass\n" );

    // Now replace according to the couplings
    TRACE("doing replace SUBSTITUTING pass....\n");
    TreePtr<Node> r = DuplicateSubtreePattern( source );
    // TODO do an overlay, means *proot needs passing in here and this fn should be renamed.
    // Also make sure tree at *proot is in the "keyed" pedigree otherwise asserts will fail.
    TRACE("replace SUBSTITUTING pass\n" );
    return r;
}


bool CompareReplace::SingleCompareReplace( TreePtr<Node> *proot ) 
{
    INDENT;

    if( ReadArgs::assert_pedigree )
    {
        TRACE("\n")("Replace pattern pedigree: "); 
        pattern_pedigree.clear();                    
        Traverse t2(replace_pattern);
        FOREACH( TreePtr<Node> n, t2 )
        {
            pattern_pedigree.insert( n );    
            TRACE(*n)(" ");
        }
        TRACE("\n");
    }

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
        *proot = MatchingDuplicateSubtree( replace_pattern );
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
    if( ReadArgs::assert_pedigree )
    {
	    dirty_grass.clear(); // TODO huh?
        duplicated_pedigree.clear();
    }
    
    TRACE("begin RCR %p\n", this);
        
    bool r=false;
    int i=0;
    for(i=0; i<repetitions; i++) 
    {
    	r = SingleCompareReplace( proot );
    	TRACE("%p SCR result %d\n", this, r);        
    	if( !r )
            break; // when the compare fails, we're done
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
         
    dirty_grass.clear();     
         
    TRACE("%p exiting\n", this);
    return i;
}


// Do a search and replace based on patterns stored in our members
void CompareReplace::operator()( TreePtr<Node> c, TreePtr<Node> *proot )
{
    INDENT("");
    TRACE("Enter S&R instance at %p\n", this);
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
    CompareReplace( NULL, NULL, im )                              
{
    if( sp )
        Configure( sp, rp );
}


void SearchReplace::Configure( TreePtr<Node> sp,
                               TreePtr<Node> rp )
{                    
    ASSERT( sp ); // a search pattern is required to configure the engine

    // Make a non-rooted search and replace (ie where the base of the search pattern
    // does not have to be the root of the whole program tree).
    // Insert a Stuff node as root of the search pattern
    // Needs to be Node, because we don't want pre-restriction action here (if we're a slave
    // we got pre-restricted already.
    TreePtr< Stuff<Node> > stuff( new Stuff<Node> );

    if( !rp )
    {
        // Search and replace immediately coupled, insert Stuff, but don't bother
        // with the redundant Overlay.
        stuff->terminus = sp;
        CompareReplace::Configure( stuff );
    }
    else
    {
        // Classic search and replace with seperate replace pattern, we can use
        // an Overlay node to overwrite the replace pattern at replace time.
        
        // Insert a Stuff node as root of the replace pattern
        TreePtr< Overlay<Node> > overlay( new Overlay<Node> );
        stuff->terminus = overlay;
        overlay->through = sp;
        overlay->overlay = rp;

        // Configure the rooted implementation (CompareReplace) with new patterns and couplings
        CompareReplace::Configure( stuff, stuff );
    }
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
        return shared_ptr<ContainerInterface>( new Flatten( x ) );
    else
        return shared_ptr<ContainerInterface>( new Expand( x, NULL, rf ) );
}


bool StarBase::MatchRange( const CompareReplace *sr,
                           ContainerInterface &range )
{
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
    TRACE("Flattening an AnyNode at ")(*x)(": { ");
    Flatten f( x );
    FOREACH( TreePtr<Node> pn, f )
        {TRACE(*pn)(" ");}
    TRACE("}\n");
        
    return shared_ptr<ContainerInterface>( new Flatten( x ) );
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
    TRACE("DuplicateSubtree source={");
	    Expand w(source);
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
