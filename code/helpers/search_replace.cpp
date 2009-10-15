#include "search_replace.hpp"

// Constructor remembers search pattern, replace pattern and any supplied match sets as required
SearchReplace::SearchReplace( shared_ptr<Node> sp, 
                              shared_ptr<Node> rp,
                              const set<MatchSet> *m )
{
	our_matches = false;
	Configure( sp, rp, m );
    ASSERT( matches );
}

void SearchReplace::Configure( shared_ptr<Node> sp,
                               shared_ptr<Node> rp,
                               const set<MatchSet> *m )
{  
    search_pattern = sp;
    replace_pattern = rp;

    if( our_matches )
        delete matches;
    our_matches = !m;
    if( our_matches )    
        matches = new set<MatchSet>;
    else
    	matches = m;
    
    ASSERT( matches );        
} 


// Destructor tries not to leak memory lol
SearchReplace::~SearchReplace()
{
    if( our_matches )    
        delete matches;
}


// Helper for IsMatchPattern that does the testing for the present node, including 
// superclass wildcarding and data member checking on Property nodes.
bool SearchReplace::IsMatchPatternLocal( shared_ptr<Node> x, shared_ptr<Node> pattern )
{
    TRACE();
    ASSERT( pattern ); // Disallow NULL pattern for now, could change this if required
    ASSERT( x ); // No NULL in tree

    // Is node correct class?
    TRACE("Is %s >= %s? ", TypeInfo(pattern).name().c_str(), TypeInfo(x).name().c_str() );
    if( !(TypeInfo(pattern) >= TypeInfo(x)) ) // Note >= is "non-strict superset" i.e. pattern is superclass of x or same class
    {
        TRACE("lol no!\n" );
        return false;
    }

    if( shared_ptr<SpecificName> pattern_id = dynamic_pointer_cast<SpecificName>(pattern) )
    {
        shared_ptr<SpecificName> x_id = dynamic_pointer_cast<SpecificName>(x);
        ASSERT( x_id );
        if( x_id != pattern_id ) // With identifiers, use the actual node address, not the name,
        	                       // in case different identifiers have the same name
        {
            TRACE("Identifiers differ\n");
            return false;
        }
    }
    if( shared_ptr<SpecificString> pattern_str = dynamic_pointer_cast<SpecificString>(pattern) )
    {
        shared_ptr<SpecificString> x_str = dynamic_pointer_cast<SpecificString>(x);
        ASSERT( x_str );
        if( x_str->value != pattern_str->value )
        {
            TRACE("Strings differ\n");
            return false;
        }
    }    
    else if( shared_ptr<SpecificInteger> pattern_int = dynamic_pointer_cast<SpecificInteger>(pattern) )
    {
        shared_ptr<SpecificInteger> x_int = dynamic_pointer_cast<SpecificInteger>(x);
        ASSERT( x_int );
        TRACE("%s %s\n", x_int->value.toString(10).c_str(), pattern_int->value.toString(10).c_str() );
        if( x_int->value != pattern_int->value )
        {
            TRACE("Integers differ\n");
            return false;
        }
    }    
    else if( shared_ptr<SpecificFloat> pattern_flt = dynamic_pointer_cast<SpecificFloat>(pattern) )
    {
        shared_ptr<SpecificFloat> x_flt = dynamic_pointer_cast<SpecificFloat>(x);
        ASSERT( x_flt );
        if( !x_flt->value.bitwiseIsEqual( pattern_flt->value ) )
        {
            TRACE("Floats differ\n");
            return false;
        }
    }
    else if( shared_ptr<SpecificFloatSemantics> pattern_sem = dynamic_pointer_cast<SpecificFloatSemantics>(pattern) )
    {
        shared_ptr<SpecificFloatSemantics> x_sem = dynamic_pointer_cast<SpecificFloatSemantics>(x);
        ASSERT( x_sem );
        if( x_sem->value != pattern_sem->value )
        {
            TRACE("Float semantics differ\n");
            return false;
        }
    }
    TRACE("yes!!\n");
    return true;
}    


