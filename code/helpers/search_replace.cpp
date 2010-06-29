#include "search_replace.hpp"
#include "validate.hpp"

// Constructor remembers search pattern, replace pattern and any supplied match sets as required
RootedSearchReplace::RootedSearchReplace( SharedPtr<Node> sp,
                                          SharedPtr<Node> rp,
                                          CouplingSet m,
                                          vector<RootedSearchReplace *> s )
{
	Configure( sp, rp, m, s );
}


void RootedSearchReplace::Configure( SharedPtr<Node> sp,
                                     SharedPtr<Node> rp,
                                     CouplingSet m,
                                     vector<RootedSearchReplace *> s )
{
    search_pattern = sp;
    replace_pattern = rp;
    couplings = m;
    slaves = s;

    Validate v(true);
    v(search_pattern, &search_pattern);
    v(replace_pattern, &replace_pattern);

    // If we have a slave, copy its match sets into ours so we have a full set
    // of all the match sets - this will be used across the board. Note that
    // the non-rooted SearchReplace adds a new match set.
    FOREACH( RootedSearchReplace *slave, slaves )
	{
		for( CouplingSet::iterator msi = slave->couplings.begin();
             msi != slave->couplings.end();
             msi++ )
		    couplings.insert( *msi );
	}

	TRACE("Merged match sets, I have %d\n", couplings.size() );

    FOREACH( RootedSearchReplace *slave, slaves )
    {
    	slave->couplings = couplings;
	}
} 


// Destructor tries not to leak memory lol
RootedSearchReplace::~RootedSearchReplace()
{
}


