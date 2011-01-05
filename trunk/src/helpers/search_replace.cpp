#include "search_replace.hpp"
#include "validate.hpp"
#include "render/graph.hpp" //  for graphing patterns
#include "conjecture.hpp"

// Master constructor remembers search pattern, replace pattern and any supplied couplings as required
RootedSearchReplace::RootedSearchReplace( TreePtr<Node> sp,
                                          TreePtr<Node> rp,
                                          CouplingSet m ) :                 
    search_pattern( sp ),
    replace_pattern( rp ),
    couplings( m )
{
    Validate v(true);
    v(search_pattern, &search_pattern);
    v(replace_pattern, &replace_pattern);

	// Count the number of times we hit each node during a walk.
	// Where we hit a node more than once, add a coupling for it.
	// This will detect nodes with multiple refs (which we want 
	// couplings for) and all nodes thereunder (which I'm not sure
	// if we want couplgs for or even if should be allowed).
	// TODO decide.
	TRACE("doing inferred couplings\n");
	Map< TreePtr<Node>, int > ms;
    Sweep wsp( sp ), wrp( rp );
    Sweep::iterator i;
    for( i=(wsp.begin()); i != wsp.end(); ++i )
    {
        TreePtr<Node> n = *i;
        if( n )
        {
            if( dynamic_pointer_cast<StuffBase>(n) )
                ms[n] = 2; // foce couplings at overlay nodes TODO refactor substitution so this is not needed
            else if( ms.IsExist( n ) )
                ms[n]++;
            else
                ms[n] = 1;
        }
    }
    for( i.ContinueAt(wrp.begin()); i != wrp.end(); ++i )
    {
        TreePtr<Node> n = *i;
        if( n )
        {
            if( ms.IsExist( n ) )
                ms[n]++;
            else
                ms[n] = 1;
        }
    }
    typedef	pair<TreePtr<Node>, int> pair;	
	FOREACH( pair pp, ms )
	{
	    if( pp.second > 1 )
			if( CouplingKeys::FindCoupling( pp.first, couplings ).empty() )
            {
                TRACE("Inserting coupling for ")(*(pp.first))(" count is %d\n", pp.second );
                couplings.insert( Coupling( pp.first ) );	
            }
	}
	
    // Look for slaves. If we find them, copy our couplings into their couplings
    // Do not just overwrite since there may be implicit Stuff node couplings.
    // TODO is there a way of unioning sets without doing lots of insert()?
    FOREACH( TreePtr<Node> n, wrp )
    {
        if( TreePtr<RootedSlaveBase> rsb = dynamic_pointer_cast<RootedSlaveBase>(n) )
		    rsb->couplings = couplings;
        else if( TreePtr<SlaveBase> rsb = dynamic_pointer_cast<SlaveBase>(n) )
			rsb->couplings = couplings;
    }
} 


// Slave constructor used by slave wrapper
RootedSearchReplace::RootedSearchReplace( TreePtr<Node> sp,
                                          TreePtr<Node> rp,
                                          int i ) :                                  
    search_pattern( sp ),
    replace_pattern( rp )
{
	(void)i;
}


// Destructor tries not to leak memory lol
RootedSearchReplace::~RootedSearchReplace()
{
}