// Helper for IsMatchPattern that does the actual match testing work for the children and recurses.
// Also checks for soft matches.
bool SearchReplace::IsMatchPatternNoKey( shared_ptr<Node> x, shared_ptr<Node> pattern )
{
    ASSERT( !!pattern ); // Disallow NULL pattern for now, could change this if required
    
    // Hand over to any soft search functionality in the search pattern node
    if( shared_ptr<SoftSearchPattern> ssp = dynamic_pointer_cast<SoftSearchPattern>(pattern) )
        return ssp->IsMatchPattern( this, x );

    // Check whether the present node matches
    if( !IsMatchPatternLocal( x, pattern ) )
        return false;
    
    // Recurse through the children. Note that the itemiser internally does a
    // dynamic_cast onto the type of pattern, and itemises over that type. x must
    // be dynamic_castable to pattern's type.
    vector< Itemiser::Element * > pattern_memb = Itemiser::Itemise( pattern.get() ); 
    vector< Itemiser::Element * > x_memb = Itemiser::Itemise( x.get(),           // The thing we're itemising
                                                              pattern.get() );   // Just get the members corresponding to pattern's class
    ASSERT( pattern_memb.size() == x_memb.size() );
    
    for( int i=0; i<pattern_memb.size(); i++ )
    {
        ASSERT( pattern_memb[i] && "itemise returned null element");
        ASSERT( x_memb[i] && "itemise returned null element");
        
        if( GenericSequence *pattern_seq = dynamic_cast<GenericSequence *>(pattern_memb[i]) )                
        {
            GenericSequence *x_seq = dynamic_cast<GenericSequence *>(x_memb[i]);
            ASSERT( x_seq && "itemise for target didn't match itemise for pattern");
            TRACE("Member %d is Sequence, target %d elts, pattern %d elts\n", i, x_seq->size(), pattern_seq->size() );
            if( !IsMatchPattern( *x_seq, *pattern_seq ) )
                return false;
        }
        else if( GenericCollection *pattern_col = dynamic_cast<GenericCollection *>(pattern_memb[i]) )
        {
        	GenericCollection *x_col = dynamic_cast<GenericCollection *>(x_memb[i]);
            ASSERT( x_col && "itemise for target didn't match itemise for pattern");
            TRACE("Member %d is Collection, target %d elts, pattern %d elts\n", i, x_col->size(), pattern_col->size() );
            if( !IsMatchPattern( *x_col, *pattern_col ) )
                return false;
        }            
        else if( GenericSharedPtr *pattern_ptr = dynamic_cast<GenericSharedPtr *>(pattern_memb[i]) )         
        {
            GenericSharedPtr *x_ptr = dynamic_cast<GenericSharedPtr *>(x_memb[i]);
            ASSERT( x_ptr && "itemise for target didn't match itemise for pattern");
            TRACE("Member %d is SharedPtr, pattern ptr=%p\n", i, pattern_ptr->get());
			if( !IsMatchPattern( *x_ptr, *pattern_ptr ) )
				return false;
        }
        else
        {
            ASSERTFAIL("got something from itemise that isnt a Sequence, Collection or a SharedPtr");
        }
    }       
   
    TRACE("Matches search pattern\n");
    return true;
}