// Helper for DecidedCompare that does the actual match testing work for the children and recurses.
// Also checks for soft matches.
Result RootedSearchReplace::DecidedCompare( SharedPtr<Node> x,
		                                             SharedPtr<Node> pattern,
		                                             CouplingKeys *keys,
		                                             bool can_key,
		                                             Conjecture &conj ) const
{
	if( !pattern )    // NULL matches anything in search patterns (just to save typing)
		return FOUND;

    if( SharedPtr<SoftSearchPattern> ssp = dynamic_pointer_cast<SoftSearchPattern>(pattern) )
    {
    	// Hand over to any soft search functionality in the search pattern node
    	Result r = ssp->DecidedCompare( this, x, keys, can_key, conj );
    	if( r != FOUND )
    		return NOT_FOUND;
    }
    else if( SharedPtr<StuffBase> stuff_pattern = dynamic_pointer_cast<StuffBase>(pattern) )
    {
    	// Invoke stuff node compare
    	return DecidedCompare( x, stuff_pattern, keys, can_key, conj );
    }
    else
    {
    	// Not a soft node, so handle explicitly
		// Check whether the present node matches
		if( !pattern->IsLocalMatch(x.get()) )
			return NOT_FOUND;

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
				ASSERT( x_seq )( "itemise for target didn't match itemise for pattern");
				TRACE("Member %d is Sequence, target %d elts, pattern %d elts\n", i, x_seq->size(), pattern_seq->size() );
				r = DecidedCompare( *x_seq, *pattern_seq, keys, can_key, conj );
			}
			else if( CollectionInterface *pattern_col = dynamic_cast<CollectionInterface *>(pattern_memb[i]) )
			{
				CollectionInterface *x_col = dynamic_cast<CollectionInterface *>(x_memb[i]);
				ASSERT( x_col )( "itemise for target didn't match itemise for pattern");
				TRACE("Member %d is Collection, target %d elts, pattern %d elts\n", i, x_col->size(), pattern_col->size() );
				r = DecidedCompare( *x_col, *pattern_col, keys, can_key, conj );
			}
			else if( SharedPtrInterface *pattern_ptr = dynamic_cast<SharedPtrInterface *>(pattern_memb[i]) )
			{
				SharedPtrInterface *x_ptr = dynamic_cast<SharedPtrInterface *>(x_memb[i]);
				ASSERT( x_ptr )( "itemise for target didn't match itemise for pattern");
				TRACE("Member %d is SharedPtr, pattern ptr=%p\n", i, pattern_ptr->get());
				r = DecidedCompare( *x_ptr, SharedPtr<Node>(*pattern_ptr), keys, can_key, conj );
			}
			else
			{
				ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a SharedPtr");
			}

			if( r != FOUND )
				return NOT_FOUND;
		}
    }
   
    // If we got here, the node matched the search pattern. Now apply match sets
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
	// Attempt to match all the elements between start and the end of the sequence; stop
	// if either pattern or subject runs out.
	ContainerInterface::iterator xit = x.begin();
	ContainerInterface::iterator pit = pattern.begin();

	while( pit != pattern.end() )
	{
		// Get the next element of the pattern
		SharedPtr<Node> pe( *pit );
		++pit;
	    if( !pe || dynamic_pointer_cast<StarBase>(pe) )
	    {
	    	TRACE("Star (pe is %d)\n", (int)!!pe);
			// We have a Star type wildcard that can match multiple elements. At present,
			// NULL is interpreted as a Star (but cannot go in a match set).

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
	    		ASSERT( !dynamic_pointer_cast<StarBase>(SharedPtr<Node>(*pit)) )
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
		    // Now make a copy of the elements that matched the star and apply match sets
		    if( pe )
		    {
		    	SharedPtr<SubSequence> ss( new SubSequence);
		    	for( ContainerInterface::iterator it=xit_begin_star; it != xit; ++it )
		    		ss->push_back( *it );
				// Apply match sets to this Star and matched range
				if( keys )
		    		if( !keys->KeyAndRestrict( SharedPtr<Node>(ss), pe, this, can_key ) )
		        	    return NOT_FOUND;
		    }
	    }
	    else // not a Star so match singly...
	    {
	    	TRACE("Not a star\n");
			// If there is one more element in x, see if it matches the pattern
			//SharedPtr<Node> xe( x[xit] );
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
    // Make a copy of the elements in the tree. As we go though the pattern, we'll erase them from
	// here so that (a) we can tell which ones we've done so far and (b) we can get the remainder
	// after decisions.
	// TODO is there some stl algorithm for this?
    SharedPtr<SubCollection> xremaining( new SubCollection );
    FOREACH( const SharedPtrInterface &xe, x )
        xremaining->insert( xe );

    SharedPtr<StarBase> star;
    bool seen_star = false;

    for( CollectionInterface::iterator pit = pattern.begin(); pit != pattern.end(); ++pit )
    {
    	TRACE("Collection compare %d remain out of %d; looking at %s in pattern\n",
    			xremaining->size(),
    			pattern.size(),
    			TypeInfo( SharedPtr<Node>(*pit) ).name().c_str() );
    	SharedPtr<StarBase> maybe_star = dynamic_pointer_cast<StarBase>( SharedPtr<Node>(*pit) );

        if( maybe_star || !(*pit) ) // Star or NULL in pattern collection?
        {
        	ASSERT(!seen_star)("Only one Star node (or NULL ptr) allowed in a search pattern Collection");
        	// TODO remove this restriction - I might want to match one star and leave another unmatched.
            star = maybe_star; // remember for later and skip to next pattern
            seen_star = true;
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
			if( !DecidedCompare( *xit, SharedPtr<Node>(*pit), keys, can_key, conj ) )
			    return NOT_FOUND;
	    }
    }

    // Now handle the star if there was one; all the non-star matches have been erased from
    // the collection, leaving only the star matches.

    if( !xremaining->empty() && !seen_star )
    	return NOT_FOUND; // there were elements left over and no star to match them against

    // If we got here, the node matched the search pattern. Now apply match sets
    TRACE("seen_star %d  star %p\n", seen_star, star.get() );
    if( keys && seen_star && star )
        if( !keys->KeyAndRestrict( SharedPtr<Node>(xremaining), star, this, can_key ) )
        	return NOT_FOUND;

	return FOUND;
}


// Helper for DecidedCompare that does the actual match testing work for the children and recurses.
// Also checks for soft matches.
Result RootedSearchReplace::DecidedCompare( SharedPtr<Node> x,
		                                             SharedPtr<StuffBase> stuff_pattern,
		                                             CouplingKeys *keys,
		                                             bool can_key,
		                                             Conjecture &conj ) const
{
	ASSERT( stuff_pattern->terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");

	// Define beginning and end
	WalkingIterator wbegin( x, stuff_pattern->restrictor );
	WalkingIterator wend;

	// Get decision from conjecture
	ContainerInterface::iterator thistime = conj.HandleDecision( wbegin, wend );
	if( thistime == (ContainerInterface::iterator)wend )
		return NOT_FOUND; // ran out of choices

	// Try out comparison at this position
	Result r = DecidedCompare( *thistime, stuff_pattern->terminus, keys, can_key, conj );

    // If we got this far, do the match sets
    if( keys && r )
    {
    	shared_ptr<StuffKey> key( new StuffKey );
    	key->root = x;
    	key->terminus = *thistime;
    	//SharedPtr<Node> = SharedPtrInterface
        r = keys->KeyAndRestrict( KeyPtr(key),
        		                  stuff_pattern,
        		                  this,
        		                  can_key );
    }
	return r;
}


Result RootedSearchReplace::MatchingDecidedCompare( SharedPtr<Node> x,
		                                                     SharedPtr<Node> pattern,
		                                                     CouplingKeys *keys,
		                                                     bool can_key,
		                                                     Conjecture &conj ) const
{
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
        
	    // Now restrict the search according to the match sets
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
    	// No match set, so just call straight through this layer
    	conj.PrepareForDecidedCompare();
    	return DecidedCompare( x, pattern, NULL, false, conj );
    }
}


Result RootedSearchReplace::Compare( SharedPtr<Node> x,
		                                      SharedPtr<Node> pattern,
		                                      CouplingKeys *keys,
		                                      bool can_key ) const
{
	TRACE("Comparing x=%s with pattern=%s, match keys at %p\n", typeid(*x).name(), typeid(*pattern).name(), keys );
	// Create the conjecture object we will use for this compare, and then go
	// into the recursive compare function
	Conjecture conj;
	Result r = conj.Search( x, pattern, keys, can_key, this );
	return r;
}


// Clear all pointer members in supplied dest to NULL
void RootedSearchReplace::ClearPtrs( SharedPtr<Node> dest ) const
{
    vector< Itemiser::Element * > dest_memb = dest->Itemise();
    for( int i=0; i<dest_memb.size(); i++ )
    {
        if( SequenceInterface *dest_seq = dynamic_cast<SequenceInterface *>(dest_memb[i]) )                
        {
        	for( ContainerInterface::iterator i=dest_seq->begin(); i!=dest_seq->end(); ++i )
            {
                SharedPtr<Node> p;
                i.Overwrite( &p ); // TODO using Overwrite() to support unordered - but does this fuction even make sense forn unordered?
            }
        }            
        else if( SharedPtrInterface *dest_ptr = dynamic_cast<SharedPtrInterface *>(dest_memb[i]) )         
        {
            *dest_ptr = SharedPtr<Node>();
        }
    }       
}


// Helper for DuplicateSubtree, fills in children of dest node from source node when source node child
// is non-NULL. This means we can call this multiple times with different sources and get a priority 
// scheme.
void RootedSearchReplace::Overlay( SharedPtr<Node> dest,
		                           SharedPtr<Node> source,
		                           CouplingKeys *keys,
		                           bool can_key,
		                           KeyPtr current_key ) const
{
    ASSERT( source->IsLocalMatch(dest.get()) )("source must be a non-strict subclass of destination, so that it does not have more members");

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of source, and itemises over that type. dest must
    // be dynamic_castable to source's type.
    vector< Itemiser::Element * > source_memb = source->Itemise();
    vector< Itemiser::Element * > dest_memb = source->Itemise( dest.get() ); // Get the members of dest corresponding to source's class
    ASSERT( source_memb.size() == dest_memb.size() );

    TRACE("Overlaying %d members %s over %s\n", dest_memb.size(), TypeInfo(source).name().c_str(), TypeInfo(dest).name().c_str());

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
        else if( SharedPtrInterface *source_ptr = dynamic_cast<SharedPtrInterface *>(source_memb[i]) )         
        {
        	TRACE();
            SharedPtrInterface *dest_ptr = dynamic_cast<SharedPtrInterface *>(dest_memb[i]);
            ASSERT( dest_ptr )( "itemise for target didn't match itemise for source");
            if( *source_ptr ) // Masked: where source is NULL, do not overwrite
                *dest_ptr = DuplicateSubtree( *source_ptr, keys, can_key, current_key );
            if( !current_key )
            	ASSERT( *dest_ptr )("Found NULL in replace pattern without a match set to substitute it");
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }        
    TRACE();
}


void RootedSearchReplace::Overlay( SequenceInterface *dest,
		                           SequenceInterface *source,
		                           CouplingKeys *keys,
		                           bool can_key,
		                           KeyPtr current_key ) const
{
    // For now, always overwrite the dest
    // TODO smarter semantics for prepend, append etc based on NULLs in the sequence)
    dest->clear();

    TRACE("duplicating sequence size %d\n", source->size() );
	FOREACH( const SharedPtrInterface &p, *source )
	{
		ASSERT( p ); // present simplified scheme disallows NULL, see above
		SharedPtr<Node> n = DuplicateSubtree( p, keys, can_key, current_key );
		if( SharedPtr<SubSequence> ss = dynamic_pointer_cast<SubSequence>(n) )
		{
			TRACE("Expanding SubSequence length %d\n", ss->size() );
		    FOREACH( const SharedPtrInterface &xx, *ss )
			    dest->push_back( xx );
   		}
		else
		{
			TRACE("Normal element, inserting directly\n");
			dest->push_back( n );
		}
	}
}


void RootedSearchReplace::Overlay( CollectionInterface *dest,
   		                           CollectionInterface *source,
		                           CouplingKeys *keys,
		                           bool can_key,
		                           KeyPtr current_key ) const
{
    // For now, always overwrite the dest
    // TODO smarter semantics for prepend, append etc based on NULLs in the sequence)
    dest->clear();

    TRACE("duplicating collection size %d\n", source->size() );

	FOREACH( const SharedPtrInterface &p, *source )
	{
		ASSERT( p ); // present simplified scheme disallows NULL, see above
		SharedPtr<Node> n = DuplicateSubtree( p, keys, can_key, current_key );
		if( SharedPtr<SubCollection> sc = dynamic_pointer_cast<SubCollection>(n) )
		{
			TRACE("Expanding SubCollection length %d\n", sc->size() );
			FOREACH( const SharedPtrInterface &xx, *sc )
				dest->insert( xx );
		}
		else
		{
			TRACE("Normal element, inserting directly\n");
			dest->insert( n );
		}
	}
}


// Duplicate an entire subtree, following the rules for inferno search and replace.
// We recurse through the subtree, using Duplicator to create the new nodes. We support
// substitution based on configured match sets, and we do not duplicate identifiers when
// substituting.
// TODO possible refactor: when we detect a match set match, maybe recurse back into DuplicateSubtree
// and get the two OverlayPtrs during unwind.
SharedPtr<Node> RootedSearchReplace::DuplicateSubtree( SharedPtr<Node> source,
		                                                CouplingKeys *keys,
		                                                bool can_key,
		                                                KeyPtr current_key ) const
{
	TRACE("Duplicating %s under_substitution=%p\n", TypeInfo(source).name().c_str(), current_key.get());

    // Are we substituting a stuff node? If so, see if we reached the terminus, and if
	// so come out of substitution.
	if( shared_ptr<StuffKey> stuff_key = dynamic_pointer_cast<StuffKey>(current_key) )
	{
		ASSERT( stuff_key->replace_pattern );
		SharedPtr<StuffBase> replace_stuff = dynamic_pointer_cast<StuffBase>( stuff_key->replace_pattern );
		ASSERT( replace_stuff );
		if( replace_stuff->terminus &&      // There is a terminus in replace pattern
			source == stuff_key->terminus ) // and the present node is the terminus in the source pattern
		{
			TRACE( "Substituting stuff: source=%s:%p, term=%s:%p\n",
					TypeInfo(source).name().c_str(), source.get(),
					TypeInfo(stuff_key->terminus).name().c_str(), stuff_key->terminus.get() );
			TRACE( "Leaving substitution to duplicate terminus replace pattern\n" );
			return DuplicateSubtree( replace_stuff->terminus, keys, can_key, KeyPtr() ); // not in substitution any more
		}
	}

	SharedPtr<Node> dest = SharedPtr<Node>();
	dest = keys->KeyAndSubstitute( KeyPtr(), source, this, can_key );
    ASSERT( !dest || !current_key )("Should only find a match in patterns"); // We'll never find a match when we're under substitution, because the
                                                                             // source is actually a match key already, so not in any match sets
    if( dest )
    {
    	TRACE("Substituted, got %s (source is %s)\n", TypeInfo(dest).name().c_str(), TypeInfo(source).name().c_str());

		// Do NOT overlay soft patterns TODO inelegant?
		if( !dynamic_pointer_cast<SoftReplacePattern>( source ) &&
			!dynamic_pointer_cast<StarBase>( source ) &&
			!dynamic_pointer_cast<StuffBase>( source ) )
		{
   	    }
		else
		{
			TRACE();
			return dest;
		}
    }
    else
    {
    	TRACE("Did not substitute  (source is %s)\n", TypeInfo(source).name().c_str());
    	ASSERT( !dynamic_pointer_cast<StuffBase>(source) )("Stuff nodes in replace pattern must be keyed\n");

       	// Allow a soft replace pattern to act
		if( SharedPtr<SoftReplacePattern> srp = dynamic_pointer_cast<SoftReplacePattern>( source ) )
		{
			// Substitute is an identifier, so preserve its uniqueness by just returning
			// the same node. Don't do any more - we wouldn't want to change the
			// identifier in the tree even if it had members, lol!
			TRACE("Invoke soft replace pattern %s\n", TypeInfo(srp).name().c_str() );
			ASSERT( !current_key )( "Found soft replace pattern while under substitution\n" );
			SharedPtr<Node> newsource = srp->DuplicateSubtree( this, keys, can_key );
			ASSERT( newsource );

			// Allow this to key a match set if required
			SharedPtr<Node> subs = keys->KeyAndSubstitute( newsource, source, this, can_key );
			if( subs )
				return subs;
			else
				return newsource;
		}

    	TRACE("duplicating supplied node\n");
		// Make the new node (destination node)
		shared_ptr<Cloner> dup_dest = source->Duplicate(source);
		dest = dynamic_pointer_cast<Node>( dup_dest );
		ASSERT(dest);

		// Make all members in the destination be NULL
		ClearPtrs( dest );
    }
    
	// Overlaying requires type compatibility - check for this
	ASSERT( source->IsLocalMatch(dest.get()) )
		  ( "replace pattern ")(*source)(" must be a non-strict superclass of substitute ")(*dest)(", so that its members are a subset");
	TRACE();
	// Copy the source over,  except for any NULLs in the source. If source is superclass
	// of destination (i.e. has possibly fewer members) the missing ones will be left alone.
	Overlay( dest, source, keys, can_key, current_key );

    ASSERT( dest );
    TRACE();

    return dest;
}


SharedPtr<Node> RootedSearchReplace::MatchingDuplicateSubtree( SharedPtr<Node> source,
		                                                        CouplingKeys *keys ) const
{
    if( keys )
    {
     	// Do a two-pass matching process: first get the keys...
       	TRACE("doing replace KEYING pass....\n");
        (void)DuplicateSubtree( source, keys, true );
        TRACE("replace KEYING pass\n" );

	    // Now restrict the search according to the match sets
    	TRACE("doing replace SUBSTITUTING pass....\n");
        SharedPtr<Node> r = DuplicateSubtree( source, keys, false );
        TRACE("replace SUBSTITUTING pass\n" );
        return r;
    }
    else
    {
    	// No match set, so just call straight through this layer
    	return DuplicateSubtree( source, NULL, false );
    }
}


#include "render/graph.hpp" // TODO get rid


Result RootedSearchReplace::SingleSearchReplace( SharedPtr<Node> *proot,
		                                                              SharedPtr<Node> search_pattern,
		                                                              SharedPtr<Node> replace_pattern,
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

    int i=0;
    FOREACH( RootedSearchReplace *slave, slaves )
    {
    	TRACE("%p Running slave\n", this);
    	int num = slave->RepeatingSearchReplace( proot, slave->search_pattern, slave->replace_pattern, keys );
    	TRACE("%p slave %d got %d hits\n", this, i++, num);
    }

    return FOUND;
}


// Perform search and replace on supplied program based
// on supplied patterns and match sets. Does search and replace
// operations repeatedly until there are no more matches. Returns how
// many hits we got.
int RootedSearchReplace::RepeatingSearchReplace( SharedPtr<Node> *proot,
	                                             SharedPtr<Node> search_pattern,
	                                             SharedPtr<Node> replace_pattern,
	                                             CouplingKeys keys ) // Pass by value is intentional - changes should not propagate back to caller
{
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
       	//ASSERT(i<100)("Too many hits");
        i++;
    }
    TRACE("%p exiting", this);
    return i;
}