// Helper for DecidedCompare that does the actual match testing work for the children and recurses.
// Also checks for soft matches.
Result RootedSearchReplace::DecidedCompare( TreePtr<Node> x,
											TreePtr<Node> pattern,
											CouplingKeys *keys,
											bool can_key,
    										Conjecture &conj ) const
{
    INDENT;
	ASSERT( x ); // Target must not be NULL
	if( !pattern )    // NULL matches anything in search patterns (just to save typing)
		return FOUND;
    TRACE("Comparing x=")
         (*x)
         (" with pattern=")
         (*pattern)
         ("\n");
    
	// Check whether the present node matches. Do this for all nodes: this will be the local
	// restriction for normal nodes and the pre-restriction for special nodes (based on
	// how IsLocalMatch() has been overridden.
	if( !pattern->IsLocalMatch(x.get()) )
		return NOT_FOUND;

	if( shared_ptr<SoftSearchPattern> ssp = dynamic_pointer_cast<SoftSearchPattern>(pattern) )
    {
    	// Hand over to any soft search functionality in the search pattern node
    	Result r = ssp->DecidedCompare( this, x, keys, can_key, conj );
    	if( r != FOUND )
    		return NOT_FOUND;
    }
    else if( TreePtr<StuffBase> stuff_pattern = dynamic_pointer_cast<StuffBase>(pattern) )
    {
    	// Invoke stuff node compare
    	// Check whether the present node matches
    	bool r = DecidedCompare( x, stuff_pattern, keys, can_key, conj );
        if( r != FOUND )
            return NOT_FOUND;
    }
    else if( TreePtr<GreenGrassBase> green_pattern = dynamic_pointer_cast<GreenGrassBase>(pattern) )
    {
        // Restrict so that everything in the input program under here must be "green grass"
        // ie unmodified by previous replaces in this RepeatingSearchReplace() run.
        Walk w( x );
        FOREACH( TreePtr<Node> p, w )
            if( dirty_grass.find( p ) != dirty_grass.end() )
                return NOT_FOUND;
        // Normal matching for the through path
        bool r = DecidedCompare( x, green_pattern->GetThrough(), keys, can_key, conj );
        if( r != FOUND )
            return NOT_FOUND;
    }
    else if( TreePtr<OverlayBase> op = dynamic_pointer_cast<OverlayBase>(pattern) )
    {
        // When Overlay node seen duriung search, just forward through the "base" path
        bool r = DecidedCompare( x, op->base, keys, can_key, conj );
        if( r != FOUND )
            return NOT_FOUND;
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
			Result r;
			ASSERT( pattern_memb[i] )( "itemise returned null element");
			ASSERT( x_memb[i] )( "itemise returned null element");

			if( SequenceInterface *pattern_seq = dynamic_cast<SequenceInterface *>(pattern_memb[i]) )
			{
				SequenceInterface *x_seq = dynamic_cast<SequenceInterface *>(x_memb[i]);
				ASSERT( x_seq )( "itemise for x didn't match itemise for pattern");
				TRACE("Member %d is Sequence, x %d elts, pattern %d elts\n", i, x_seq->size(), pattern_seq->size() );
				r = DecidedCompare( *x_seq, *pattern_seq, keys, can_key, conj );
			}
			else if( CollectionInterface *pattern_col = dynamic_cast<CollectionInterface *>(pattern_memb[i]) )
			{
				CollectionInterface *x_col = dynamic_cast<CollectionInterface *>(x_memb[i]);
				ASSERT( x_col )( "itemise for x didn't match itemise for pattern");
				TRACE("Member %d is Collection, x %d elts, pattern %d elts\n", i, x_col->size(), pattern_col->size() );
				r = DecidedCompare( *x_col, *pattern_col, keys, can_key, conj );
			}
			else if( TreePtrInterface *pattern_ptr = dynamic_cast<TreePtrInterface *>(pattern_memb[i]) )
			{
				TreePtrInterface *x_ptr = dynamic_cast<TreePtrInterface *>(x_memb[i]);
				ASSERT( x_ptr )( "itemise for x didn't match itemise for pattern");
				TRACE("Member %d is TreePtr, pattern ptr=%p\n", i, pattern_ptr->get());
				r = DecidedCompare( *x_ptr, TreePtr<Node>(*pattern_ptr), keys, can_key, conj );
			}
			else
			{
				ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a TreePtr");
			}

			if( r != FOUND )
				return NOT_FOUND;
		}
    }
   
    // If we got here, the node matched the search pattern. Now apply couplings
    if( keys )
        return keys->KeyAndRestrict( x, pattern, this, can_key );

    return FOUND;
}


// xstart and pstart are the indexes into the sequence where we will begin checking for a match.
// It is assumed that elements before these have already been matched and may be ignored.
Result RootedSearchReplace::DecidedCompare( SequenceInterface &x,
		                                    SequenceInterface &pattern,
		                                    CouplingKeys *keys,
		                                    bool can_key,
		                                    Conjecture &conj ) const
{
    INDENT;
	// Attempt to match all the elements between start and the end of the sequence; stop
	// if either pattern or subject runs out.
	ContainerInterface::iterator xit = x.begin();
	ContainerInterface::iterator pit = pattern.begin();

	while( pit != pattern.end() )
	{
		ASSERT( xit == x.end() || *xit );

		// Get the next element of the pattern
		TreePtr<Node> pe( *pit );
		++pit;
	    if( !pe || dynamic_pointer_cast<StarBase>(pe) )
	    {
	    	TRACE("Star (pe is %d)\n", (int)!!pe);
			// We have a Star type wildcard that can match multiple elements. At present,
			// NULL is interpreted as a Star (but cannot go in a coupling).

	    	// Remember where we are - this is the beginning of the subsequence that
	    	// potentially matches the Star.
	    	ContainerInterface::iterator xit_begin_star = xit;

	    	// Star always matches at the end of a sequence, so we only bother checking when there
	    	// are more elements left
	    	if( pit == pattern.end() )
	    	{
	    		xit = x.end(); // match all remaining members of x; jump to end
	    	}
	    	else
	    	{
	    		TRACE("Pattern continues after star\n");

	    		// Star not at end so there is more stuff to match; ensure not another star
	    		ASSERT( !dynamic_pointer_cast<StarBase>(TreePtr<Node>(*pit)) )
	    		      ( "Not allowed to have two neighbouring Star elements in search pattern Sequence");

		    	// Decide how many elements the current * should match, using conjecture. Jump forward
	    		// that many elements, to the element after the star
		    	xit = conj.HandleDecision( xit_begin_star, x.end() );

		    	// If we got to the end of the subject Sequence, there's no way to match the >0 elements
				// we know are still in the pattern.
				if( xit == x.end() )
				{
					TRACE("Ran out of candidate\n");
					return NOT_FOUND;
				}
            }

			// Star matched [xit_begin_star, xit) i.e. xit-xit_begin_star elements
		    // Now make a copy of the elements that matched the star and apply couplings
		    if( pe )
		    {
		    	TreePtr<SubSequence> ss( new SubSequence);
		    	for( ContainerInterface::iterator it=xit_begin_star; it != xit; ++it ) // TODO FOREACH?
		    	{
		    		// Apply prerestriction
		    		if( !pe->IsLocalMatch( it->get()) )
		    			return NOT_FOUND;
		    		ss->push_back( *it );
		    	}
				// Apply couplings to this Star and matched range
				if( keys )
		    		if( !keys->KeyAndRestrict( TreePtr<Node>(ss), pe, this, can_key ) )
		        	    return NOT_FOUND;
		    }
	    }
	    else // not a Star so match singly...
	    {
	    	TRACE("Not a star, x=")
	    	     (**xit)
	    	     (" pattern=")
	    	     (*pe)
	    	     ("\n");
			// If there is one more element in x, see if it matches the pattern
			//TreePtr<Node> xe( x[xit] );
			if( xit != x.end() && DecidedCompare( *xit, pe, keys, can_key, conj ) == FOUND )
			{
				++xit;
			}
			else
			{
				TRACE("Element mismatched\n");
				return NOT_FOUND;
			}
	    }
	}

    // If we finished the job and pattern and subject are still aligned, then it was a match
	TRACE("Finishing compare sequence %d %d\n", xit==x.end(), pit==pattern.end() );
    return (xit==x.end() && pit==pattern.end()) ? FOUND : NOT_FOUND;
}