// xstart and pstart are the indexes into the sequence where we will begin checking for a match.
// It is assumed that elements before these have already been matched and may be ignored.
bool SearchReplace::IsMatchPattern( GenericSequence &x, GenericSequence &pattern, int xstart, int pstart )
{
	// Attempt to match all the elements between start and the end of the sequence; stop
	// if either pattern or subject runs out.
	int xi=xstart;
	int pi=pstart;
	while( pi < pattern.size() )
	{
		TRACE("pattern at %d; x at %d\n", pi, xi );
		// Get the next element of the pattern
		shared_ptr<Node> pe( pattern[pi] );
		pi++;
	    if( !pe || dynamic_pointer_cast<StarBase>(pe) )
	    {
	    	TRACE("Star (pe is %d)\n", (int)!!pe);
			// We have a Star type wildcard that can match multiple elements. At present,
			// NULL is interpreted as a Star (but cannot go in a match set). Remember where
	    	// we are - this is the beginning of the subsequence that potentially matches the Star.
	    	int xi_begin_star = xi;

	    	// Star always matches at the end of a sequence, so we only bother checking when there
	    	// are more elements left
	    	if( pi == pattern.size() )
	    	{
	    		xi = x.size(); // match all remaining members of x; jump to end
	    	}
	    	else
	    	{
	    		TRACE("Pattern continues after star\n");
	    		// Star not at end so there is more stuff to match; ensure not another star
	    		ASSERT( !dynamic_pointer_cast<StarBase>(shared_ptr<Node>(pattern[pi])) )
	    		      ( "Not allowed to have two neighbouring Star elements in search pattern Sequence");

	    		// Try out different numbers of elements to match the Star, counting up from zero
				while( xi < x.size() )
				{
					// Recursively attempt to match the remaining elements *after* the present star.
					if( IsMatchPattern( x, pattern, xi, pi ) )
					{
						break;
					}
					xi++;
				}
				// We got to the end of the subject Sequence, there's no way to match the >0 elements
				// we know are still in the pattern.
				if( xi == x.size() )
				{
					TRACE("Ran out of candidate\n");
					return false;
				}
	    	}

			// Star matched [xi_begin_star, xi) i.e. xi-xi_begin_star elements
		    // Now make a copy of the elements that matched the star and apply match sets
		    if( pe )
		    {
		    	TRACE("Copying matched star for match set\n");
		    	// Copy the matched subsequence into a SubSequence node for the benefit of match sets
			    shared_ptr<SubSequence> xcopy( new SubSequence );
				for( int i=xi_begin_star; i<xi; i++ )
					xcopy->push_back( x[i] );

				// Apply match sets to this Star and matched SubSequence
		    	if( !UpdateAndCheckMatchSets( xcopy, pe ) )
		    	{
		    		TRACE("Mat set disallows because keyed already and key differs\n");
		        	return false;
		    	}
		    }
	    }
	    else // not a Star so match singly...
	    {
	    	TRACE("Not a star\n");
			// If there is one more element in x, see if it matches the pattern
			//shared_ptr<Node> xe( x[xi] );
			if( xi < x.size() && IsMatchPattern( x[xi], pe ) )
			{
				xi++;
			}
			else
			{
				TRACE("Element mismatched\n");
				return false;
			}
	    }
	}

    // If we finished the job and pattern and subject are still aligned, then it was a match
	TRACE("Finished; pattern got to %d out of %d; x got to %d out of %x\n", xi, x.size(), pi, pattern.size() );
    return xi==x.size() && pi==pattern.size();
}

bool SearchReplace::IsMatchPattern( GenericCollection &x, GenericCollection &pattern )
{
    // We'll need a copy since we'll be erasing elements
    shared_ptr<SubCollection> xcopy( new SubCollection );
    FOREACH( const GenericSharedPtr &xe, x )
        xcopy->insert( xe );

    shared_ptr<StarBase> star;
    bool seen_star = false;

    FOREACH( const GenericSharedPtr &gpe, pattern )
    {
    	shared_ptr<Node> pe( gpe );
        if( !pe || dynamic_pointer_cast<StarBase>(pe) ) // NULL in pattern collection?
        {
        	ASSERT(!seen_star)("Only one Star node (or NULL ptr) allowed in a search pattern Collection");
            star = dynamic_pointer_cast<StarBase>(pe); // remember for later and skip to next pattern
            seen_star = true;
        }
	    else // not a Star so match singly...
	    {
			// Look for a single element of x that matches the present element of the pattern
			bool found = false;
			FOREACH( const GenericSharedPtr &xe, *xcopy )
			{
				if( IsMatchPattern( xe, pe ) )
				{
					found = true;
					xcopy->erase( xe );
					break;
				}
			}
			if( !found )
				return false; // present pattern element had no match
	    }
    }

    // Now handle the star if there was one; all the non-star matches have been erased from
    // the collectionleaving only the star matches.

    if( !xcopy->empty() && !seen_star )
    	return false; // there were elements left over and no star to match them against

    // If we got here, the node matched the search pattern. Now apply match sets
    if( seen_star && star )
        if( !UpdateAndCheckMatchSets( xcopy, star ) )
        	return false;

	return true;
}

// Try to match a pattern with the inferno rules: soft patterns allowed to
// determine own match result, match sets restrict to same actual node. Also
// keys the match sets as matches are found.
bool SearchReplace::IsMatchPattern( shared_ptr<Node> x, shared_ptr<Node> pattern )
{
	if( !pattern )    // NULL matches anything in search patterns (just to save typing)
		return true;
    
    // Does the search pattern match?
    if( !IsMatchPatternNoKey( x, pattern ) )
        return false;
    
    // If we got here, the node matched the search pattern. Now apply match sets
    if( !UpdateAndCheckMatchSets( x, pattern ) )
    	return false;
    
    return true;
}