// Do a search and replace based on patterns stored in our members
void RootedSearchReplace::operator()( SharedPtr<Node> c, SharedPtr<Node> *proot )
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

	// Do the search and replace with before and after vaidation
	Validate()( *pcontext, proot );
	(void)RepeatingSearchReplace( proot, search_pattern, replace_pattern );
	Validate()( *pcontext, proot );


    pcontext = NULL; // just to avoid us relying on the context outside of a search+replace pass
}


// Find a match set containing the supplied node
Coupling CouplingKeys::FindCoupling( SharedPtr<Node> node,
		                                                                       const CouplingSet &couplings )
{
	ASSERT( this );
	Coupling found; // returns an empty coupling if not found
	for( CouplingSet::iterator msi = couplings.begin();
         msi != couplings.end();
         msi++ )
    {
        Coupling::iterator ni = msi->find( node );
        if( ni != msi->end() )
        {
        	ASSERT( found.empty() )("Found more than one coupling for a node - please merge the couplings");
        	found = *msi;
        }
    }
    return found;
}



Result CouplingKeys::KeyAndRestrict( SharedPtr<Node> x,
		                                                                    SharedPtr<Node> pattern,
		                                                                    const RootedSearchReplace *sr,
		                                                                    bool can_key )
{
	KeyPtr key( new Key );
	key->root = x;
	return KeyAndRestrict( key, pattern, sr, can_key );
}