Result RootedSearchReplace::DecidedCompare( CollectionInterface &x,
											CollectionInterface &pattern,
											CouplingKeys *keys,
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

        if( maybe_star || !(*pit) ) // Star or NULL in pattern collection?
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
				return NOT_FOUND;

	    	// Remove the chosen element from the remaineder collection. If it is not there (ret val==0)
	    	// then the present chosen iterator has been chosen before and the choices are conflicting.
	    	// We'll just return NOT_FOUND so we do not stop trying further choices (xit+1 may be legal).
	    	if( xremaining->erase( *xit ) == 0 )
	    		return NOT_FOUND;

	    	// Recurse into comparison function for the chosen node
			if( !DecidedCompare( *xit, TreePtr<Node>(*pit), keys, can_key, conj ) )
			    return NOT_FOUND;
	    }
    }

    // Now handle the star if there was one; all the non-star matches have been erased from
    // the collection, leaving only the star matches.

    if( !xremaining->empty() && !seen_star )
    	return NOT_FOUND; // there were elements left over and no star to match them against

    // Apply pre-restriction to the star
    if( seen_star && star )
        FOREACH( const TreePtrInterface &xe, *xremaining )
            if( !star->IsLocalMatch( xe.get()) )
		    	return NOT_FOUND;

    // If we got here, the node matched the search pattern. Now apply couplings
    TRACE("seen_star %d star %p size of xremaining %d\n", seen_star, star.get(), xremaining->size() );
    if( keys && seen_star && star )
        if( !keys->KeyAndRestrict( TreePtr<Node>(xremaining), star, this, can_key ) )
        	return NOT_FOUND;
    TRACE("matched\n");
	return FOUND;
}