// Search supplied program for a match to the configured search pattern.
// If found, return double pointer to assist replace algorithm.
bool SearchReplace::Search( shared_ptr<Node> program, GenericContainer::iterator &it )
{
    Walk w( program );
    while(!w.Done())
    {
        it = w.GetIterator(); // get an iterator for current position in tree, so we can change it                    
        ClearKeys();
        if( IsMatchPattern( *it, search_pattern ) )
            return true;
        w.AdvanceInto(); 
    }    
    
    return false;
}


// Clear all pointer members in supplied dest to NULL
void SearchReplace::ClearPtrs( shared_ptr<Node> dest )
{
    vector< Itemiser::Element * > dest_memb = Itemiser::Itemise( dest.get() );
    for( int i=0; i<dest_memb.size(); i++ )
    {
        if( GenericSequence *dest_seq = dynamic_cast<GenericSequence *>(dest_memb[i]) )                
        {
//            FOREACH( GenericSharedPtr &p, *dest_seq )
        	for( GenericContainer::iterator i=dest_seq->begin(); i!=dest_seq->end(); ++i )
            {
                SharedPtr<Node> p;
                i.Overwrite( &p ); // TODO using Overwrite() to support unordered - but does this fuction even make sense forn unordered?
            }
        }            
        else if( GenericSharedPtr *dest_ptr = dynamic_cast<GenericSharedPtr *>(dest_memb[i]) )         
        {
            *dest_ptr = shared_ptr<Node>();
        }
    }       
}