Result CouplingKeys::KeyAndRestrict( KeyPtr key,
		                                                                    SharedPtr<Node> pattern,
		                                                                    const RootedSearchReplace *sr,
		                                                                    bool can_key )
{
	ASSERT( this );
	// Find a match set for this node. If the node is not in a match set then there's
	// nothing for us to do, so return without restricting the search.
	Coupling coupling = FindCoupling( pattern, sr->couplings );
	if( coupling.empty() )
		return FOUND;

	// If we're keying and we haven't keyed this node so far, key it now
	TRACE("MATCH: can_key=%d\n", (int)can_key);
	if( can_key && !keys_map[coupling] )
	{
		TRACE("keying... key ptr %p new value %p, presently %d keys out of %d couplings\n",
				keys_map[coupling].get(), key.get(),
				keys_map.size(), sr->couplings.size() );

		keys_map[coupling] = key;

		return FOUND; // always match when keying (could restrict here too as a slight optimisation, but KISS for now)
	}

    // Always restrict
	// We are restricting the search, and this node has been keyed, so compare the present tree node
	// with the tree node stored for the match set. This comparison should not match any match sets
	// (it does not include stuff from any search or replace pattern) so do not allow match sets.
	// Since collections (which require decisions) can exist within the tree, we must allow iteration
	// through choices, and since the number of decisions seen may vary, we must start a new conjecture.
	// Therefore, we recurse back to Compare().
	ASSERT( keys_map[coupling] ); // should have been caught by CheckMatchSetsKeyed()
	Result r;
	if( key->root != keys_map[coupling]->root )
		r = sr->Compare( key->root, keys_map[coupling]->root );
	else
		r = FOUND; // TODO optimisation being done in wrong place
	TRACE("result %d\n", r);
	return r;
}

