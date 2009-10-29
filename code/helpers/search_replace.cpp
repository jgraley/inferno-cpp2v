#include "search_replace.hpp"

/*
 * Crazy onion searching algorithm
 *
 * The search code as of typing this comment is buggy - if there are two containers A and B
 * in the search pattern, where a is reached first during a walk, and an element a' of A is tied
 * to an element b' of B by a match set, then the search algorithm will fail to detect
 * certain matches.
 *
 * Problem arises because the matching algorithm will try the first candidate for a'
 * and key the match set to it. It will correctly retry different elements of B until
 * it finds a matching b', but if there is none, it will not go back and try another a'.
 *
 * Consequently, a match where a' has to be a later element of A because of the extra
 * constraint of the match set, will never be found.
 *
 * The solution requires some redesigning, and this comment tries to describe the planned
 * new design. It is somewhat onion-like in that it has many layers. The following rubbish
 * ascii-art shows the structure. Each box is a group of functions which may recurse among
 * themselves, and can call functions in contained boxes but not out to surrounding boxes.
 * Thus the overall function call hierarchy goes from outside to in. I've tried to draw
 * the important data flows: arrows pointing in are call arguments and arrows pointing out are
 * return values (or "out" arguments via refs or pointers).
 *
 *  +----------------------------------------------------------------------------------------------------------+
 *  | Top layer runs Search, when successful it replaces                                                       |
 *  | then repeats, if not we're all done                                                                      |
 *  |                                                                                                          |
 *  |  +--------------------------------------------------------------------------------------^-------------+  |
 *  |  | Search layer, walks program tree calling Compare     Starting node                   |             |  |
 *  |  |  +---------------------------------------------------- | --------------------------- ^ ---------+  |  |
 *  |  |  | Compare layer tries different choices for the       |                             |          |  |  |
 *  |  |  | decisions, recurses through the stack of decisions  |         Choice increments   |          |  |  |
 *  |  |  | and calls DecidedCompare for each combination       |                   |         |          |  |  |
 *  |  |  |  +------------------------------------------------- | ----------- ^ --- | ------- ^ ------+  |  |  |
 *  |  |  |  | DecidedCompare calls MatchlessDecidedCompare     |             |     |         |       |  |  |  |
 *  |  |  |  | and then restricts according to the match set    |             |     |      >(And)     |  |  |  |
 *  |  |  |  |  +---------------------------------------------- v --- ^ ----- | --- v ------- ^ ---+  |  |  |  |
 *  |  |  |  |  | MatchlessDecidedCompare layer recurses              |       |               |    |  |  |  |  |
 *  |  |  |  |  | the search pattern checking for matches.       Match set   New decisions    |    |  |  |  |  |
 *  |  |  |  |  | Decisions are made according to choices        mappings                  >(And)  |  |  |  |  |
 *  |  |  |  |  | passed in from Compare layer. Any choices        |       Current node       |    |  |  |  |  |
 *  |  |  |  |  | not present in the supplied decision stack       |             |            |    |  |  |  |  |
 *  |  |  |  |  | are defaulted to the first option and appended   |  +----------v------------^-+  |  |  |  |  |
 *  |  |  |  |  | to the vector. This layer also remembers the     |  | LocalCompare   Is match?|  |  |  |  |  |
 *  |  |  |  |  | mapping of nodes in the search pattern to        |  | compares just           |  |  |  |  |  |
 *  |  |  |  |  | corresponding nodes in the program tree and      |  | a single node           |  |  |  |  |  |
 *  |  |  |  |  | returns this mapping to DeicdedCompare.          |  +-------------------------+  |  |  |  |  |
 *  |  |  |  |  +------------------------------------------------- | ------------------------------+  |  |  |  |
 *  |  |  |  +---------------------------------------------------- | ---------------------------------+  |  |  |
 *  |  |  +------------------------------------------------------- | ------------------------------------+  |  |
 *  |  +---------------------------------------------------------- | ---------------------------------------+  |
 *  |                                                              |                                           |
 *  |  +---------------------------------------------------------- v ---------------------------------------+  |
 *  |  | Replace recurses through replace pattern                                                           |  |
 *  |  +----------------------------------------------------------------------------------------------------+  |
 *  |                                                                                                          |
 *  +----------------------------------------------------------------------------------------------------------+
 *
 * Top and Search layers are as one would expect, searching, replacing and lazily restarting the search after
 * each success.
 *
 * The Compare layer represents a generic subtree compare function, supporting all the special features of
 * inferno search and replace like match sets, wildcards etc. Match set associations are returned for the
 * benefit of the replace. This layer fills in all the choices and calls the DecidedCompare layer.
 *
 * DecidedCompare layer simply invokes the DecidedCompare layer using the search pattern
 * and then checks the match set mappings for mismatches. This requires further calls to
 * DecidedCompare, but these calls are comparing tree to tree so there will be no decisions.
 *
 * DecidedCompare performs a conventional recursive comparison, using the supplied choices.
 *
 * Each layer appears stateless to the layer that calls it. That is to say, the functions are idempotent.
 * For this reason we do not need to segregate them into separate classes. They will all be "const" members
 * of the RootedSearchReplace class and will not modify class members (like the search pattern and match sets).
 *
 * Match sets are as documented elsewhere. Instead of "hidden" mutable key elements, there will be a backing
 * map linking tree nodes to search pattern nodes in the match set. This will be an output of
 * DecidedCompare and will be used by DecidedCompare and the Replace algorithm.
 *
 * A decision is a construct in the search pattern whereby there are (potentially) multiple choices
 * for how to map the search pattern to the tree.
 * They include:
 * 1. the index (or iterator) of the first Sequence element to appear after a star
 *    (equivalent to the size of the star's subsequence)
 * 2. the iterator for any element in a collection except the last one, or any if a star is present
 *    (if no star, there is still a choice of relative ordering)
 * 3. the position of the root of the search/replace patterns in the program tree
 * 4. the walking done by a recursive star (or "stuff") node
 *
 * Of these, 3 may be ignored since this decision is independent of match sets and may be done at a higher level, as
 * shown in the diagram. 4. relates to an unimplemented feature, I will not worry about this at the moment. This
 * leaves 1 and 2 as the decisions to consider. Each may be represented using a GenericIterator and constrained
 * by a "begin" (inclusive) and an "end" (exclusive). Beginning and end may be a subrange of the container's
 * full range.
 *
 * As we walk the search pattern, we will always encounter the same decisions in the same order (at least
 * until we stop due to mismatch). Consequently they may be stored in an ordered container like a vector
 * or a stack and each choice will always take the same index. Such a structure may be passed as an input
 * and an output by Compare, through DecidedCompare and to/from DecidedCompare. By simply adopting
 * the convention that a non-existent or uninitialised choice is equivalent to choosing "begin" we can allow
 * DecidedCompare to lazily fill in actual "begin" iterators.
 *
 * The begin and end constraints may be restricted in a way that depends on earlier choices. A choice
 * that is out of range is considered a mismatch. Each run of DecidedCompare should return the
 * number of decisions that it reached (and hence applied the choices) before stopping due to mismatch. The
 * Compare layer may then choose not to bother exploring the decisions that did not get exercised.
 *
 * It is expected that the Compare layer will actually implement a loop construct to loop through the
 * legal choices of a particular decision. The first call (from the Search layer) would begin iterating
 * through the first decision, and the function would recurse to itself in order to loop through the other
 * decisions, but only when the DecidedCompare layer actually got that far through before stopping due to
 * mismatch.
 *
 * This technique of a loop containing a self-recursion should hopefully avoid the need to state-out the
 * decision exploring algorithm.
 *
 * Note that separating the match set checking is not strictly required, and may not happen in an
 * initial first-strike implementation. It does however make eg "Not" soft-nodes easier to implement
 * and makes things generally tidier. It may  be slower, but it's not clear how much slower for
 * practical search patterns.
 *
 * Note: from looking at STL docs, it seems like std::vector is the container to use for the decision
 * stacks, since we want some mixture of stack-like and array-like access.
 *
 * Note: Conjecture will be the word for a vector of choices as described here.
 */

