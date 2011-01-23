#include "search_replace.hpp"
#include "validate.hpp"
#include "render/graph.hpp" //  for graphing patterns
#include "conjecture.hpp"

// Master constructor remembers search pattern, replace pattern and any supplied couplings as required
CompareReplace::CompareReplace( TreePtr<Node> cp,
                                TreePtr<Node> rp,
                                bool im ) :
    is_master( im ),                                                 
    compare_pattern( NULL ),
    replace_pattern( NULL ),
    coupling_keys( new CouplingKeys )   
{
    if( cp )
        Configure( cp, rp );
}    
    
void CompareReplace::Configure( TreePtr<Node> cp,
                                TreePtr<Node> rp )
{
    ASSERT( cp );
    Validate v(true);        
    v(cp, &cp);
    v(rp, &rp);            
    
    if( is_master )
    {
        // Count the number of times we hit each node during a walk.
        // Where we hit a node more than once, add a coupling for it.
        // This will detect nodes with multiple refs (which we want 
        // couplings for) and all nodes thereunder (which I'm not sure
        // if we want couplgs for or even if should be allowed).
        // TODO decide.
        TRACE("doing inferred couplings, search pattern ")(*cp)("\n");
        
        // We will store counts of links into nodes here
        Map< TreePtr<Node>, int > ms;
        
        // Make a walker that uniquifies as a recursion filter. So each link
        // only gets followed once. Ergo, each node appears once for every
        // incoming link. We want to include search pattern and replace pattern so 
        // apply the same filter object to both walks.
        UniqueFilter uf;
        Expand wsp( cp, NULL, &uf ), wrp( rp, NULL, &uf );
        
        Expand::iterator i;
        FOREACH( TreePtr<Node> n, wsp )
        {
            TRACE("Got %p\n", n.get());
            if( n )
            {
                if( dynamic_pointer_cast<OverlayBase>(n) || dynamic_pointer_cast<StuffBase>(n) )
                    ms[n] = 2; // foce couplings at overlay nodes TODO refactor substitution so this is not needed
                else if( ms.IsExist( n ) )
                    ms[n]++;
                else
                    ms[n] = 1;
            }
        }
        TRACE("doing inferred couplings, replace pattern\n");
        FOREACH( TreePtr<Node> n, wrp )
        {
            TRACE("Got %p\n", n.get());
            if( n )
            {
                if( ms.IsExist( n ) )
                    ms[n]++;
                else
                    ms[n] = 1;
            }
        }
        TRACE("inserting inferred couplings\n");
        typedef	pair<TreePtr<Node>, int> pair;	
        FOREACH( pair pp, ms )
        {
            if( pp.second > 1 )
                if( !couplings.IsExist( pp.first ) )
                {
                    TRACE("Inserting coupling for ")(*(pp.first))(" count is %d\n", pp.second );
                    couplings.insert( pp.first );	
                }
        }
       
        // Fill in fields on the stuff nodes
        Traverse tsp( cp ); 
        FOREACH( TreePtr<Node> n, tsp )
        {        
            if(n)TRACE("Elaborating ")(*n)("\n");
            if( TreePtr<StuffBase> sb = dynamic_pointer_cast<StuffBase>(n) )
            {
                TRACE("Elaborating Stuff@%p, rr@%p\n", sb.get(), sb->recurse_restriction.get());
                sb->recurse_comparer.couplings = couplings; 
                sb->recurse_comparer.coupling_keys = coupling_keys; 
                sb->recurse_comparer.compare_pattern = sb->recurse_restriction; // TODO could move into a Stuff node constructor if there was one
            }
        }
        Traverse trp( rp ); 
        FOREACH( TreePtr<Node> n, trp )
        {        
            if(n)TRACE("Elaborating ")(*n)("\n");
            if( TreePtr<StuffBase> sb = dynamic_pointer_cast<StuffBase>(n) )
            {
                TRACE("Elaborating Stuff@%p, rr@%p\n", sb.get(), sb->recurse_restriction.get());
                sb->recurse_comparer.couplings = couplings; 
                sb->recurse_comparer.coupling_keys = coupling_keys; 
                sb->recurse_comparer.compare_pattern = sb->recurse_restriction; // TODO could move into a Stuff node constructor if there was one
            }
        }
        
        // Look for slaves. If we find them, copy our couplings into their couplings
        // Do not just overwrite since there may be implicit Stuff node couplings.
        ParentTraverse ss( rp );
        FOREACH( TreePtr<Node> n, ss )
        {
            if( TreePtr<SlaveBase> rsb = dynamic_pointer_cast<SlaveBase>(n) )
            {
                rsb->MergeCouplings( couplings, coupling_keys ); 
            }
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
Result CompareReplace::DecidedCompare( TreePtr<Node> x,
									   TreePtr<Node> pattern,
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
    	Result r = ssp->DecidedCompare( this, x, can_key, conj );
    	if( r != FOUND )
    		return NOT_FOUND;
    }
    else if( TreePtr<StuffBase> stuff_pattern = dynamic_pointer_cast<StuffBase>(pattern) )
    {
    	// Invoke stuff node compare
    	// Check whether the present node matches
    	bool r = DecidedCompare( x, stuff_pattern, can_key, conj );
        if( r != FOUND )
            return NOT_FOUND;
    }
    else if( TreePtr<GreenGrassBase> green_pattern = dynamic_pointer_cast<GreenGrassBase>(pattern) )
    {
        // Restrict so that everything in the input program under here must be "green grass"
        // ie unmodified by previous replaces in this RepeatingSearchReplace() run.
        Expand w( x );
        FOREACH( TreePtr<Node> p, w )
            if( dirty_grass.find( p ) != dirty_grass.end() )
                return NOT_FOUND;
        // Normal matching for the through path
        bool r = DecidedCompare( x, green_pattern->GetThrough(), can_key, conj );
        if( r != FOUND )
            return NOT_FOUND;
    }
    else if( TreePtr<OverlayBase> op = dynamic_pointer_cast<OverlayBase>(pattern) )
    {
        // When DoOverlay node seen duriung search, just forward through the "base" path
        bool r = DecidedCompare( x, op->GetThrough(), can_key, conj );
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

			if( r != FOUND )
				return NOT_FOUND;
		}
    }
   
    // If we got here, the node matched the search pattern. Now apply couplings
    return coupling_keys->KeyAndRestrict( x, pattern, this, can_key );

    return FOUND;
}


// xstart and pstart are the indexes into the sequence where we will begin checking for a match.
// It is assumed that elements before these have already been matched and may be ignored.
Result CompareReplace::DecidedCompare( SequenceInterface &x,
		                               SequenceInterface &pattern,
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
      	  	    if( !coupling_keys->KeyAndRestrict( TreePtr<Node>(ss), pe, this, can_key ) )
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
			if( xit != x.end() && DecidedCompare( *xit, pe, can_key, conj ) == FOUND )
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


Result CompareReplace::DecidedCompare( CollectionInterface &x,
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
			if( !DecidedCompare( *xit, TreePtr<Node>(*pit), can_key, conj ) )
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
    if( seen_star && star )
        if( !coupling_keys->KeyAndRestrict( TreePtr<Node>(xremaining), star, this, can_key ) )
        	return NOT_FOUND;
    TRACE("matched\n");
	return FOUND;
}


// Helper for DecidedCompare that does the actual match testing work for the children and recurses.
// Also checks for soft matches.
Result CompareReplace::DecidedCompare( TreePtr<Node> x,
									   TreePtr<StuffBase> stuff_pattern,
									   bool can_key,
									   Conjecture &conj ) const
{
    INDENT;
	ASSERT( stuff_pattern->terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");

	// Define a walk, rooted at this node, restricted as specified in search pattern
    Filter *rf = NULL;
	if( stuff_pattern->recurse_restriction )
    {
        TRACE("Comparing ")(*stuff_pattern)("@%p, rr@%p\n", stuff_pattern.get(), stuff_pattern->recurse_restriction.get());
        ASSERT( stuff_pattern->recurse_comparer.compare_pattern );     
        rf = &(stuff_pattern->recurse_comparer);
    }
    
    Expand wx( x, NULL, rf );        

	// Get decision from conjecture about where we are in the walk
	ContainerInterface::iterator thistime = conj.HandleDecision( wx.begin(), wx.end() );
	if( thistime == (ContainerInterface::iterator)(wx.end()) )
		return NOT_FOUND; // ran out of choices

	// Try out comparison at this position
	Result r = DecidedCompare( *thistime, stuff_pattern->terminus, can_key, conj );

    // If we got this far, do the couplings
    if( r )
    {
    	shared_ptr<StuffKey> key( new StuffKey );
    	key->root = x;
    	key->terminus = *thistime;
    	//TreePtr<Node> = TreePtrInterface
        r = coupling_keys->KeyAndRestrict( shared_ptr<Key>(key),
                                          stuff_pattern,
                                          this,
                                          can_key );
    }
	return r;
}


Result CompareReplace::MatchingDecidedCompare( TreePtr<Node> x,
		                                       TreePtr<Node> pattern,
		                                       bool can_key,
		                                       Conjecture &conj ) const
{
    INDENT;
    Result r;
    // TODO might not need to sake keys here because do it in RepeatingSerachReplace
    CouplingKeys original_match_keys = *coupling_keys; // deep copy here

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
        if( r != FOUND )
        {
            *coupling_keys = original_match_keys; // revert match keys since we failed
            return NOT_FOUND;                  // Save time by giving up if no match found
        }
    }
    
    // Now restrict the search according to the couplings
    TRACE("doing RESTRICTING pass....\n");
    conj.PrepareForDecidedCompare();
    r = DecidedCompare( x, pattern, false, conj );
    TRACE("RESTRICTING pass result %d\n", r );
    if( r != FOUND )
    {
        *coupling_keys = original_match_keys; // revert match keys since we failed
        return NOT_FOUND;	               // Save time by giving up if no match found
    }

    // Do not revert match keys if we were successful - keep them for replace
    // and any slave search and replace operations we might do.
    return FOUND;
}


Result CompareReplace::Compare( TreePtr<Node> x,
		                        TreePtr<Node> pattern,
								bool can_key ) const
{
    INDENT;
    ASSERT( x );
    ASSERT( pattern );
	TRACE("Comparing x=")(*x);
    TRACE(" pattern=")(*pattern);
    TRACE(" can_key=%d context=", (int)can_key);
    TRACE(**pcontext)(" @%p\n", pcontext);
	// Create the conjecture object we will use for this compare, and then go
	// into the recursive compare function
	Conjecture conj;
	return conj.Search( x, pattern, can_key, this );
}


bool CompareReplace::IsMatch( TreePtr<Node> context,       
                              TreePtr<Node> root )
{
    pcontext = &context;
    ASSERT( compare_pattern );
    Result r = Compare( root, compare_pattern, false );
    pcontext = NULL;
    return r == FOUND;
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


// Helper for DuplicateSubtree, fills in children of dest node from source node when source node child
// is non-NULL. This means we can call this multiple times with different sources and get a priority 
// scheme.
void CompareReplace::DoOverlay( TreePtr<Node> dest,
		                        TreePtr<Node> source,
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
#if 0
    TRACE("DoOverlay dest={");
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
            DoOverlay( dest_seq, source_seq, can_key, current_key );
        }            
        else if( CollectionInterface *source_col = dynamic_cast<CollectionInterface *>(source_memb[i]) )
        {
        	CollectionInterface *dest_col = dynamic_cast<CollectionInterface *>(dest_memb[i]);
            ASSERT( dest_col )( "itemise for dest didn't match itemise for source");
            DoOverlay( dest_col, source_col, can_key, current_key );
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
                source_child = DuplicateSubtree( source_child, can_key, current_key );
            
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
                     DoOverlay( dest_child, source_child, can_key, current_key );
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


void CompareReplace::DoOverlay( SequenceInterface *dest,
		                        SequenceInterface *source,
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
		TreePtr<Node> n = DuplicateSubtree( p, can_key, current_key );
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


void CompareReplace::DoOverlay( CollectionInterface *dest,
   		                        CollectionInterface *source,
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
		TreePtr<Node> n = DuplicateSubtree( p, can_key, current_key );
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
TreePtr<Node> CompareReplace::DuplicateSubtree( TreePtr<Node> source,
		                                        bool can_key,
		                                        shared_ptr<Key> current_key ) const
{
	INDENT;
 	TRACE("Duplicating %s under_substitution=%p\n", ((string)*source).c_str(), current_key.get());
#if 0
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
    // Under substitution, we should be duplicating a subtree of the input
    // program, which should not contain any special nodes
    ASSERT( !(dynamic_pointer_cast<SpecialBase>(source) && current_key) )
          ("Duplicating special node ")
          (*source)
          (" while under substitution\n" );

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
			TreePtr<Node> term = DuplicateSubtree( replace_stuff->terminus, can_key, shared_ptr<Key>() ); // not in substitution any more
			TRACE( "Returning to substitution for rest of stuff\n" );
			return term;
		}
	}
	
    // See if the source node is coupled to anything
    TreePtr<Node> dest = coupling_keys->KeyAndSubstitute( TreePtr<Node>(), source, this, can_key );
    ASSERT( !(dest && current_key) )("Should only find a match in patterns"); // We'll never find a match when we're under substitution, because the

    if( TreePtr<SlaveBase> sb = dynamic_pointer_cast<SlaveBase>(source) )
    {
        dest = DuplicateSubtree( sb->GetThrough(), can_key, current_key );
        if(!can_key) // do not run slaves until we have all the keys of the master 
        {
            (*sb)( *pcontext, &dest );
        }
    } 
	else if( shared_ptr<SoftReplacePattern> srp = dynamic_pointer_cast<SoftReplacePattern>( source ) )
    {
        if( !dest )
        {
            // Call the soft pattern impl (TODO don't like calling the function DuplicateSubtree, confusing)
            dest = srp->DuplicateSubtree( this, can_key );
            // Allow this to key a coupling 
            (void)coupling_keys->KeyAndSubstitute( dest, source, this, can_key );
        }
    }
    else if( shared_ptr<OverlayBase> ob = dynamic_pointer_cast<OverlayBase>( source ) )
    {
        source = ob->GetOverlay();
        if( !dest || !source->IsLocalMatch(ob->GetThrough().get()) )
        {
            // Base not coupled or is not compatible for overlaying, so just duplicate the overlay leg.
            dest = DuplicateSubtree( source, can_key, current_key );
        }
        else
        {
            // Base coupled and can be overlayed so overlay. 
            // TODO what if ob->overlay is another soft node? Should we not always DuplicateSubtree()?
            ASSERT( !dynamic_pointer_cast<SpecialBase>( source ) );
            DoOverlay( dest, source, can_key, current_key );
        }
    }
    else if( dynamic_pointer_cast<SpecialBase>(source) )
    {
        ASSERT( dest )
              ("Special nodes in replace pattern not coupled and does not produce any output (")(*source)(")\n");
    }
    else
    {
        if( !dest )
        {
            // No coupling to key to, so just make a copy
            TRACE("Copying source ")(*source)("\n");
            
            // Make the new node (destination node)
            shared_ptr<Cloner> dup_dest = source->Duplicate(source);
            dest = dynamic_pointer_cast<Node>( dup_dest );
            ASSERT(dest);
     
            // Replace all the links in the new node with duplicates of the links in the old node.
            // To do this we re-use the DoOverlay function (no point writing 2 very similar functions)
            ClearPtrs( dest ); 
        }
        DoOverlay( dest, source, can_key, current_key );

        // If not substituting a Stuff node, remember this node is dirty for GreenGrass restriction
        // Also dirty the dest if the source was dirty when we are substituting Stuff
        if( !current_key || !dynamic_pointer_cast<StuffKey>(current_key) || dirty_grass.find( source ) != dirty_grass.end() )
            dirty_grass.insert( dest );
    }        
    
    return dest;
}


TreePtr<Node> CompareReplace::MatchingDuplicateSubtree( TreePtr<Node> source ) const
{
    //TODO can_key should be an enum eg RelpacePass {KEYING, SUBSTITUTING} for clarity
    // Do a two-pass matching process: first get the keys...
    TRACE("doing replace KEYING pass....\n");
    (void)DuplicateSubtree( source, true );
    TRACE("replace KEYING pass\n" );

    // Now restrict the search according to the couplings
    TRACE("doing replace SUBSTITUTING pass....\n");
    TreePtr<Node> r = DuplicateSubtree( source, false );
    TRACE("replace SUBSTITUTING pass\n" );
    return r;
}


Result CompareReplace::SingleCompareReplace( TreePtr<Node> *proot ) 
{
	TRACE("%p Begin search\n", this);
    
    // Explicitly preserve the coupling keys structure - we do this instead
    // of clearing the keys in case the keys were set up in advance, as will
    // be the case if this is a slave.
    CouplingKeys preserved_keys = *coupling_keys;
    
	Result r = Compare( *proot, compare_pattern, true );
	if( r != FOUND )
		return NOT_FOUND;

    if( r == FOUND && replace_pattern )
    {
    	TRACE("%p Search successful, now replacing\n", this);
        *proot = MatchingDuplicateSubtree( replace_pattern );
    }

    *coupling_keys = preserved_keys;

    return r;
}


// Perform search and replace on supplied program based
// on supplied patterns and couplings. Does search and replace
// operations repeatedly until there are no more matches. Returns how
// many hits we got.
int CompareReplace::RepeatingCompareReplace( TreePtr<Node> *proot )
{
	dirty_grass.clear();

    int i=0;
    while(i<20) // TODO!!
    {
    	Result r = SingleCompareReplace( proot );
    	TRACE("%p result %d", this, r);        
    	if( r != FOUND )
            break;
       // Validate()( *pcontext, proot );
       	//ASSERT(i<100)("Too many hits"); JSG validate can fail due to a missing decl, but maybe master puts it in...?
        i++;
    }

    TRACE("%p exiting", this);
    return i;
}


// Do a search and replace based on patterns stored in our members
void CompareReplace::operator()( TreePtr<Node> c, TreePtr<Node> *proot )
{
    ASSERT( compare_pattern )("CompareReplace (or SearchReplace) object was not configured before invocation.\n"
                              "Either call Configure() or supply pattern arguments to constructor.\n"
                              "Thank you for taking the time to read this message.\n");
    
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

	if( is_master )
        Validate()( *pcontext, proot ); 
        
        
    (void)RepeatingCompareReplace( proot );   

    if( is_master )
        Validate()( *pcontext, proot ); 

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
    TreePtr< Stuff<Scope> > stuff( new Stuff<Scope> );

    if( rp ) // Is there a replace pattern?
    {
        // Insert a Stuff node as root of the replace pattern
        TreePtr< Overlay<Node> > overlay( new Overlay<Node> );
        stuff->terminus = overlay;
        overlay->through = sp;
        overlay->overlay = rp;

        // Configure the rooted implementation (CompareReplace) with new patterns and couplings
        CompareReplace::Configure( stuff, stuff );
    }
    else
    {
        stuff->terminus = sp;
        CompareReplace::Configure( stuff );
    }
}


void SearchReplace::GetGraphInfo( vector<string> *labels, 
                                  vector< TreePtr<Node> > *links ) const
{
    // Find the original patterns
    TreePtr< Stuff<Scope> > stuff = dynamic_pointer_cast< Stuff<Scope> >(compare_pattern);
    ASSERT( stuff );
    TreePtr< Overlay<Node> > overlay = dynamic_pointer_cast< Overlay<Node> >(stuff->terminus);
    ASSERT( overlay );
        
    labels->push_back("search");    
    links->push_back(overlay->through);
    labels->push_back("replace");
    links->push_back(overlay->overlay);
}
    
    
void CompareReplace::Test()
{
    CompareReplace sr = CompareReplace( MakeTreePtr<Nop>() );

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