SharedPtr<Node> CouplingKeys::KeyAndSubstitute( SharedPtr<Node> x,
                                                                   SharedPtr<Node> pattern,
                                                                   const RootedSearchReplace *sr,
                                                                   bool can_key )
{
	KeyPtr key( new Key );
	key->root = x;
	return KeyAndSubstitute( key, pattern, sr, can_key );
}

// Note return is NULL in all cases unless we substituted in which case it is the result of the
// substitution, duplicated for our convenience. Always check the return value for NULL.
SharedPtr<Node> CouplingKeys::KeyAndSubstitute( KeyPtr key, // key may be NULL meaning we are not allowed to key the node
		                                                           SharedPtr<Node> pattern,
		                                                           const RootedSearchReplace *sr,
		                                                           bool can_key )
{
	ASSERT( this );
	ASSERT( !key || key->root != pattern ); // just a general usage check

	// Find a match set for this node. If the node is not in a match set then there's
	// nothing for us to do, so return without restricting the search.
	Coupling coupling = FindCoupling( pattern, sr->couplings );
	if( coupling.empty() )
		return SharedPtr<Node>();
	TRACE("MATCH: ");

	// If we're keying and we haven't keyed this node so far, key it now
	TRACE("can_key=%d ", (int)can_key);
	if( can_key && key && !keys_map[coupling] )
	{
		TRACE("keying... match set %p key ptr %p new value %p, presently %d keys out of %d match sets\n",
				&coupling, &keys_map[coupling], key.get(),
				keys_map.size(), sr->couplings.size() );
		keys_map[coupling] = key;

		return key->root;
	}

	if( keys_map[coupling] )
	{
		// Always substitute
		TRACE("substituting ");
		ASSERT( keys_map[coupling] );
		keys_map[coupling]->replace_pattern = pattern; // Only fill this in while substituting under the node
		SharedPtr<Node> subs = sr->DuplicateSubtree( keys_map[coupling]->root, this, can_key, keys_map[coupling] ); // Enter substitution
		// TODO can_key should be false in the above?
		keys_map[coupling]->replace_pattern = SharedPtr<Node>();
		return subs;
	}

    ASSERT( can_key ); // during substitution pass we should have all match sets keyed
    // In KEYING and this match set not keyed yet (because it will be keyed by another node
    // in the replace pattern). We've got to produce something - don't want to supply the pattern
    // or key without duplication because that breaks rules about using stuff directly, but don't
    // want to call DuplicateSubtree etc because it might recurse endlessly or have other unwanted
    // side-effects. Since this is the KEYING pass the generated tree will get thrown away so
    // just produce a nondescript Node.
    return SharedPtr<Node>();
}

