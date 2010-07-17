#include "search_replace.hpp"
#include "validate.hpp"
#include "render/graph.hpp" //  for graphing patterns

// Constructor remembers search pattern, replace pattern and any supplied couplings as required
RootedSearchReplace::RootedSearchReplace( TreePtr<Node> sp,
                                          TreePtr<Node> rp,
                                          CouplingSet m,
                                          vector<RootedSearchReplace *> s )
{
	Configure( sp, rp, m, s );
}


void RootedSearchReplace::Configure( TreePtr<Node> sp,
                                     TreePtr<Node> rp,
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

    // If we have a slave, copy its couplings into ours so we have a full set
    // of all the couplings - this will be used across the board. Note that
    // the non-rooted SearchReplace adds a new coupling.
    FOREACH( RootedSearchReplace *slave, slaves )
	{
		for( CouplingSet::iterator msi = slave->couplings.begin();
             msi != slave->couplings.end();
             msi++ )
		    couplings.insert( *msi );
	}

	TRACE("Merged couplings, I have %d\n", couplings.size() );

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
Result RootedSearchReplace::DecidedCompare( TreePtr<Node> x,
		                                             TreePtr<Node> pattern,
		                                             CouplingKeys *keys,
		                                             bool can_key,
		                                             Conjecture &conj ) const
{
	if( !pattern )    // NULL matches anything in search patterns (just to save typing)
		return FOUND;

    if( TreePtr<SoftSearchPattern> ssp = dynamic_pointer_cast<SoftSearchPattern>(pattern) )
    {
    	// Hand over to any soft search functionality in the search pattern node
    	Result r = ssp->DecidedCompare( this, x, keys, can_key, conj );
    	if( r != FOUND )
    		return NOT_FOUND;
    }
    else if( TreePtr<StuffBase> stuff_pattern = dynamic_pointer_cast<StuffBase>(pattern) )
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
			else if( TreePtrInterface *pattern_ptr = dynamic_cast<TreePtrInterface *>(pattern_memb[i]) )
			{
				TreePtrInterface *x_ptr = dynamic_cast<TreePtrInterface *>(x_memb[i]);
				ASSERT( x_ptr )( "itemise for target didn't match itemise for pattern");
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
	// Attempt to match all the elements between start and the end of the sequence; stop
	// if either pattern or subject runs out.
	ContainerInterface::iterator xit = x.begin();
	ContainerInterface::iterator pit = pattern.begin();

	while( pit != pattern.end() )
	{
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
		    	for( ContainerInterface::iterator it=xit_begin_star; it != xit; ++it )
		    		ss->push_back( *it );
				// Apply couplings to this Star and matched range
				if( keys )
		    		if( !keys->KeyAndRestrict( TreePtr<Node>(ss), pe, this, can_key ) )
		        	    return NOT_FOUND;
		    }
	    }
	    else // not a Star so match singly...
	    {
	    	TRACE("Not a star\n");
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
			if( !DecidedCompare( *xit, TreePtr<Node>(*pit), keys, can_key, conj ) )
			    return NOT_FOUND;
	    }
    }

    // Now handle the star if there was one; all the non-star matches have been erased from
    // the collection, leaving only the star matches.

    if( !xremaining->empty() && !seen_star )
    	return NOT_FOUND; // there were elements left over and no star to match them against

    // If we got here, the node matched the search pattern. Now apply couplings
    TRACE("seen_star %d  star %p\n", seen_star, star.get() );
    if( keys && seen_star && star )
        if( !keys->KeyAndRestrict( TreePtr<Node>(xremaining), star, this, can_key ) )
        	return NOT_FOUND;

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
	ASSERT( stuff_pattern->terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");

	// Define a walk, rooted at this node, restricted as specified in search pattern
	WalkContainer wx( x, stuff_pattern->restrictor );

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
	TRACE("Comparing x=%s with pattern=%s, match keys at %p\n", typeid(*x).name(), typeid(*pattern).name(), keys );
	// Create the conjecture object we will use for this compare, and then go
	// into the recursive compare function
	Conjecture conj;
	Result r = conj.Search( x, pattern, keys, can_key, this );
	return r;
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
        else if( TreePtrInterface *source_ptr = dynamic_cast<TreePtrInterface *>(source_memb[i]) )
        {
        	TRACE();
            TreePtrInterface *dest_ptr = dynamic_cast<TreePtrInterface *>(dest_memb[i]);
            ASSERT( dest_ptr )( "itemise for target didn't match itemise for source");
            if( *source_ptr ) // Masked: where source is NULL, do not overwrite
                *dest_ptr = DuplicateSubtree( *source_ptr, keys, can_key, current_key );
            if( !current_key )
            	ASSERT( *dest_ptr )("Found NULL in replace pattern without a coupling to substitute it");
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
		                           shared_ptr<Key> current_key ) const
{
    // For now, always overwrite the dest
    // TODO smarter semantics for prepend, append etc based on NULLs in the sequence)
    dest->clear();

    TRACE("duplicating sequence size %d\n", source->size() );
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
			TRACE("Normal element, inserting directly\n");
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
    // For now, always overwrite the dest
    // TODO smarter semantics for prepend, append etc based on NULLs in the sequence)
    dest->clear();

    TRACE("duplicating collection size %d\n", source->size() );

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
			TRACE("Normal element, inserting directly\n");
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
	TRACE("Duplicating %s under_substitution=%p\n", TypeInfo(source).name().c_str(), current_key.get());

    // Are we substituting a stuff node? If so, see if we reached the terminus, and if
	// so come out of substitution.
	if( shared_ptr<StuffKey> stuff_key = dynamic_pointer_cast<StuffKey>(current_key) )
	{
		ASSERT( stuff_key->replace_pattern );
		TreePtr<StuffBase> replace_stuff = dynamic_pointer_cast<StuffBase>( stuff_key->replace_pattern );
		ASSERT( replace_stuff );
		if( replace_stuff->terminus &&      // There is a terminus in replace pattern
			source == stuff_key->terminus ) // and the present node is the terminus in the source pattern
		{
			TRACE( "Substituting stuff: source=%s:%p, term=%s:%p\n",
					TypeInfo(source).name().c_str(), source.get(),
					TypeInfo(stuff_key->terminus).name().c_str(), stuff_key->terminus.get() );
			TRACE( "Leaving substitution to duplicate terminus replace pattern\n" );
			return DuplicateSubtree( replace_stuff->terminus, keys, can_key, shared_ptr<Key>() ); // not in substitution any more
		}
	}

	TreePtr<Node> dest = TreePtr<Node>();
	dest = keys->KeyAndSubstitute( shared_ptr<Key>(), source, this, can_key );
    ASSERT( !dest || !current_key )("Should only find a match in patterns"); // We'll never find a match when we're under substitution, because the
                                                                             // source is actually a match key already, so not in any couplings
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
		if( TreePtr<SoftReplacePattern> srp = dynamic_pointer_cast<SoftReplacePattern>( source ) )
		{
			// Substitute is an identifier, so preserve its uniqueness by just returning
			// the same node. Don't do any more - we wouldn't want to change the
			// identifier in the tree even if it had members, lol!
			TRACE("Invoke soft replace pattern %s\n", TypeInfo(srp).name().c_str() );
			ASSERT( !current_key )( "Found soft replace pattern while under substitution\n" );
			TreePtr<Node> newsource = srp->DuplicateSubtree( this, keys, can_key );
			ASSERT( newsource );

			// Allow this to key a coupling if required
			TreePtr<Node> subs = keys->KeyAndSubstitute( newsource, source, this, can_key );
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


TreePtr<Node> RootedSearchReplace::MatchingDuplicateSubtree( TreePtr<Node> source,
		                                                        CouplingKeys *keys ) const
{
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
// on supplied patterns and couplings. Does search and replace
// operations repeatedly until there are no more matches. Returns how
// many hits we got.
int RootedSearchReplace::RepeatingSearchReplace( TreePtr<Node> *proot,
	                                             TreePtr<Node> search_pattern,
	                                             TreePtr<Node> replace_pattern,
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

	// Do the search and replace with before and after vaidation
	Validate()( *pcontext, proot );
	(void)RepeatingSearchReplace( proot, search_pattern, replace_pattern );
	Validate()( *pcontext, proot );


    pcontext = NULL; // just to avoid us relying on the context outside of a search+replace pass
}


// Find a coupling containing the supplied node
Coupling CouplingKeys::FindCoupling( TreePtr<Node> node,
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



Result CouplingKeys::KeyAndRestrict( TreePtr<Node> x,
		                                                                    TreePtr<Node> pattern,
		                                                                    const RootedSearchReplace *sr,
		                                                                    bool can_key )
{
	shared_ptr<Key> key( new Key );
	key->root = x;
	return KeyAndRestrict( key, pattern, sr, can_key );
}

Result CouplingKeys::KeyAndRestrict( shared_ptr<Key> key,
		                                                                    TreePtr<Node> pattern,
		                                                                    const RootedSearchReplace *sr,
		                                                                    bool can_key )
{
	ASSERT( this );
	// Find a coupling for this node. If the node is not in a coupling then there's
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
	// with the tree node stored for the coupling. This comparison should not match any couplings
	// (it does not include stuff from any search or replace pattern) so do not allow couplings.
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

TreePtr<Node> CouplingKeys::KeyAndSubstitute( TreePtr<Node> x,
                                                                   TreePtr<Node> pattern,
                                                                   const RootedSearchReplace *sr,
                                                                   bool can_key )
{
	shared_ptr<Key> key( new Key );
	key->root = x;
	return KeyAndSubstitute( key, pattern, sr, can_key );
}

// Note return is NULL in all cases unless we substituted in which case it is the result of the
// substitution, duplicated for our convenience. Always check the return value for NULL.
TreePtr<Node> CouplingKeys::KeyAndSubstitute( shared_ptr<Key> key, // key may be NULL meaning we are not allowed to key the node
		                                                           TreePtr<Node> pattern,
		                                                           const RootedSearchReplace *sr,
		                                                           bool can_key )
{
	ASSERT( this );
	ASSERT( !key || key->root != pattern ); // just a general usage check

	// Find a coupling for this node. If the node is not in a coupling then there's
	// nothing for us to do, so return without restricting the search.
	Coupling coupling = FindCoupling( pattern, sr->couplings );
	if( coupling.empty() )
		return TreePtr<Node>();
	TRACE("MATCH: ");

	// If we're keying and we haven't keyed this node so far, key it now
	TRACE("can_key=%d ", (int)can_key);
	if( can_key && key && !keys_map[coupling] )
	{
		TRACE("keying... coupling %p key ptr %p new value %p, presently %d keys out of %d couplings\n",
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
		TreePtr<Node> subs = sr->DuplicateSubtree( keys_map[coupling]->root, this, can_key, keys_map[coupling] ); // Enter substitution
		// TODO can_key should be false in the above?
		keys_map[coupling]->replace_pattern = TreePtr<Node>();
		return subs;
	}

    ASSERT( can_key ); // during substitution pass we should have all couplings keyed
    // In KEYING and this coupling not keyed yet (because it will be keyed by another node
    // in the replace pattern). We've got to produce something - don't want to supply the pattern
    // or key without duplication because that breaks rules about using stuff directly, but don't
    // want to call DuplicateSubtree etc because it might recurse endlessly or have other unwanted
    // side-effects. Since this is the KEYING pass the generated tree will get thrown away so
    // just produce a nondescript Node.
    return TreePtr<Node>();
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


Result Conjecture::Search( TreePtr<Node> x,
																	 TreePtr<Node> pattern,
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


SearchReplace::SearchReplace( TreePtr<Node> sp,
                              TreePtr<Node> rp,
                              CouplingSet m,
                              vector<RootedSearchReplace *> s )
{
	Configure( sp, rp, m, s );
}


void SearchReplace::Configure( TreePtr<Node> sp,
                               TreePtr<Node> rp,
                               CouplingSet m,
                               vector<RootedSearchReplace *> s )
{
	if( !sp )
		return;

	// Make a non-rooted search and replace (ie where the base of the search pattern
	// does not have to be the root of the whole program tree).
	// Insert a Stuff node as root of the search pattern
	TreePtr< Stuff<Scope> > search_root( new Stuff<Scope> );
	search_root->terminus = sp;

	if( rp ) // Is there also a replace pattern?
	{
		// Insert a Stuff node as root of the replace pattern
		TreePtr< Stuff<Scope> > replace_root( new Stuff<Scope> );
	    replace_root->terminus = rp;

	    // Key them together
		Coupling root_match(( search_root, replace_root ));
	    m.insert( root_match );

	    // Configure the rooted implementation with new patterns and couplings
	    RootedSearchReplace::Configure( search_root, replace_root, m, s );
	}
	else
	{
	    // Configure the rooted implementation with new pattern
        RootedSearchReplace::Configure( search_root, rp, m, s );
	}
}

Result TransformToBase::DecidedCompare( const RootedSearchReplace *sr,
		                                                     TreePtr<Node> x,
		                                                     CouplingKeys *keys,
		                                                     bool can_key,
		                                                     Conjecture &conj ) const
{
    // Transform the candidate expression
    TreePtr<Node> xt = (*transformation)( sr->GetContext(), x );
	if( xt )
	{
	    // Punt it back into the search/replace engine
	    return sr->DecidedCompare( xt, TreePtr<Node>(pattern), keys, can_key, conj );
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