// Constructor remembers search pattern, replace pattern and any supplied match sets as required
RootedSearchReplace::RootedSearchReplace( shared_ptr<Node> sp,
                                          shared_ptr<Node> rp,
                                          set<MatchSet *> m )
{
	Configure( sp, rp, m );
}


void RootedSearchReplace::Configure( shared_ptr<Node> sp,
                                     shared_ptr<Node> rp,
                                     set<MatchSet *> m )
{
    search_pattern = sp;
    replace_pattern = rp;
    matches = MatchKeys( m ); // Convert from set< MAtchSet *> to subclass MatchKeys
} 


// Destructor tries not to leak memory lol
RootedSearchReplace::~RootedSearchReplace()
{
}


// Helper for DecidedCompare that does the testing for the present node, including
// superclass wildcarding and data member checking on Property nodes.
bool RootedSearchReplace::LocalCompare( shared_ptr<Node> x, shared_ptr<Node> pattern ) const
{
    TRACE();
    ASSERT( pattern ); // Disallow NULL pattern for now, could change this if required
    ASSERT( x )("Found NULL in tree"); // No NULL in tree

    // Is node correct class?
    TRACE("Is %s >= %s? ", TypeInfo(pattern).name().c_str(), TypeInfo(x).name().c_str() );
    if( !(TypeInfo(pattern) >= TypeInfo(x)) ) // Note >= is "non-strict superset" i.e. pattern is superclass of x or same class
    {
        TRACE("lol no!\n" );
        return false;
    }

    if( shared_ptr<SpecificIdentifier> pattern_id = dynamic_pointer_cast<SpecificIdentifier>(pattern) )
    {
      shared_ptr<SpecificIdentifier> x_id = dynamic_pointer_cast<SpecificIdentifier>(x);
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


// Helper for DecidedCompare that does the actual match testing work for the children and recurses.
// Also checks for soft matches.
RootedSearchReplace::Result RootedSearchReplace::DecidedCompare( shared_ptr<Node> x,
		                                             shared_ptr<Node> pattern,
		                                             MatchKeys *match_keys,
		                                             Conjecture &conj,
		                      		                 unsigned context_flags ) const
{
	if( !pattern )    // NULL matches anything in search patterns (just to save typing)
		return FOUND;

    if( shared_ptr<SoftSearchPattern> ssp = dynamic_pointer_cast<SoftSearchPattern>(pattern) )
    {
    	// Hand over to any soft search functionality in the search pattern node
    	Result r = ssp->DecidedCompare( this, x, match_keys, conj, context_flags );
    	if( r != FOUND )
    		return NOT_FOUND;
    }
    else if( shared_ptr<StuffBase> stuff_pattern = dynamic_pointer_cast<StuffBase>(pattern) )
    {
    	// Invoke stuff node compare
    	return DecidedCompare( x, stuff_pattern, match_keys, conj, context_flags );
    }
    else
    {
    	// Not a soft node, so handle explicitly
		// Check whether the present node matches
		if( !LocalCompare( x, pattern ) )
			return NOT_FOUND;

		// Recurse through the children. Note that the itemiser internally does a
		// dynamic_cast onto the type of pattern, and itemises over that type. x must
		// be dynamic_castable to pattern's type.
		vector< Itemiser::Element * > pattern_memb = Itemiser::Itemise( pattern.get() );
		vector< Itemiser::Element * > x_memb = Itemiser::Itemise( x.get(),           // The thing we're itemising
																  pattern.get() );   // Just get the members corresponding to pattern's class
		ASSERT( pattern_memb.size() == x_memb.size() );
		for( int i=0; i<pattern_memb.size(); i++ )
		{
			Result r;
			ASSERT( pattern_memb[i] && "itemise returned null element");
			ASSERT( x_memb[i] && "itemise returned null element");

			if( GenericSequence *pattern_seq = dynamic_cast<GenericSequence *>(pattern_memb[i]) )
			{
				GenericSequence *x_seq = dynamic_cast<GenericSequence *>(x_memb[i]);
				ASSERT( x_seq && "itemise for target didn't match itemise for pattern");
				TRACE("Member %d is Sequence, target %d elts, pattern %d elts\n", i, x_seq->size(), pattern_seq->size() );
				r = DecidedCompare( *x_seq, *pattern_seq, match_keys, conj, context_flags );
			}
			else if( GenericCollection *pattern_col = dynamic_cast<GenericCollection *>(pattern_memb[i]) )
			{
				GenericCollection *x_col = dynamic_cast<GenericCollection *>(x_memb[i]);
				ASSERT( x_col && "itemise for target didn't match itemise for pattern");
				TRACE("Member %d is Collection, target %d elts, pattern %d elts\n", i, x_col->size(), pattern_col->size() );
				r = DecidedCompare( *x_col, *pattern_col, match_keys, conj, context_flags );
			}
			else if( GenericSharedPtr *pattern_ptr = dynamic_cast<GenericSharedPtr *>(pattern_memb[i]) )
			{
				GenericSharedPtr *x_ptr = dynamic_cast<GenericSharedPtr *>(x_memb[i]);
				ASSERT( x_ptr && "itemise for target didn't match itemise for pattern");
				TRACE("Member %d is SharedPtr, pattern ptr=%p\n", i, pattern_ptr->get());
				r = DecidedCompare( *x_ptr, *pattern_ptr, match_keys, conj, context_flags );
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
    if( match_keys )
        return match_keys->KeyAndRestrict( x, pattern, this, context_flags );

    return FOUND;
}


// xstart and pstart are the indexes into the sequence where we will begin checking for a match.
// It is assumed that elements before these have already been matched and may be ignored.
RootedSearchReplace::Result RootedSearchReplace::DecidedCompare( GenericSequence &x,
		                                             GenericSequence &pattern,
		                                             MatchKeys *match_keys,
		                                             Conjecture &conj,
		                      		                 unsigned context_flags ) const
{
	// Attempt to match all the elements between start and the end of the sequence; stop
	// if either pattern or subject runs out.
	GenericContainer::iterator xit = x.begin();
	GenericContainer::iterator pit = pattern.begin();

	while( pit != pattern.end() )
	{
		// Get the next element of the pattern
		shared_ptr<Node> pe( *pit );
		++pit;
	    if( !pe || dynamic_pointer_cast<StarBase>(pe) )
	    {
	    	TRACE("Star (pe is %d)\n", (int)!!pe);
			// We have a Star type wildcard that can match multiple elements. At present,
			// NULL is interpreted as a Star (but cannot go in a match set).

	    	// Remember where we are - this is the beginning of the subsequence that
	    	// potentially matches the Star.
	    	GenericContainer::iterator xit_begin_star = xit;

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
	    		ASSERT( !dynamic_pointer_cast<StarBase>(shared_ptr<Node>(*pit)) )
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
		    	TRACE("%p\n", (*xit).GetNodePtr().get() );
            }

			// Star matched [xit_begin_star, xit) i.e. xit-xit_begin_star elements
		    // Now make a copy of the elements that matched the star and apply match sets
		    if( pe )
		    {
		    	shared_ptr<SubSequence> ss( new SubSequence);
		    	for( GenericContainer::iterator it=xit_begin_star; it != xit; ++it )
		    		ss->push_back( *it );
				// Apply match sets to this Star and matched range
				if( match_keys )
		    		if( !match_keys->KeyAndRestrict( ss, pe, this, context_flags ) )
		        	    return NOT_FOUND;
		    }
	    }
	    else // not a Star so match singly...
	    {
	    	TRACE("Not a star\n");
			// If there is one more element in x, see if it matches the pattern
			//shared_ptr<Node> xe( x[xit] );
			if( xit != x.end() && DecidedCompare( *xit, pe, match_keys, conj, context_flags ) == FOUND )
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


RootedSearchReplace::Result RootedSearchReplace::DecidedCompare( GenericCollection &x,
		                                             GenericCollection &pattern,
		                                             MatchKeys *match_keys,
		                                             Conjecture &conj,
		                      		                 unsigned context_flags ) const
{
    // Make a copy of the elements in the tree. As we go though the pattern, we'll erase them from
	// here so that (a) we can tell which ones we've done so far and (b) we can get the remainder
	// after decisions.
	// TODO is there some stl algorithm for this?
    shared_ptr<SubCollection> xremaining( new SubCollection );
    FOREACH( const GenericSharedPtr &xe, x )
        xremaining->insert( xe );

    shared_ptr<StarBase> star;
    bool seen_star = false;

    for( GenericCollection::iterator pit = pattern.begin(); pit != pattern.end(); ++pit )
    {
    	TRACE("Collection compare %d remain out of %d; looking at %s in pattern\n",
    			xremaining->size(),
    			pattern.size(),
    			TypeInfo( SharedPtr<Node>(*pit) ).name().c_str() );
    	shared_ptr<StarBase> maybe_star = dynamic_pointer_cast<StarBase>( shared_ptr<Node>(*pit) );

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
	    	GenericContainer::iterator xit = conj.HandleDecision( x.begin(), x.end() );
			if( xit == x.end() )
				return NOT_FOUND;

	    	// Remove the chosen element from the remaineder collection. If it is not there (ret val==0)
	    	// then the present chosen iterator has been chosen before and the choices are conflicting.
	    	// We'll just return NOT_FOUND so we do not stop trying further choices (xit+1 may be legal).
	    	if( xremaining->erase( *xit ) == 0 )
	    		return NOT_FOUND;

	    	// Recurse into comparison function for the chosen node
			if( !DecidedCompare( *xit, *pit, match_keys, conj, context_flags ) )
			    return NOT_FOUND;
	    }
    }

    // Now handle the star if there was one; all the non-star matches have been erased from
    // the collection, leaving only the star matches.

    if( !xremaining->empty() && !seen_star )
    	return NOT_FOUND; // there were elements left over and no star to match them against

    // If we got here, the node matched the search pattern. Now apply match sets
    TRACE("seen_star %d  star %p\n", seen_star, star.get() );
    if( match_keys && seen_star && star )
        if( !match_keys->KeyAndRestrict( xremaining, star, this, context_flags ) )
        	return NOT_FOUND;

	return FOUND;
}


// Helper for DecidedCompare that does the actual match testing work for the children and recurses.
// Also checks for soft matches.
RootedSearchReplace::Result RootedSearchReplace::DecidedCompare( shared_ptr<Node> x,
		                                             shared_ptr<StuffBase> stuff_pattern,
		                                             MatchKeys *match_keys,
		                                             Conjecture &conj,
		                      		                 unsigned context_flags ) const
{
	ASSERT( stuff_pattern->terminus )("Stuff node without terminus, seems pointless, if there's a reason for it remove this assert");

	GenericCountingIterator begin(0), end(0);
	{ // just count the nodes seen during the walk, to get an "end" iterator
		Walk w( x );
		while(!w.Done())
		{
			shared_ptr<Node> element = *(w.GetIterator());
			if( !stuff_pattern->restrictor || TypeInfo(element) <= TypeInfo(stuff_pattern->restrictor) )
				w.AdvanceInto();
			else
				w.AdvanceOver();
			++end;
		}
	}
	TRACE("Stuff at %p; counting %d nodes underneath\n", stuff_pattern.get(), end.GetCount());

    // Get decision from conjecture
	GenericContainer::iterator thistime = conj.HandleDecision( begin, end );
	TRACE("Conjecture asks for number %d\n", thistime.GetCount());
	if( thistime == (GenericContainer::iterator)end )
		return NOT_FOUND; // ran out of choices

    // Walk that many places into the subtree
	Walk w( x );
	GenericContainer::iterator cur = begin;
	while( !(cur == thistime) )
	{
		ASSERT( !w.Done() );
		shared_ptr<Node> element = *(w.GetIterator());
		if( !stuff_pattern->restrictor || TypeInfo(element) <= TypeInfo(stuff_pattern->restrictor) )
			w.AdvanceInto();
		else
			w.AdvanceOver();
		++cur;
	}
	ASSERT( cur==thistime );

	// Try out comparison at this position
	ASSERT( !(cur == (GenericContainer::iterator)end) );
	shared_ptr<Node> term_candidate = *(w.GetIterator()); // get an iterator for current position in tree, so we can change it
	Result r = DecidedCompare( term_candidate, stuff_pattern->terminus, match_keys, conj, context_flags );
	TRACE("Result was %d\n", r);

    // If we got this far, do the match sets
    if( match_keys && r )
    {
    	shared_ptr<StuffKey> key( new StuffKey );
    	key->root = x;
    	key->terminus = term_candidate;
        r = match_keys->KeyAndRestrict( key,
        		                        stuff_pattern,
        		                        this,
        		                        context_flags );
    }
	return r;
}


RootedSearchReplace::Result RootedSearchReplace::MatchingDecidedCompare( shared_ptr<Node> x,
		                                                     shared_ptr<Node> pattern,
		                                                     MatchKeys *match_keys,
		                                                     Conjecture &conj ) const
{
    static const unsigned DEFAULT_CONTEXT_FLAGS = 0;
    if( match_keys )
    {
        // Clear down the keys in preperation for a new match set run
    	match_keys->ClearKeys();

    	// Do a two-pass matching process: first get the keys...
       	TRACE("doing KEYING pass....\n");
        match_keys->SetPass( MatchKeys::KEYING );
        Result r = DecidedCompare( x, pattern, match_keys, conj, DEFAULT_CONTEXT_FLAGS );
        TRACE("KEYING pass result %d\n", r );
	    if( r != FOUND )
	    	return r;	    // Save time by giving up if no match found

	    // Now restrict the search according to the match sets
    	TRACE("doing RESTRICTING pass....\n");
        match_keys->SetPass( MatchKeys::RESTRICTING );
        r = DecidedCompare( x, pattern, match_keys, conj, DEFAULT_CONTEXT_FLAGS );
        TRACE("RESTRICTING pass result %d\n", r );
        return r;
    }
    else
    {
    	// No match set, so just call straight through this layer
    	return DecidedCompare( x, pattern, NULL, conj, DEFAULT_CONTEXT_FLAGS );
    }
}


RootedSearchReplace::Result RootedSearchReplace::Compare( shared_ptr<Node> x,
		                                      shared_ptr<Node> pattern,
		                                      MatchKeys *match_keys,
		                                      Conjecture &conj,
		                                      int threshold ) const
{
	// Do a compare with the current conjecture.
	conj.PrepareForDecidedCompare();
	Result r = MatchingDecidedCompare( x, pattern, match_keys, conj );

	// Try different choices for the decisions at the current level. Recurse
	// so that other decisions may be modified.
	while( conj.ShouldTryMore( r, threshold ) )
	{
		r = Compare( x, pattern, match_keys, conj, threshold+1 );
		if( conj.ShouldTryMore( r, threshold ) )
			++conj[threshold];
	}
	return r;
}


RootedSearchReplace::Result RootedSearchReplace::Compare( shared_ptr<Node> x,
		                                      shared_ptr<Node> pattern,
		                                      MatchKeys *match_keys ) const
{
	TRACE("Comparing x=%s with pattern=%s, match keys at %p\n", typeid(*x).name(), typeid(*pattern).name(), match_keys );
	// Create the conjecture object we will use for this compare, and then go
	// into the recursive compare function
	Conjecture conj;
	Result r = Compare( x, pattern, match_keys, conj, 0 );
	return r;
}


// Clear all pointer members in supplied dest to NULL
void RootedSearchReplace::ClearPtrs( shared_ptr<Node> dest ) const
{
    vector< Itemiser::Element * > dest_memb = Itemiser::Itemise( dest.get() );
    for( int i=0; i<dest_memb.size(); i++ )
    {
        if( GenericSequence *dest_seq = dynamic_cast<GenericSequence *>(dest_memb[i]) )                
        {
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
void RootedSearchReplace::Overlay( shared_ptr<Node> dest,
		                           shared_ptr<Node> source,
		                           MatchKeys *match_keys,
		                           shared_ptr<Key> current_key ) const
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
    	TRACE("Overlaying member %d\n", i );
        ASSERT( source_memb[i] && "itemise returned null element" );
        ASSERT( dest_memb[i] && "itemise returned null element" );
        
        if( GenericSequence *source_seq = dynamic_cast<GenericSequence *>(source_memb[i]) )                
        {
            GenericSequence *dest_seq = dynamic_cast<GenericSequence *>(dest_memb[i]);
            ASSERT( dest_seq && "itemise for dest didn't match itemise for source");
            Overlay( dest_seq, source_seq, match_keys, current_key );
        }            
        else if( GenericCollection *source_col = dynamic_cast<GenericCollection *>(source_memb[i]) )
        {
        	GenericCollection *dest_col = dynamic_cast<GenericCollection *>(dest_memb[i]);
            ASSERT( dest_col && "itemise for dest didn't match itemise for source");
            Overlay( dest_col, source_col, match_keys, current_key );
        }
        else if( GenericSharedPtr *source_ptr = dynamic_cast<GenericSharedPtr *>(source_memb[i]) )         
        {
            GenericSharedPtr *dest_ptr = dynamic_cast<GenericSharedPtr *>(dest_memb[i]);
            ASSERT( dest_ptr && "itemise for target didn't match itemise for source");
            if( *source_ptr ) // Masked: where source is NULL, do not overwrite
                *dest_ptr = DuplicateSubtree( *source_ptr, match_keys, current_key );
            if( !current_key )
            	ASSERT( *dest_ptr )("Found NULL in replace pattern without a match set to substitute it");
        }
        else
        {
            ASSERTFAIL("got something from itemise that isn't a sequence or a shared pointer");
        }
    }        
}


void RootedSearchReplace::Overlay( GenericSequence *dest,
		                     GenericSequence *source,
		                     MatchKeys *match_keys,
		                     shared_ptr<Key> current_key ) const
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
			// thing must be a SubSequence. Find it then expand the elements one by one directly
			// into the destination Sequence.
			ASSERT( match_keys );
			const MatchSet *match = match_keys->FindMatchSet( pp );
			ASSERT( match )( "Star in replace pattern must be in a match set");
			shared_ptr<Node> n = DuplicateSubtree( match->key->root, match_keys, match->key );
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
			shared_ptr<Node> n = DuplicateSubtree( p, match_keys, current_key );
			dest->push_back( n );
		}
	}
}


void RootedSearchReplace::Overlay( GenericCollection *dest,
		                     GenericCollection *source,
		                     MatchKeys *match_keys,
		                     shared_ptr<Key> current_key ) const
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
			// thing must be a SubCollection. Find it then expand the elements one by one directly
			// into the destination Collection.
			ASSERT( match_keys );
            const MatchSet *match = match_keys->FindMatchSet( pp );
			ASSERT( match )( "Star in replace pattern must be keyed for substitution");
			shared_ptr<Node> n = DuplicateSubtree( match->key->root, match_keys, match->key );
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
			shared_ptr<Node> n = DuplicateSubtree( p, match_keys, current_key );
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
shared_ptr<Node> RootedSearchReplace::DuplicateSubtree( shared_ptr<Node> source,
		                                                MatchKeys *match_keys,
		                                                shared_ptr<Key> current_key ) const
{
	TRACE("Duplicating, under_substitution=%p\n", current_key.get());

    // Are we substituting a stuff node?
	if( shared_ptr<StuffKey> stuff_key = dynamic_pointer_cast<StuffKey>(current_key) )
	{
		shared_ptr<StuffBase> replace_stuff = stuff_key->replace_stuff;
		ASSERT( replace_stuff );
		if( replace_stuff->terminus &&      // There is a terminus in replace pattern
			source == stuff_key->terminus ) // and the present node is the terminus in the source pattern
		{
			TRACE( "Substituting stuff: source=%s:%p, term=%s:%p\n",
					TypeInfo(source).name().c_str(), source.get(),
					TypeInfo(stuff_key->terminus).name().c_str(), stuff_key->terminus.get() );
			TRACE( "Leaving substitution to duplicate terminus replace pattern\n" );
			return DuplicateSubtree( replace_stuff->terminus, match_keys, shared_ptr<Key>() );
		}
	}

	shared_ptr<Node> dest;

    // Check match sets for a match to the source of our duplication. If one is found, we
	// substitute the match set's key node instead of the supplied source node.
    const MatchSet *match = NULL;
    if( match_keys )
    	match = match_keys->FindMatchSet( source );

    if( match && match->key ) // Only substitute for match sets already keyed
    {
    	ASSERT( !current_key )("Should only find a match in patterns"); // We'll never find a match when we're under substitution, because the
    	                                                                      // source is actually a match key already, so not in any match sets
		// It's in a match set, so substitute the key. Simplest to recurse for this. We will
		// still overlay any non-NULL members of the replace pattern node onto the result (see below)
    	TRACE("substituting because found in match set\n");

    	if( shared_ptr<StuffBase> replace_stuff = dynamic_pointer_cast<StuffBase>(source) )
    	{
    		// If it's a stuff node, do NOT overlay the replace pattern - instead store it in the stuff key
    		// for use when we hit terminus.
    		shared_ptr<StuffKey> stuff_key = dynamic_pointer_cast<StuffKey>(match->key);
    		stuff_key->replace_stuff = replace_stuff;
    	    dest = DuplicateSubtree( match->key->root, match_keys, match->key );
    	}
    	else
    	{
    		// For other nodes, we duplicate and then fall through to overlay - so any non-NULL children in the replace
    		// pattern can be overlayed over the substituted key.
			dest = DuplicateSubtree( match->key->root, match_keys, match->key );

			// Do NOT overlay soft patterns - they must self-terminate TODO inelegant?
			if( !dynamic_pointer_cast<SoftReplacePattern>( source ) )
			{
				// Overlaying requires type compatibility - check for this
				ASSERT( TypeInfo(source) >= TypeInfo(match->key->root) )
					  ( "replace pattern %s must be a non-strict superclass of substitute %s, so that its members are a subset",
						TypeInfo(source).name().c_str(), TypeInfo(match->key->root).name().c_str() );
					  //TODO simply require that every member of a match set has the exact same type

				// Copy the source over,  except for any NULLs in the source. If source is superclass
				// of destination (i.e. has possibly fewer members) the missing ones will be left alone.
				Overlay( dest, source, match_keys, current_key );
			}
    	}
    }
    else
    {
       	// Allow a soft replace pattern to act
		if( shared_ptr<SoftReplacePattern> srp = dynamic_pointer_cast<SoftReplacePattern>( source ) )
		{
			// Substitute is an identifier, so preserve its uniqueness by just returning
			// the same node. Don't do any more - we wouldn't want to change the
			// identifier in the tree even if it had members, lol!
			TRACE("Invoke soft replace pattern %s\n", TypeInfo(srp).name().c_str() );
			ASSERT( !current_key )( "Found soft replace pattern while under substitution\n" );
			shared_ptr<Node> newsource = srp->DuplicateSubtree( this, match_keys );
			ASSERT( newsource );

			// Allow this to key a match set if required
			// TODO do a proper 2-pass thing like with searching, and have a new function like
			// MatchKeys::KeyAndDuplicate() or something
			match_keys->KeyAndRestrict( newsource, source, this, 0 );
			source = newsource;
		}

    	// Do not duplicate specific identifiers.
    	if( dynamic_pointer_cast<SpecificIdentifier>( source ) )
    	{
    		// Substitute is an identifier, so preserve its uniqueness by just returning
    		// the same node. Don't do any more - we wouldn't want to change the
    		// identifier in the tree even if it had members, lol!
    		TRACE("Not duplicating identifiers when under substitution\n");
    		return source;
    	}

     	TRACE("duplicating supplied node\n");
		// Make the new node (destination node)
		shared_ptr<Cloner> dup_dest = Cloner::Clone( source );
		dest = dynamic_pointer_cast<Node>( dup_dest );
		ASSERT(dest);

		// Make all members in the destination be NULL
		ClearPtrs( dest );

		// Copy the source over, not allowing any NULLs this time
		Overlay( dest, source, match_keys, current_key );
    }
    
    ASSERT( dest );
    return dest;
}

#include "render/graph.hpp" // TODO get rid


RootedSearchReplace::Result RootedSearchReplace::SingleSearchReplace( shared_ptr<Program> p )
{
	program = p;

	SharedPtr<Node> prog(p);
	GenericPointIterator pit( prog );
	TRACE("Begin search\n");
	Result r = Compare( *pit, search_pattern, &matches );
	if( r != FOUND )
		return r;

    if( replace_pattern )
    {
    	TRACE("Search successful, now replacing\n");
        ASSERT( replace_pattern );
        matches.SetPass( MatchKeys::KEYING );
        SharedPtr<Node> nn( DuplicateSubtree( replace_pattern, &matches ) );
        pit.Overwrite( &nn );
       	TRACE("Done replace\n");

        // TODO operator() should take a ref to the shared_ptr<Program> and just change it directly
        shared_ptr<Program> pp = dynamic_pointer_cast<Program>(prog);
        ASSERT(pp)("Replace changed root Program node into something else!");
        *p = *pp; // Egregiously copy over the contents of the program node
    }
    matches.CheckMatchSetsKeyed();

    program = shared_ptr<Program>(); // just to avoid us relying on the program outside of a search+replace pass
    return FOUND;
}

// Perform search and replace on supplied program based
// on current patterns and match sets. Does search and replace
// operations repeatedly until there are no more matches.
void RootedSearchReplace::operator()( shared_ptr<Program> p )
{
    int i=0;
    while(1)
    {
    	TRACE("Begin search\n");
    	Result r = SingleSearchReplace( p );
        if( r != FOUND )
            break;
       	ASSERT(i<100)("Too many hits");
        i++;
    }
}


// Find a match set containing the supplied node
const RootedSearchReplace::MatchSet *RootedSearchReplace::MatchKeys::FindMatchSet( shared_ptr<Node> node )
{
	ASSERT( this );
	MatchSet *found = NULL;
	for( set<MatchSet *>::iterator msi = begin();
         msi != end();
         msi++ )
    {
        MatchSet::iterator ni = (*msi)->find( node );
        if( ni != (*msi)->end() )
        {
        	ASSERT( !found )("Found more than one match set for a node - please merge the match sets");
        	found = *msi;
        }
    }
    return found;
}


void RootedSearchReplace::MatchKeys::CheckMatchSetsKeyed()
{
	ASSERT( this );

	int unkeyed=0;
	set<MatchSet *>::iterator msi;
	int i;
    for( msi = begin(), i=0;
         msi != end();
         msi++, i++ )
    {
    	if( !((*msi)->key) )
    	{
    		unkeyed++;
    		TRACE("%d not keyed\n", i);
    	}
    }
    ASSERT( unkeyed==0 )("Detected %d unkeyed match sets", unkeyed);
}


// Reset the keys in all the matchsets 
void RootedSearchReplace::MatchKeys::ClearKeys()
{
	ASSERT( this );

	for( set<MatchSet *>::iterator msi = begin();
         msi != end();
         msi++ )
    {
        (*msi)->key = shared_ptr<Key>();
    }
}


// During search, once two nodes are known to match, use this function to
// 1. Key it into a match set of required and
// 2. Detect whether a match set required two parts of the search tree to match and
// reject if they don't.
RootedSearchReplace::Result RootedSearchReplace::MatchKeys::KeyAndRestrict( shared_ptr<Node> x,
		                                                        shared_ptr<Node> pattern,
		                                                        const RootedSearchReplace *sr,
		                                                        unsigned context_flags )
{
	shared_ptr<Key> key( new Key );
	key->root = x;
	return KeyAndRestrict( key, pattern, sr, context_flags );
}

RootedSearchReplace::Result RootedSearchReplace::MatchKeys::KeyAndRestrict( shared_ptr<Key> key,
		                                                                    shared_ptr<Node> pattern,
		                                                                    const RootedSearchReplace *sr,
		                                                                    unsigned context_flags )
{
	ASSERT( this );
	// Find a match set for this node. If the node is not in a match set then there's
	// nothing for us to do, so return without restricting the search.
	const MatchSet *match = FindMatchSet( pattern );
	if( !match )
		return FOUND;
	TRACE("MATCH: ");

	// If we're keying and we haven't keyed this node so far, key it now
	TRACE("in pass %d ", (int)pass);
	if( pass==KEYING && !(match->key) )
	{
		TRACE("keying... match set %p key ptr %p new value %p\n", &match, &(match->key), key.get());
		match->key = key;

		return FOUND; // always match when keying (could restrict here too as a slight optimisation, but KISS for now)
	}

	// If we're restricting, compare the supplied range with the one we keyed
	if( pass==RESTRICTING )
	{
		TRACE("restricting ");
	    // We are restricting the search, and this node has been keyed, so compare the present tree node
		// with the tree node stored for the match set. This comparison should not match any match sets
		// (it does not include stuff from any search or replace pattern) so do not allow match sets.
		// Since collections (which require decisions) can exist within the tree, we must allow iteration
		// through choices, and since the number of decisions seen may vary, we must start a new conjecture.
		// Therefore, we recurse back to Compare().
		ASSERT( match->key ); // should have been caught by CheckMatchSetsKeyed()
		Result r = sr->Compare( key->root, match->key->root, NULL );
		TRACE("result %d\n", r);
		return r;
	}

	TRACE("\n");
	return FOUND;
}

void RootedSearchReplace::Conjecture::PrepareForDecidedCompare()
{
	ASSERT( this );

	TRACE("Decision prepare\n");
	decisions_count = 0;
}

bool RootedSearchReplace::Conjecture::ShouldTryMore( Result r, int threshold )
{
	ASSERT( this );

	if( r == FOUND )
    	return false; // stop trying if we found a match

    if( decisions_count <= threshold ) // we've made all the decisions we can OR
        return false;                  // our last decision went out of bounds

    return true;
}

RootedSearchReplace::Choice RootedSearchReplace::Conjecture::HandleDecision( RootedSearchReplace::Choice begin,
		                                                         RootedSearchReplace::Choice end )
{
	ASSERT( this );

	// Now we know we have a decision to make; see if it needs to be added to the present Conjecture
	if( size() == decisions_count ) // this decision missing from conjecture?
	{
		ASSERT( size() >= decisions_count ); // consistency check
		push_back( begin ); // append this decision, initialised to begin
		TRACE("Decision %d appending begin\n", decisions_count );
	}

	// Adopt the current decision based on Conjecture
	RootedSearchReplace::Choice c = (*this)[decisions_count]; // Get present decision

	// Check the decision obeys bounds
	if( c == end )
	{
		// throw away the bad iterator; will force initialisation to begin() next time
		// NOTE: we will still return end in this case, i.e. an invlaid iterator. This tells
		// the caller to please not try to do any matching with this decision, but fall out
		// with NOT_FOUND.
		TRACE("Decision %d hit end\n", decisions_count );
		resize( decisions_count );
	}
	else
	{
		// That decision is OK, so move to the next one
		TRACE("Decision %d OK\n", decisions_count );

		bool seen_c=false;
		for( Choice i = begin; i != end; ++i )
			seen_c |= (i==c);
		ASSERT( seen_c )("Decision #%d: c not in x or x.end(), seems to have overshot!!!!", decisions_count);

		decisions_count++;
	}

    return c;
}


SearchReplace::SearchReplace( shared_ptr<Node> sp,
                              shared_ptr<Node> rp,
                              set<MatchSet *> m )
{
	Configure( sp, rp, m );
}


void SearchReplace::Configure( shared_ptr<Node> sp,
                               shared_ptr<Node> rp,
                               set<MatchSet *> m )
{
	if( !sp )
		return;

	// Make a non-rooted search and replace (ie where the base of the search pattern
	// does not have to be the root of the whole program tree).
	// Insert a Stuff node as root of the search pattern
	shared_ptr< Stuff<Scope> > search_root( new Stuff<Scope> );
	search_root->terminus = sp;

	if( rp ) // Is there also a replace pattern?
	{
		// Insert a Stuff node as root of the replace pattern
		shared_ptr< Stuff<Scope> > replace_root( new Stuff<Scope> );
	    replace_root->terminus = rp;

	    // Key them together
	    root_match.clear();
	    root_match.insert( search_root );
	    root_match.insert( replace_root );
	    m.insert( &root_match );

	    // Configure the rooted implementation with new patterns and match sets
	    RootedSearchReplace::Configure( search_root, replace_root, m );
	}
	else
	{
	    // Configure the rooted implementation with new pattern
        RootedSearchReplace::Configure( search_root, rp, m );
	}
}


void RootedSearchReplace::Test()
{
    RootedSearchReplace sr;
    
    {
        // single node with topological wildcarding
        shared_ptr<Void> v(new Void);
        ASSERT( sr.Compare( v, v ) == FOUND );
        shared_ptr<Boolean> b(new Boolean);
        ASSERT( sr.Compare( v, b ) == NOT_FOUND );
        ASSERT( sr.Compare( b, v ) == NOT_FOUND );
        shared_ptr<Type> t(new Type);
        ASSERT( sr.Compare( v, t ) == FOUND );
        ASSERT( sr.Compare( t, v ) == NOT_FOUND );
        ASSERT( sr.Compare( b, t ) == FOUND );
        ASSERT( sr.Compare( t, b ) == NOT_FOUND );
        
        // node points directly to another with TC
        shared_ptr<Pointer> p1(new Pointer);
        p1->destination = v;
        ASSERT( sr.Compare( p1, b ) == NOT_FOUND );
        ASSERT( sr.Compare( p1, p1 ) == FOUND );
        shared_ptr<Pointer> p2(new Pointer);
        p2->destination = b;
        ASSERT( sr.Compare( p1, p2 ) == NOT_FOUND );
        p2->destination = t;
        ASSERT( sr.Compare( p1, p2 ) == FOUND );
        ASSERT( sr.Compare( p2, p1 ) == NOT_FOUND );
    }
    
    {
        // string property
        shared_ptr<SpecificString> s1( new SpecificString );
        shared_ptr<SpecificString> s2( new SpecificString );
        s1->value = "here";
        s2->value = "there";
        ASSERT( sr.Compare( s1, s1 ) == FOUND );
        ASSERT( sr.Compare( s1, s2 ) == NOT_FOUND );
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
        ASSERT( sr.Compare( i1, i1 ) == FOUND );
        ASSERT( sr.Compare( i1, i2 ) == NOT_FOUND );
    }    
    
    {
        // node with sequence, check lengths 
        shared_ptr<Compound> c1( new Compound );
        ASSERT( sr.Compare( c1, c1 ) == FOUND );
        shared_ptr<Nop> n1( new Nop );
        c1->statements.push_back( n1 );
        ASSERT( sr.Compare( c1, c1 ) == FOUND );
        shared_ptr<Nop> n2( new Nop );
        c1->statements.push_back( n2 );
        ASSERT( sr.Compare( c1, c1 ) == FOUND );
        shared_ptr<Compound> c2( new Compound );
        ASSERT( sr.Compare( c1, c2 ) == NOT_FOUND );
        shared_ptr<Nop> n3( new Nop );
        c2->statements.push_back( n3 );
        ASSERT( sr.Compare( c1, c2 ) == NOT_FOUND );
        shared_ptr<Nop> n4( new Nop );
        c2->statements.push_back( n4 );
        ASSERT( sr.Compare( c1, c2 ) == FOUND );
        shared_ptr<Nop> n5( new Nop );
        c2->statements.push_back( n5 );
        ASSERT( sr.Compare( c1, c2 ) == NOT_FOUND );
    }

    {
        // node with sequence, TW 
        shared_ptr<Compound> c1( new Compound );
        shared_ptr<Nop> n1( new Nop );
        c1->statements.push_back( n1 );
        shared_ptr<Compound> c2( new Compound );
        shared_ptr<Statement> s( new Statement );
        c2->statements.push_back( s );
        ASSERT( sr.Compare( c1, c2 ) == FOUND );
        ASSERT( sr.Compare( c2, c1 ) == NOT_FOUND );
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
        ASSERT( sr.Compare( l, d ) == true );
        ASSERT( sr.Compare( d, l ) == false );
        shared_ptr<Private> p3( new Private );
        d->access = p3;
        ASSERT( sr.Compare( l, d ) == false );
        ASSERT( sr.Compare( d, l ) == false );
        shared_ptr<AccessSpec> p4( new AccessSpec );
        d->access = p4;
        ASSERT( sr.Compare( l, d ) == true );
        ASSERT( sr.Compare( d, l ) == false );
        */
    }
}