void Conjecture::PrepareForDecidedCompare()
{
	ASSERT( this );

	TRACE("Decision prepare\n");
	decision_index = 0;
}

bool Conjecture::ShouldTryMore( Result r, int threshold )
{
	ASSERT( this );

	if( r == FOUND )
    	return false; // stop trying if we found a match

    if( choices.size() <= threshold ) // we've made all the decisions we can OR
        return false;         // our last decision went out of bounds

    return true;
}


Result Conjecture::Search( SharedPtr<Node> x,
																	 SharedPtr<Node> pattern,
																	 CouplingKeys *keys,
																	 bool can_key,
																	 const RootedSearchReplace *sr )
{
	// Loop through candidate conjectures
	while(1)
	{
		// Try out the current conjecture. This will call HandlDecision() once for each decision;
		// HandleDecision() will return the current choice for that decision, if absent it will
		// add the decision and choose the first choice, if the decision reaches the end it
		// will remove the decision.
		Result r = sr->MatchingDecidedCompare( x, pattern, keys, can_key, *this );

		// If we got a match, we're done. If we didn't, and we've run out of choices, we're done.
		if( r || choices.empty() )
		    return r;
		else
			++(choices.back()); // There are potentially more choices so increment the last decision
	}
}


ContainerInterface::iterator Conjecture::HandleDecision( ContainerInterface::iterator begin,
		                                                                    ContainerInterface::iterator end )
{
	ASSERT( this );
	ASSERT( choices.size() >= decision_index ); // consistency check; as we see more decisions, we should be adding them to the conjecture
	Choice c;

	// See if this decision needs to be added to the present Conjecture
	if( choices.size() == decision_index ) // this decision missing from conjecture?
	{
		c = begin; // Choose the first option supplied
		choices.push_back( c ); // append this decision so we will iterate it later
		TRACE("Decision %d appending begin\n", decision_index );
	}
	else // already know about this decision
	{
		// Adopt the current decision based on Conjecture
	    c = choices[decision_index]; // Get present decision
	}

	// Check the decision obeys bounds
	if( c == end ) // gone off the end?
	{
		// throw away the bad iterator; will force initialisation to begin() next time
		// NOTE: we will still return end in this case, i.e. an invalid iterator. This tells
		// the caller to please not try to do any matching with this decision, but fall out
		// with NOT_FOUND.
		TRACE("Decision %d hit end\n", decision_index );
		choices.resize( decision_index );
	}
	else
	{
		// That decision is OK, so move to the next one
		TRACE("Decision %d OK\n", decision_index );

/* JSG this is slow!
 		bool seen_c=false;
		for( Choice i = begin; i != end; ++i )
		{
			TRACE("%p == %p?\n", (*i).get(), (*c).get() );
			seen_c |= (i==c);
		}
		ASSERT( seen_c )("Decision #%d: c not in x or x.end(), seems to have overshot!!!!", decision_index);
*/
		decision_index++;
	}

	// Return whatever choice we made
    return c;
}