// Helper for DuplicateSubtree, fills in children of dest node from source node when source node child
// is non-NULL. This means we can call this multiple times with different sources and get a priority 
// scheme.
void SearchReplace::OverlayPtrs( shared_ptr<Node> dest, shared_ptr<Node> source, bool under_substitution )
{
    ASSERT( TypeInfo(source) >= TypeInfo(dest) )("source must be a non-strict subclass of destination, so that it does not have more members");

    // Itemise the members. Note that the itemiser internally does a
    // dynamic_cast onto the type of source, and itemises over that type. dest must
    // be dynamic_castable to source's type.
    vector< Itemiser::Element * > source_memb = Itemiser::Itemise( source.get() ); 
    vector< Itemiser::Element * > dest_memb = Itemiser::Itemise( dest.get(),       // The thing we're itemising
                                                                 source.get() );   // Just get the members corresponding to source's class
    ASSERT( source_memb.size() == dest_memb.size() );

    // Loop over all the members of source (which can be a subset of dest)
    // and for non-NULL members, duplicate them by recursing and write the
    // duplicates to the destination.
    for( int i=0; i<dest_memb.size(); i++ )
    {
        ASSERT( source_memb[i] && "itemise returned null element" );
        ASSERT( dest_memb[i] && "itemise returned null element" );
        
        if( GenericSequence *source_seq = dynamic_cast<GenericSequence *>(source_memb[i]) )                
        {
            GenericSequence *dest_seq = dynamic_cast<GenericSequence *>(dest_memb[i]);
            ASSERT( dest_seq && "itemise for dest didn't match itemise for source");
            DuplicateSequence( dest_seq, source_seq, under_substitution );
        }            
        else if( GenericCollection *source_col = dynamic_cast<GenericCollection *>(source_memb[i]) )
        {
        	GenericCollection *dest_col = dynamic_cast<GenericCollection *>(dest_memb[i]);
            ASSERT( dest_col && "itemise for dest didn't match itemise for source");
            DuplicateCollection( dest_col, source_col, under_substitution );
         }
        else if( GenericSharedPtr *source_ptr = dynamic_cast<GenericSharedPtr *>(source_memb[i]) )         
        {
            GenericSharedPtr *dest_ptr = dynamic_cast<GenericSharedPtr *>(dest_memb[i]);
            ASSERT( dest_ptr && "itemise for target didn't match itemise for source");

            TRACE("overlaying shared_ptr, %d\n", (int)!!source_ptr );
            if( *source_ptr ) // Masked: where source is NULL, do not overwrite
                *dest_ptr = DuplicateSubtree( *source_ptr, under_substitution );
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }        
}

void SearchReplace::DuplicateSequence( GenericSequence *dest, GenericSequence *source, bool under_substitution )
{
    // For now, always overwrite the dest
    // TODO smarter semantics for prepend, append etc based on NULLs in the sequence)
    dest->clear();

    TRACE("duplicating sequence size %d\n", source->size() );
	FOREACH( const GenericSharedPtr &p, *source )
	{
		ASSERT( p ); // present simplified scheme disallows NULL, see above
		shared_ptr<Node> pp( p );
		if( dynamic_pointer_cast<StarBase>(pp) )
		{
			// Seen a Star wildcard in replace pattern. It must be keyed to something, and that
			// thing must be a SubSequence. Find it then expand the emements one by one directly
			// into the destination Sequence.
			const MatchSet *match = FindMatchSet( pp );
			ASSERT( match )( "Star in replace pattern must be in a match set");
			ASSERT( match->key_x )( "match set did not get keyed successfully");
			shared_ptr<Node> n = DuplicateSubtree( match->key_x, true );
			shared_ptr<SubSequence> ss = dynamic_pointer_cast<SubSequence>(n);
			ASSERT( ss )( "Star keyed to wrong thing, expected SubSequence");
			TRACE("star seen; inserting subsequence length %d\n", ss->size() );
			FOREACH( const GenericSharedPtr &xx, *ss )
				dest->push_back( xx );
		}
		else
		{
			TRACE("non-star element, inserting directly\n");
			ASSERT( p ); // present simplified scheme disallows NULL, see above
			shared_ptr<Node> n = DuplicateSubtree( p, under_substitution );
			dest->push_back( n );
		}
	}
}

void SearchReplace::DuplicateCollection( GenericCollection *dest, GenericCollection *source, bool under_substitution )
{
    // For now, always overwrite the dest
    // TODO smarter semantics for prepend, append etc based on NULLs in the sequence)
    dest->clear();

    TRACE("duplicating collection size %d\n", source->size() );

	FOREACH( const GenericSharedPtr &p, *source )
	{
		ASSERT( p ); // present simplified scheme disallows NULL, see above
		shared_ptr<Node> pp( p );
		if( dynamic_pointer_cast<StarBase>(pp) )
		{
			// Seen a Star wildcard in replace pattern. It must be keyed to something, and that
			// thing must be a SubCollection. Find it then expand the emements one by one directly
			// into the destination Collection.
            const MatchSet *match = FindMatchSet( pp );
			ASSERT( match )( "Star in replace pattern must be keyed for substitution");
			ASSERT( match->key_x )( "match set did not get keyed successfully");
			shared_ptr<Node> n = DuplicateSubtree( match->key_x, true );
			shared_ptr<SubCollection> sc = dynamic_pointer_cast<SubCollection>(n);
			ASSERT( sc )( "Star keyed to wrong thing, expected SubCollection");
			TRACE("star seen; inserting subcollection length %d\n", sc->size() );
			FOREACH( const GenericSharedPtr &xx, *sc )
				dest->insert( xx );
		}
		else
		{
			TRACE("non-star element, inserting directly\n");
			ASSERT( p ); // present simplified scheme disallows NULL, see above
			SharedPtr<Node> n = DuplicateSubtree( p, under_substitution );
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
shared_ptr<Node> SearchReplace::DuplicateSubtree( shared_ptr<Node> source, bool under_substitution )
{
	TRACE("Duplicating, us=%d\n", (int)under_substitution);
	// Do not duplicate identifiers if they are being substituted from the original tree.
	if( under_substitution && dynamic_pointer_cast<Identifier>( source ) )
	{
		// Substitute is an identifier, so preserve its uniqueness by just returning
		// the same node. Don't do any more - we wouldn't want to change the
		// identifier in the tree even if it had members, lol!
		TRACE("Not duplicating identifiers when under substitution\n");
		return source;
	}

	shared_ptr<Node> dest;

    // Check match sets for a match to the source of our duplication. If one is found, we
	// substitute the match set's key node instead of the supplied source node.
    const MatchSet *match = FindMatchSet( source );

    if( match )
    {
    	TRACE("substituting because found in match set\n");
        // It's in a match set, so substitute the key. Simplest to recurse for this. We will
    	// still overlay any non-NULL members of the source pattern node onto the result (see below)
        ASSERT( match->key_x )("Match set in replace pattern but did not key to search pattern");
        ASSERT( TypeInfo(source) >= TypeInfo(match->key_x) )
              ("source must be a non-strict superclass of local_substitute, so that it does not have more members (match set probably not all the same types)");
              //TODO simply require that every member of a match set has the exact same type
        dest = DuplicateSubtree( match->key_x, true );
    }
    else
    {
    	TRACE("duplicating node\n");
		// Make the new node (destination node)
		shared_ptr<Cloner> dup_dest = Cloner::Clone( source );
		dest = dynamic_pointer_cast<Node>( dup_dest );
		ASSERT(dest);

		// Make all members in the destination be NULL
		ClearPtrs( dest );
    }
    
    // Copy the source over, except for any NULLs in the source. If source is superclass
    // of destination (i.e. has possibly fewer members) the missing ones will be left alone.
    ASSERT( dest );
    OverlayPtrs( dest, source, under_substitution );

    return dest;
}

#include "render/graph.hpp"
// Perform the configured replacement at the supplied target.
// Note target is a double pointer, since we wish to enact the
// replacement by changing a SharedPtr somewhere.
void SearchReplace::Replace( GenericContainer::iterator target )
{
    ASSERT( replace_pattern );
    SharedPtr<Node> nn( DuplicateSubtree( replace_pattern ) );
    target.Overwrite( &nn );
    TRACE("*target=%p nn=%p\n", target->get(), nn.get() );
}


// Perform search and replace on supplied program based
// on current patterns and match sets. Does search and replace
// operations repeatedly until there are no more matches.
void SearchReplace::operator()( shared_ptr<Program> p )
{
    program = p;
    GenericContainer::iterator it;
    while(1)
    {
        bool found = Search( program, it );        
        if( found )
        {
        	TRACE("Search successful, now replacing\n");
            Replace( it );
        }
        else
            break;
    }
    program = shared_ptr<Program>();
}


// Find a match set containing the supplied node
const SearchReplace::MatchSet *SearchReplace::FindMatchSet( shared_ptr<Node> node )
{
    for( set<MatchSet>::iterator msi = matches->begin();
         msi != matches->end();
         msi++ )
    {
        MatchSet::iterator ni = msi->find( node );
        if( ni != msi->end() )
            return &*msi;
    }
    return 0;
}


// Reset the keys in all the matchsets 
void SearchReplace::ClearKeys()
{
    for( set<MatchSet>::iterator msi = matches->begin();
         msi != matches->end();
         msi++ )
    {
        msi->key_x = shared_ptr<Node>();
        msi->key_pattern = shared_ptr<Node>();
    }
}


// During search, once two nodes are known to match, use this function to
// 1. Key it into a match set of required and
// 2. Detect whether a match set required two parts of the search tree to match and
// reject if they don't.
bool SearchReplace::UpdateAndCheckMatchSets( shared_ptr<Node> x, shared_ptr<Node> pattern )
{
	const MatchSet *m = FindMatchSet( pattern );
	if( m )
	{
		// It's in a match set!!
		if( !(m->key_pattern) || (m->key_pattern == pattern) )
		{
			// Not keyed yet OR seeing the same pattern node we already keyed to (ie
			// we are repeating part of the search), so key it now!!!
			m->key_pattern = pattern;
			m->key_x = x;
		}
		else
		{
			// This match set has already been keyed! This means there is more than one
			// entry in the match set pointing to the search pattern. We will require
			// that they match each other for the search as a whole to match.
			// In this call both pattern and subject are in the input program tree, and
			// match sets must not point to the input program, so we won't hit a match set.
			if( !IsMatchPattern( x, m->key_x ) )
				return false;
		}
	}
	return true;
}


void SearchReplace::Test()
{
    SearchReplace sr;
    
    {
        // single node with topological wildcarding
        shared_ptr<Void> v(new Void);
        ASSERT( sr.IsMatchPattern( v, v ) == true );
        shared_ptr<Boolean> b(new Boolean);
        ASSERT( sr.IsMatchPattern( v, b ) == false );
        ASSERT( sr.IsMatchPattern( b, v ) == false );
        shared_ptr<Type> t(new Type);
        ASSERT( sr.IsMatchPattern( v, t ) == true );
        ASSERT( sr.IsMatchPattern( t, v ) == false );
        ASSERT( sr.IsMatchPattern( b, t ) == true );
        ASSERT( sr.IsMatchPattern( t, b ) == false );
        
        // node points directly to another with TC
        shared_ptr<Pointer> p1(new Pointer);
        p1->destination = v;
        ASSERT( sr.IsMatchPattern( p1, b ) == false );
        ASSERT( sr.IsMatchPattern( p1, p1 ) == true );
        shared_ptr<Pointer> p2(new Pointer);
        p2->destination = b;
        ASSERT( sr.IsMatchPattern( p1, p2 ) == false );
        p2->destination = t;
        ASSERT( sr.IsMatchPattern( p1, p2 ) == true );
        ASSERT( sr.IsMatchPattern( p2, p1 ) == false );
    }
    
    {
        // string property
        shared_ptr<SpecificString> s1( new SpecificString );
        shared_ptr<SpecificString> s2( new SpecificString );
        s1->value = "here";
        s2->value = "there";
        ASSERT( sr.IsMatchPattern( s1, s1 ) == true );
        ASSERT( sr.IsMatchPattern( s1, s2 ) == false );        
    }    
    
    {
        // int property
        llvm::APSInt apsint( 32, true );
        shared_ptr<SpecificInteger> i1( new SpecificInteger );
        shared_ptr<SpecificInteger> i2( new SpecificInteger );
        apsint = 3;
        i1->value = apsint;
        apsint = 5;
        i2->value = apsint;
        TRACE("  %s %s\n", i1->value.toString(10).c_str(), i2->value.toString(10).c_str() );
        ASSERT( sr.IsMatchPattern( i1, i1 ) == true );
        ASSERT( sr.IsMatchPattern( i1, i2 ) == false );        
    }    
    
    {
        // node with sequence, check lengths 
        shared_ptr<Compound> c1( new Compound );
        ASSERT( sr.IsMatchPattern( c1, c1 ) == true );
        shared_ptr<Nop> n1( new Nop );
        c1->statements.push_back( n1 );
        ASSERT( sr.IsMatchPattern( c1, c1 ) == true );
        shared_ptr<Nop> n2( new Nop );
        c1->statements.push_back( n2 );
        ASSERT( sr.IsMatchPattern( c1, c1 ) == true );
        shared_ptr<Compound> c2( new Compound );
        ASSERT( sr.IsMatchPattern( c1, c2 ) == false );
        shared_ptr<Nop> n3( new Nop );
        c2->statements.push_back( n3 );
        ASSERT( sr.IsMatchPattern( c1, c2 ) == false );
        shared_ptr<Nop> n4( new Nop );
        c2->statements.push_back( n4 );
        ASSERT( sr.IsMatchPattern( c1, c2 ) == true );        
        shared_ptr<Nop> n5( new Nop );
        c2->statements.push_back( n5 );
        ASSERT( sr.IsMatchPattern( c1, c2 ) == false );        
    }

    {
        // node with sequence, TW 
        shared_ptr<Compound> c1( new Compound );
        shared_ptr<Nop> n1( new Nop );
        c1->statements.push_back( n1 );
        shared_ptr<Compound> c2( new Compound );
        shared_ptr<Statement> s( new Statement );
        c2->statements.push_back( s );
        ASSERT( sr.IsMatchPattern( c1, c2 ) == true );
        ASSERT( sr.IsMatchPattern( c2, c1 ) == false );
    }
    
    {        
        // topological with extra member in target node
        /* gone obsolete with tree changes TODO un-obsolete
        shared_ptr<Label> l( new Label );
        shared_ptr<Public> p1( new Public );
        l->access = p1;
        shared_ptr<LabelIdentifier> li( new LabelIdentifier );
        li->name = "mylabel";
        l->identifier = li;
        shared_ptr<Declaration> d( new Declaration );
        shared_ptr<Public> p2( new Public );
        d->access = p2;
        ASSERT( sr.IsMatchPattern( l, d ) == true );
        ASSERT( sr.IsMatchPattern( d, l ) == false );
        shared_ptr<Private> p3( new Private );
        d->access = p3;
        ASSERT( sr.IsMatchPattern( l, d ) == false );
        ASSERT( sr.IsMatchPattern( d, l ) == false );
        shared_ptr<AccessSpec> p4( new AccessSpec );
        d->access = p4;
        ASSERT( sr.IsMatchPattern( l, d ) == true );
        ASSERT( sr.IsMatchPattern( d, l ) == false );
        */
    }
}