// Helper for DecidedCompare that does the actual match testing work for the children and recurses.
// Also checks for soft matches.
Result RootedSearchReplace::DecidedCompare( TreePtr<Node> x,
										    TreePtr<StuffBase> stuff_pattern,
										    CouplingKeys *keys,
										    bool can_key,
										    Conjecture &conj ) const
{
    INDENT;
	ASSERT( stuff_pattern->terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");

	// Define a walk, rooted at this node, restricted as specified in search pattern
	Walk wx( x, stuff_pattern->recurse_restriction, this, keys );

	// Get decision from conjecture about where we are in the walk
	ContainerInterface::iterator thistime = conj.HandleDecision( wx.begin(), wx.end() );
	if( thistime == (ContainerInterface::iterator)(wx.end()) )
		return NOT_FOUND; // ran out of choices

	// Try out comparison at this position
	Result r = DecidedCompare( *thistime, stuff_pattern->terminus, keys, can_key, conj );

    // If we got this far, do the couplings
    if( keys && r )
    {
    	shared_ptr<StuffKey> key( new StuffKey );
    	key->root = x;
    	key->terminus = *thistime;
    	//TreePtr<Node> = TreePtrInterface
        r = keys->KeyAndRestrict( shared_ptr<Key>(key),
        		                  stuff_pattern,
        		                  this,
        		                  can_key );
    }
	return r;
}


Result RootedSearchReplace::MatchingDecidedCompare( TreePtr<Node> x,
		                                                     TreePtr<Node> pattern,
		                                                     CouplingKeys *keys,
		                                                     bool can_key,
		                                                     Conjecture &conj ) const
{
    INDENT;
    Result r;
    if( keys )
    {
        CouplingKeys original_match_keys = *keys; // deep copy here

        // Only key if the keys are already set to KEYING (which is 
        // the initial value). Keys could be RESTRICTING if we're under
        // a SoftNot node, in which case we only want to restrict.
        if( can_key )
        {
            // Do a two-pass matching process: first get the keys...
           	TRACE("doing KEYING pass....\n");
        	conj.PrepareForDecidedCompare();
            r = DecidedCompare( x, pattern, keys, true, conj );
            TRACE("KEYING pass result %d\n", r );
    	    if( r != FOUND )
    	    {
    	    	*keys = original_match_keys; // revert match keys since we failed
    	    	return NOT_FOUND;                  // Save time by giving up if no match found
    	    }
        }
        
	    // Now restrict the search according to the couplings
    	TRACE("doing RESTRICTING pass....\n");
    	conj.PrepareForDecidedCompare();
        r = DecidedCompare( x, pattern, keys, false, conj );
        TRACE("RESTRICTING pass result %d\n", r );
	    if( r != FOUND )
	    {
	    	*keys = original_match_keys; // revert match keys since we failed
	    	return NOT_FOUND;	               // Save time by giving up if no match found
	    }

	    // Do not revert match keys if we were successful - keep them for replace
	    // and any slave search and replace operations we might do.
	    return FOUND;
    }
    else
    {
    	// No coupling, so just call straight through this layer
    	conj.PrepareForDecidedCompare();
    	return DecidedCompare( x, pattern, NULL, false, conj );
    }
}


Result RootedSearchReplace::Compare( TreePtr<Node> x,
		                             TreePtr<Node> pattern,
									 CouplingKeys *keys,
								  	 bool can_key ) const
{
    INDENT;
	TRACE("Comparing x=%s\n", typeid(*x).name() );
	// Create the conjecture object we will use for this compare, and then go
	// into the recursive compare function
	Conjecture conj;
	return conj.Search( x, pattern, keys, can_key, this );
}


// Clear all pointer members in supplied dest to NULL
void RootedSearchReplace::ClearPtrs( TreePtr<Node> dest ) const
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


// Helper for DuplicateSubtree, fills in children of dest node from source node when source node child
// is non-NULL. This means we can call this multiple times with different sources and get a priority 
// scheme.
void RootedSearchReplace::Overlay( TreePtr<Node> dest,
		                           TreePtr<Node> source,
		                           CouplingKeys *keys,
		                           bool can_key,
		                           shared_ptr<Key> current_key ) const
{
	INDENT;
    ASSERT( source );
    ASSERT( dest );
    ASSERT( source->IsLocalMatch(dest.get()) )
    ("source=")
    (*source)
    (" must be a non-strict superclass of destination=")
    (*dest)
    (", so that it does not have more members");

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of source, and itemises over that type. dest must
    // be dynamic_castable to source's type.
    vector< Itemiser::Element * > source_memb = source->Itemise();
    vector< Itemiser::Element * > dest_memb = source->Itemise( dest.get() ); // Get the members of dest corresponding to source's class
    ASSERT( source_memb.size() == dest_memb.size() );

    TRACE("Overlaying %d members source=%s dest=%s\n", dest_memb.size(), TypeInfo(source).name().c_str(), TypeInfo(dest).name().c_str());
   TRACE("Overlay dest={");
   {    Walk w(dest);
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
   {    Walk w(source);
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
    // Loop over all the members of source (which can be a subset of dest)
    // and for non-NULL members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<dest_memb.size(); i++ )
    {
    	TRACE("Overlaying member %d\n", i );
        ASSERT( source_memb[i] )( "itemise returned null element" );
        ASSERT( dest_memb[i] )( "itemise returned null element" );
        
        if( SequenceInterface *source_seq = dynamic_cast<SequenceInterface *>(source_memb[i]) )                
        {
            SequenceInterface *dest_seq = dynamic_cast<SequenceInterface *>(dest_memb[i]);
            ASSERT( dest_seq )( "itemise for dest didn't match itemise for source");
            Overlay( dest_seq, source_seq, keys, can_key, current_key );
        }            
        else if( CollectionInterface *source_col = dynamic_cast<CollectionInterface *>(source_memb[i]) )
        {
        	CollectionInterface *dest_col = dynamic_cast<CollectionInterface *>(dest_memb[i]);
            ASSERT( dest_col )( "itemise for dest didn't match itemise for source");
            Overlay( dest_col, source_col, keys, can_key, current_key );
        }
        else if( TreePtrInterface *source_ptr = dynamic_cast<TreePtrInterface *>(source_memb[i]) )
        {
        	TRACE();
            TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( dest_ptr )( "itemise for target didn't match itemise for source");
            TreePtr<Node> source_child = *source_ptr;
            TreePtr<Node> dest_child = *dest_ptr;

            if( current_key )
            {
                ASSERT( !dest_child )("substitution should just be a copy, so dest should be NULL");
                ASSERT( source_child )("substitution should just be a copy, so source should be non-NULL");
            }
            ASSERT( source_child || dest_child )
                  ("When overlaying ")
                  (*source)
                  (" over ")
                  (*dest)
                  (", found a member that is NULL in both (one must be non-NULL)\n");
            
            // This avoids linking into the replace pattern AND to allow couplings to get 
            // substituted.
            if( source_child ) 
                source_child = DuplicateSubtree( source_child, keys, can_key, current_key );
            
            ASSERT( (source_child && source_child->IsFinal()) || (dest_child && dest_child->IsFinal()) )
                  ("When overlaying ")
                  (*source)
                  (" over ")
                  (*dest)
                  (", found a member that is intermediate in both (one must be final) even after substitution\n");
            
            if( source_child ) // Masked: where source is NULL, do not overwrite
            {
                // General overlaying policy: if the current child nodes of the source and dest are
                // compatible with overlying, then overlay them (means recursing into both) otherwise
                // just duplicate the source (means we only recurse the source).                                
                if( dest_child && source_child->IsLocalMatch(dest_child.get()) ) // overlaying
                {
                     Overlay( dest_child, source_child, keys, can_key, current_key );
                     ASSERT( dest_child );
                     ASSERT( dest_child->IsFinal() );
                }
                else // just copying
                {  
                    dest_child = source_child;
                    ASSERT( dest_child );
                    ASSERT( dest_child->IsFinal() );
                }
            }
            *dest_ptr = dest_child;
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }
    ASSERT( dest );
}


void RootedSearchReplace::Overlay( SequenceInterface *dest,
		                           SequenceInterface *source,
		                           CouplingKeys *keys,
		                           bool can_key,
		                           shared_ptr<Key> current_key ) const
{
	INDENT;
    // For now, always overwrite the dest
    // TODO smarter semantics for prepend, append etc based on NULLs in the sequence)
    dest->clear();

    TRACE("Overlaying sequence size %d\n", source->size() );
	FOREACH( const TreePtrInterface &p, *source )
	{
		ASSERT( p ); // present simplified scheme disallows NULL, see above
		TreePtr<Node> n = DuplicateSubtree( p, keys, can_key, current_key );
		if( TreePtr<SubSequence> ss = dynamic_pointer_cast<SubSequence>(n) )
		{
			TRACE("Expanding SubSequence length %d\n", ss->size() );
		    FOREACH( const TreePtrInterface &xx, *ss )
			    dest->push_back( xx );
   		}
		else
		{
			TRACE("Normal element, inserting %s directly\n", TypeInfo(n).name().c_str());
			dest->push_back( n );
		}
	}
}


void RootedSearchReplace::Overlay( CollectionInterface *dest,
   		                           CollectionInterface *source,
		                           CouplingKeys *keys,
		                           bool can_key,
		                           shared_ptr<Key> current_key ) const
{
	INDENT;
    // For now, always overwrite the dest
    // TODO smarter semantics for prepend, append etc based on NULLs in the sequence)
    dest->clear();

    TRACE("Overlaying collection size %d\n", source->size() );

	FOREACH( const TreePtrInterface &p, *source )
	{
		ASSERT( p ); // present simplified scheme disallows NULL, see above
		TreePtr<Node> n = DuplicateSubtree( p, keys, can_key, current_key );
		if( TreePtr<SubCollection> sc = dynamic_pointer_cast<SubCollection>(n) )
		{
			TRACE("Expanding SubCollection length %d\n", sc->size() );
			FOREACH( const TreePtrInterface &xx, *sc )
				dest->insert( xx );
		}
		else
		{
			TRACE("Normal element, inserting %s directly\n", TypeInfo(n).name().c_str());
			dest->insert( n );
		}
	}
}


// Duplicate an entire subtree, following the rules for inferno search and replace.
// We recurse through the subtree, using Duplicator to create the new nodes. We support
// substitution based on configured couplings, and we do not duplicate identifiers when
// substituting.
// TODO possible refactor: when we detect a coupling match, maybe recurse back into DuplicateSubtree
// and get the two OverlayPtrs during unwind.
TreePtr<Node> RootedSearchReplace::DuplicateSubtree( TreePtr<Node> source,
		                                                CouplingKeys *keys,
		                                                bool can_key,
		                                                shared_ptr<Key> current_key ) const
{
	INDENT;
	TRACE("Duplicating %s under_substitution=%p\n", ((string)*source).c_str(), current_key.get());
    TreePtr<Node> dest;
   TRACE("DuplicateSubtree source={");
	    Walk w(source);
	    bool first=true;
	    FOREACH( TreePtr<Node> n, w )
	    {
	    	if( !first )
	    		TRACE(", ");
	    	TRACE( n ? *n : string("NULL"));
	    	first=false;
	    }
	    TRACE("}\n"); // TODO put this in as a common utility somewhere


    // Are we substituting a stuff node? If so, see if we reached the terminus, and if
	// so come out of substitution.
	if( shared_ptr<StuffKey> stuff_key = dynamic_pointer_cast<StuffKey>(current_key) )
	{
		TRACE( "Substituting stuff: source=%s:%p, term=%s:%p\n",
				TypeInfo(source).name().c_str(), source.get(),
				TypeInfo(stuff_key->terminus).name().c_str(), stuff_key->terminus.get() );
		ASSERT( stuff_key->replace_pattern );
		TreePtr<StuffBase> replace_stuff = dynamic_pointer_cast<StuffBase>( stuff_key->replace_pattern );
		ASSERT( replace_stuff );
		if( replace_stuff->terminus &&      // There is a terminus in replace pattern
			source == stuff_key->terminus ) // and the present node is the terminus in the source pattern
		{
			TRACE( "Leaving substitution to duplicate terminus replace pattern at ")(*(replace_stuff->terminus))("\n" );
			dest = DuplicateSubtree( replace_stuff->terminus, keys, can_key, shared_ptr<Key>() ); // not in substitution any more
			TRACE( "Returning to substitution for rest of stuff\n" );
			return dest;
		}
	}

	if( TreePtr<RootedSlaveBase> rsb = dynamic_pointer_cast<RootedSlaveBase>(source) )
	{
		dest = DuplicateSubtree( rsb->GetThrough(), keys, can_key, current_key );
		if(!can_key) // do not run slaves until we have all the keys of the master 
        {
            RootedSearchReplace *slave = rsb.get();
    	    slave->pcontext = pcontext;
    	    (void)slave->DefaultRepeatingSearchReplace( &dest, *keys );
        }
		return dest;
	}

	if( TreePtr<SlaveBase> sb = dynamic_pointer_cast<SlaveBase>(source) )
	{
		dest = DuplicateSubtree( sb->GetThrough(), keys, can_key, current_key );
        if(!can_key) // do not run slaves until we have all the keys of the master 
        {
		    SearchReplace *slave = sb.get();
		    slave->pcontext = pcontext;
    	    (void)slave->DefaultRepeatingSearchReplace( &dest, *keys );
        }
		return dest;
	}    
	
    TreePtr<Node> newsource=source, key=TreePtr<Node>(), overlay=source;
    if( shared_ptr<SoftReplacePattern> srp = dynamic_pointer_cast<SoftReplacePattern>( source ) )
    {
        // Substitute is an identifier, so preserve its uniqueness by just returning
        // the same node. Don't do any more - we wouldn't want to change the
        // identifier in the tree even if it had members, lol!
        ASSERT( !current_key )( "Found soft replace pattern while under substitution\n" );
        key = newsource = srp->DuplicateSubtree( this, keys, can_key );
        ASSERT( newsource );
    }

    // Allow this to key a coupling if required
    dest = keys->KeyAndSubstitute( key, source, this, can_key );
    ASSERT( !dest || !current_key )("Should only find a match in patterns"); // We'll never find a match when we're under substitution, because the
  
    if( shared_ptr<OverlayBase> ob = dynamic_pointer_cast<OverlayBase>( source ) )
    {
        // Substitute is an identifier, so preserve its uniqueness by just returning
        // the same node. Don't do any more - we wouldn't want to change the
        // identifier in the tree even if it had members, lol!
        ASSERT( !current_key )( "Found overlay pattern while under substitution\n" ); // TODO maybe disallow all special nodes when under substitution?
        // TODO I think we should recurse instead of changing source, so other 
        // checks can be made eg for other special nodes

        // Note that this can effectively throw away the ob->base if either (a) we were coupled
        // or (b) ob->overlay is not a compatible overly. It's OK because ob->base was used
        // in the search pattern that must have matched for us to get here in the first place.
        if( ob->overlay->IsLocalMatch(ob->base.get()) )
        {
            overlay = ob->overlay;
        // Allow this to key a coupling if required
            if( !dest )
            {
                dest = keys->KeyAndSubstitute( key, ob->overlay, this, can_key );                                
                ASSERT( !dest || !current_key )("Should only find a match in patterns"); // We'll never find a match when we're under substitution, because the
                if( dest)
                    return dest;
                newsource = ob->base;
            }
        }
        else
        {
            return DuplicateSubtree( ob->overlay, keys, can_key, current_key );
        }
    }

    if( !dest )
    {
        // No coupling to key to, so just make a copy
    	TRACE("Did not substitute (newsource is %s)\n", TypeInfo(newsource).name().c_str());
        
        ASSERT( !dynamic_pointer_cast<SpecialBase>(newsource) )
              ("Special nodes in replace pattern must be keyed\n")
              ("source=")
              (*source)
              (" newsource=")
              (*newsource)
              (" dest=NULL\n");
        
        TRACE("duplicating supplied node\n");
        // Make the new node (destination node)
        shared_ptr<Cloner> dup_dest = newsource->Duplicate(newsource);
        dest = dynamic_pointer_cast<Node>( dup_dest );
        ASSERT(dest);
        ClearPtrs( dest ); // must clear otherwise we get inks into the input program 

        // If not substituting a Stuff node, remember this node is dirty for GreenGrass restriction
        // Also dirty the dest if the source was dirty when we are substituting Stuff
        // TODO should we use source or newsource here?
        if( !current_key || !dynamic_pointer_cast<StuffKey>(current_key) || dirty_grass.find( source ) != dirty_grass.end() )
            dirty_grass.insert( dest );
    }
    
    ASSERT( dest );
    
    // Don't overlay special nodes 
    if( !dynamic_pointer_cast<SpecialBase>( overlay ) )
    {
        // Overlaying requires type compatibility - check for this
	    ASSERT( overlay->IsLocalMatch(dest.get()) )
	    	  ( "When overlaying, the replace pattern ")(*overlay)(" must be a non-strict superclass of substitute ")(*dest)(", so that its members are a subset");
	    TRACE();
	    
        // Copy the source over, except for any NULLs in the source. If source is superclass
	    // of destination (i.e. has possibly fewer members) the missing ones will be left alone.
        Overlay( dest, overlay, keys, can_key, current_key );
        
        ASSERT( dest );
    }        

    return dest;
}


TreePtr<Node> RootedSearchReplace::MatchingDuplicateSubtree( TreePtr<Node> source,
		                                                        CouplingKeys *keys ) const
{
    //TODO can_key should be an enum eg RelpacePass {KEYING, SUBSTITUTING} for clarity
    if( keys )
    {
     	// Do a two-pass matching process: first get the keys...
       	TRACE("doing replace KEYING pass....\n");
        (void)DuplicateSubtree( source, keys, true );
        TRACE("replace KEYING pass\n" );

	    // Now restrict the search according to the couplings
    	TRACE("doing replace SUBSTITUTING pass....\n");
        TreePtr<Node> r = DuplicateSubtree( source, keys, false );
        TRACE("replace SUBSTITUTING pass\n" );
        return r;
    }
    else
    {
    	// No coupling, so just call straight through this layer
    	return DuplicateSubtree( source, NULL, false );
    }
}


Result RootedSearchReplace::SingleSearchReplace( TreePtr<Node> *proot,
		                                                              TreePtr<Node> search_pattern,
		                                                              TreePtr<Node> replace_pattern,
		                                                              CouplingKeys keys ) // Pass by value is intentional - changes should not propogate back to caller
{
	TRACE("%p Begin search\n", this);
	Result r = Compare( *proot, search_pattern, &keys, true );
	if( r != FOUND )
		return NOT_FOUND;

    if( replace_pattern )
    {
    	TRACE("%p Search successful, now replacing\n", this);
        *proot = MatchingDuplicateSubtree( replace_pattern, &keys );
    }

    return FOUND;
}


// Perform search and replace on supplied program based
// on supplied patterns and couplings. Does search and replace
// operations repeatedly until there are no more matches. Returns how
// many hits we got.
int RootedSearchReplace::RepeatingSearchReplace( TreePtr<Node> *proot,
	                                             TreePtr<Node> search_pattern,
	                                             TreePtr<Node> replace_pattern,
	                                             CouplingKeys keys ) // Pass by value is intentional - changes should not propagate back to caller
{
	dirty_grass.clear();

    int i=0;
    while(i<20) // TODO!!
    {
    	Result r = SingleSearchReplace( proot,
    			                        search_pattern,
    			                        replace_pattern,
    			                        keys );
    	TRACE("%p result %d", this, r);        
    	if( r != FOUND )
            break;
        Validate()( *pcontext, proot );
       	//ASSERT(i<100)("Too many hits");
        i++;
    }

    TRACE("%p exiting", this);
    return i;
}


void RootedSearchReplace::DefaultRepeatingSearchReplace( TreePtr<Node> *proot,
										            	 CouplingKeys keys )
{
	(void)RepeatingSearchReplace( proot, search_pattern, replace_pattern, keys );
}


// Do a search and replace based on patterns stored in our members
void RootedSearchReplace::operator()( TreePtr<Node> c, TreePtr<Node> *proot )
{
	if( ReadArgs::pattern_graph && ReadArgs::quitafter == 0 )
	{
        Graph g;
        g( this );
        exit(0); // There's nothing - literally NOTHING -left to do
	}

	// If the initial root and context are the same node, then arrange for the context
	// to follow the root node as we modify it (in SingleSearchReplace()). This ensures
	// new declarations can be found in slave searches. We could get the
	// same effect by taking the context as a reference, but leave it like this for now.
	// If *proot is under context, then we're OK as long as proot points to the actual
	// tree node - then the walk at context will follow the new *proot pointer and get
	// into the new subtree.
	if( c == *proot )
		pcontext = proot;
	else
		pcontext = &c;

	// Do the search and replace with before and after validation
	Validate()( *pcontext, proot );
	DefaultRepeatingSearchReplace( proot );
	Validate()( *pcontext, proot );

    pcontext = NULL; // just to avoid us relying on the context outside of a search+replace pass
}


SearchReplace::SearchReplace( TreePtr<Node> sp,
                              TreePtr<Node> rp,
                              CouplingSet m ) :
    RootedSearchReplace( sp, rp, m )                              
{
}

SearchReplace::SearchReplace( TreePtr<Node> sp,
                              TreePtr<Node> rp,
                              int i ) :
    RootedSearchReplace( sp, rp, i )                              
{
}

void SearchReplace::DefaultRepeatingSearchReplace( TreePtr<Node> *proot,
                                                   CouplingKeys keys )
{
    // Make a non-rooted search and replace (ie where the base of the search pattern
    // does not have to be the root of the whole program tree).
    // Insert a Stuff node as root of the search pattern
    TreePtr< Stuff<Scope> > stuff( new Stuff<Scope> );
    stuff->terminus = search_pattern;

    if( replace_pattern ) // Is there a replace pattern?
    {
        // Insert a Stuff node as root of the replace pattern
        TreePtr< ::Overlay<Scope> > overlay( new ::Overlay<Scope> );
        stuff->terminus = overlay;
        overlay->base = search_pattern;
        overlay->overlay = replace_pattern;

        // Key them together
        Coupling stuff_match(( stuff ));
        couplings.insert( stuff_match );

        // Configure the rooted implementation with new patterns and couplings
        (void)RepeatingSearchReplace( proot, stuff, stuff, keys );

        // Undo the change to the couplings we made earlier
        couplings.erase( stuff_match );
    }
    else
    {
        // Configure the rooted implementation with new pattern
        (void)RepeatingSearchReplace( proot, stuff, TreePtr<Node>(), keys );
    }
}

void RootedSearchReplace::Test()
{
    RootedSearchReplace sr = RootedSearchReplace( MakeTreePtr<Nop>() );

    {
        // single node with topological wildcarding
        TreePtr<Void> v(new Void);
        ASSERT( sr.Compare( v, v ) == FOUND );
        TreePtr<Boolean> b(new Boolean);
        ASSERT( sr.Compare( v, b ) == NOT_FOUND );
        ASSERT( sr.Compare( b, v ) == NOT_FOUND );
        TreePtr<Type> t(new Type);
        ASSERT( sr.Compare( v, t ) == FOUND );
        ASSERT( sr.Compare( t, v ) == NOT_FOUND );
        ASSERT( sr.Compare( b, t ) == FOUND );
        ASSERT( sr.Compare( t, b ) == NOT_FOUND );
        
        // node points directly to another with TC
        TreePtr<Pointer> p1(new Pointer);
        p1->destination = v;
        ASSERT( sr.Compare( p1, b ) == NOT_FOUND );
        ASSERT( sr.Compare( p1, p1 ) == FOUND );
        TreePtr<Pointer> p2(new Pointer);
        p2->destination = b;
        ASSERT( sr.Compare( p1, p2 ) == NOT_FOUND );
        p2->destination = t;
        ASSERT( sr.Compare( p1, p2 ) == FOUND );
        ASSERT( sr.Compare( p2, p1 ) == NOT_FOUND );
    }
    
    {
        // string property
        TreePtr<SpecificString> s1( new SpecificString("here") );
        TreePtr<SpecificString> s2( new SpecificString("there") );
        ASSERT( sr.Compare( s1, s1 ) == FOUND );
        ASSERT( sr.Compare( s1, s2 ) == NOT_FOUND );
    }    
    
    {
        // int property
        TreePtr<SpecificInteger> i1( new SpecificInteger(3) );
        TreePtr<SpecificInteger> i2( new SpecificInteger(5) );
        TRACE("  %s %s\n", ((llvm::APSInt)*i1).toString(10).c_str(), ((llvm::APSInt)*i2).toString(10).c_str() );
        ASSERT( sr.Compare( i1, i1 ) == FOUND );
        ASSERT( sr.Compare( i1, i2 ) == NOT_FOUND );
    }    
    
    {
        // node with sequence, check lengths 
        TreePtr<Compound> c1( new Compound );
        ASSERT( sr.Compare( c1, c1 ) == FOUND );
        TreePtr<Nop> n1( new Nop );
        c1->statements.push_back( n1 );
        ASSERT( sr.Compare( c1, c1 ) == FOUND );
        TreePtr<Nop> n2( new Nop );
        c1->statements.push_back( n2 );
        ASSERT( sr.Compare( c1, c1 ) == FOUND );
        TreePtr<Compound> c2( new Compound );
        ASSERT( sr.Compare( c1, c2 ) == NOT_FOUND );
        TreePtr<Nop> n3( new Nop );
        c2->statements.push_back( n3 );
        ASSERT( sr.Compare( c1, c2 ) == NOT_FOUND );
        TreePtr<Nop> n4( new Nop );
        c2->statements.push_back( n4 );
        ASSERT( sr.Compare( c1, c2 ) == FOUND );
        TreePtr<Nop> n5( new Nop );
        c2->statements.push_back( n5 );
        ASSERT( sr.Compare( c1, c2 ) == NOT_FOUND );
    }

    {
        // node with sequence, TW 
        TreePtr<Compound> c1( new Compound );
        TreePtr<Nop> n1( new Nop );
        c1->statements.push_back( n1 );
        TreePtr<Compound> c2( new Compound );
        TreePtr<Statement> s( new Statement );
        c2->statements.push_back( s );
        ASSERT( sr.Compare( c1, c2 ) == FOUND );
        ASSERT( sr.Compare( c2, c1 ) == NOT_FOUND );
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
}