SearchReplace::SearchReplace( SharedPtr<Node> sp,
                              SharedPtr<Node> rp,
                              CouplingSet m,
                              vector<RootedSearchReplace *> s )
{
	Configure( sp, rp, m, s );
}


void SearchReplace::Configure( SharedPtr<Node> sp,
                               SharedPtr<Node> rp,
                               CouplingSet m,
                               vector<RootedSearchReplace *> s )
{
	if( !sp )
		return;

	// Make a non-rooted search and replace (ie where the base of the search pattern
	// does not have to be the root of the whole program tree).
	// Insert a Stuff node as root of the search pattern
	SharedPtr< Stuff<Scope> > search_root( new Stuff<Scope> );
	search_root->terminus = sp;

	if( rp ) // Is there also a replace pattern?
	{
		// Insert a Stuff node as root of the replace pattern
		SharedPtr< Stuff<Scope> > replace_root( new Stuff<Scope> );
	    replace_root->terminus = rp;

	    // Key them together
		Coupling root_match(( search_root, replace_root ));
	    m.insert( root_match );

	    // Configure the rooted implementation with new patterns and match sets
	    RootedSearchReplace::Configure( search_root, replace_root, m, s );
	}
	else
	{
	    // Configure the rooted implementation with new pattern
        RootedSearchReplace::Configure( search_root, rp, m, s );
	}
}

Result TransformToBase::DecidedCompare( const RootedSearchReplace *sr,
		                                                     SharedPtr<Node> x,
		                                                     CouplingKeys *keys,
		                                                     bool can_key,
		                                                     Conjecture &conj ) const
{
    // Transform the candidate expression
    SharedPtr<Node> xt = (*transformation)( sr->GetContext(), x );
	if( xt )
	{
	    // Punt it back into the search/replace engine
	    return sr->DecidedCompare( xt, SharedPtr<Node>(pattern), keys, can_key, conj );
	}
	else
	{
	    // Transformation returned NULL, probably because the candidate was of the wrong
		// type, so just don't match
	    return NOT_FOUND;
	}
}


void RootedSearchReplace::Test()
{
    RootedSearchReplace sr;
    
    {
        // single node with topological wildcarding
        SharedPtr<Void> v(new Void);
        ASSERT( sr.Compare( v, v ) == FOUND );
        SharedPtr<Boolean> b(new Boolean);
        ASSERT( sr.Compare( v, b ) == NOT_FOUND );
        ASSERT( sr.Compare( b, v ) == NOT_FOUND );
        SharedPtr<Type> t(new Type);
        ASSERT( sr.Compare( v, t ) == FOUND );
        ASSERT( sr.Compare( t, v ) == NOT_FOUND );
        ASSERT( sr.Compare( b, t ) == FOUND );
        ASSERT( sr.Compare( t, b ) == NOT_FOUND );
        
        // node points directly to another with TC
        SharedPtr<Pointer> p1(new Pointer);
        p1->destination = v;
        ASSERT( sr.Compare( p1, b ) == NOT_FOUND );
        ASSERT( sr.Compare( p1, p1 ) == FOUND );
        SharedPtr<Pointer> p2(new Pointer);
        p2->destination = b;
        ASSERT( sr.Compare( p1, p2 ) == NOT_FOUND );
        p2->destination = t;
        ASSERT( sr.Compare( p1, p2 ) == FOUND );
        ASSERT( sr.Compare( p2, p1 ) == NOT_FOUND );
    }
    
    {
        // string property
        SharedPtr<SpecificString> s1( new SpecificString("here") );
        SharedPtr<SpecificString> s2( new SpecificString("there") );
        ASSERT( sr.Compare( s1, s1 ) == FOUND );
        ASSERT( sr.Compare( s1, s2 ) == NOT_FOUND );
    }    
    
    {
        // int property
        SharedPtr<SpecificInteger> i1( new SpecificInteger(3) );
        SharedPtr<SpecificInteger> i2( new SpecificInteger(5) );
        TRACE("  %s %s\n", ((llvm::APSInt)*i1).toString(10).c_str(), ((llvm::APSInt)*i2).toString(10).c_str() );
        ASSERT( sr.Compare( i1, i1 ) == FOUND );
        ASSERT( sr.Compare( i1, i2 ) == NOT_FOUND );
    }    
    
    {
        // node with sequence, check lengths 
        SharedPtr<Compound> c1( new Compound );
        ASSERT( sr.Compare( c1, c1 ) == FOUND );
        SharedPtr<Nop> n1( new Nop );
        c1->statements.push_back( n1 );
        ASSERT( sr.Compare( c1, c1 ) == FOUND );
        SharedPtr<Nop> n2( new Nop );
        c1->statements.push_back( n2 );
        ASSERT( sr.Compare( c1, c1 ) == FOUND );
        SharedPtr<Compound> c2( new Compound );
        ASSERT( sr.Compare( c1, c2 ) == NOT_FOUND );
        SharedPtr<Nop> n3( new Nop );
        c2->statements.push_back( n3 );
        ASSERT( sr.Compare( c1, c2 ) == NOT_FOUND );
        SharedPtr<Nop> n4( new Nop );
        c2->statements.push_back( n4 );
        ASSERT( sr.Compare( c1, c2 ) == FOUND );
        SharedPtr<Nop> n5( new Nop );
        c2->statements.push_back( n5 );
        ASSERT( sr.Compare( c1, c2 ) == NOT_FOUND );
    }

    {
        // node with sequence, TW 
        SharedPtr<Compound> c1( new Compound );
        SharedPtr<Nop> n1( new Nop );
        c1->statements.push_back( n1 );
        SharedPtr<Compound> c2( new Compound );
        SharedPtr<Statement> s( new Statement );
        c2->statements.push_back( s );
        ASSERT( sr.Compare( c1, c2 ) == FOUND );
        ASSERT( sr.Compare( c2, c1 ) == NOT_FOUND );
    }
    
    {        
        // topological with extra member in target node
        /* gone obsolete with tree changes TODO un-obsolete
        SharedPtr<Label> l( new Label );
        SharedPtr<Public> p1( new Public );
        l->access = p1;
        SharedPtr<LabelIdentifier> li( new LabelIdentifier );
        li->name = "mylabel";
        l->identifier = li;
        SharedPtr<Declaration> d( new Declaration );
        SharedPtr<Public> p2( new Public );
        d->access = p2;
        ASSERT( sr.Compare( l, d ) == true );
        ASSERT( sr.Compare( d, l ) == false );
        SharedPtr<Private> p3( new Private );
        d->access = p3;
        ASSERT( sr.Compare( l, d ) == false );
        ASSERT( sr.Compare( d, l ) == false );
        SharedPtr<AccessSpec> p4( new AccessSpec );
        d->access = p4;
        ASSERT( sr.Compare( l, d ) == true );
        ASSERT( sr.Compare( d, l ) == false );
        */
    }
}


