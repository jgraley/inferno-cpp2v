#include "coupling.hpp"
#include "search_replace.hpp" // TODO remove the need for this: search side only uses couplings member, replace side calls in but could maybe declare local as per search side?

Result CouplingKeys::KeyAndRestrict( TreePtr<Node> x,
									 TreePtr<Node> pattern,
									 const CompareReplace *sr,
									 bool can_key )
{
	shared_ptr<Key> key( new Key );
    key->root = x;
	return KeyAndRestrict( key, pattern, sr, can_key );
}

Result CouplingKeys::KeyAndRestrict( shared_ptr<Key> key,
									 TreePtr<Node> pattern,
									 const CompareReplace *sr,
									 bool can_key )
{
	ASSERT( this );
    ASSERT( key );
    ASSERT( key->root );
    ASSERT( key->root->IsFinal() );
	// Find a coupling for this node. If the node is not in a coupling then there's
	// nothing for us to do, so return without restricting the search.
	if( !sr->couplings.IsExist(pattern) )
		return FOUND;

	// If we're keying and we haven't keyed this node so far, key it now
	TRACE("MATCH: can_key=%d\n", (int)can_key);
	if( can_key && !keys_map[pattern] )
	{
		TRACE("keying... to ")
		     (*(key->root))
		     (" key ptr %p new value %p, presently %d keys out of %d couplings\n",
				keys_map[pattern].get(), key.get(),
				keys_map.size(), sr->couplings.size() );

		keys_map[pattern] = key;

		return FOUND; // always match when keying (could restrict here too as a slight optimisation, but KISS for now)
	}

    // Always restrict
	// We are restricting the search, and this node has been keyed, so compare the present tree node
	// with the tree node stored for the coupling. This comparison should not match any couplings
	// (it does not include stuff from any search or replace pattern) so do not allow couplings.
	// Since collections (which require decisions) can exist within the tree, we must allow iteration
	// through choices, and since the number of decisions seen may vary, we must start a new conjecture.
	// Therefore, we recurse back to Compare().
	ASSERT( keys_map[pattern] ); // should have been caught by CheckMatchSetsKeyed()
	Result r;

	if( key->root != keys_map[pattern]->root )
	{
		r = sr->Compare( key->root, keys_map[pattern]->root );
	}
	else
		r = FOUND; // TODO optimisation being done in wrong place
	TRACE("result %d\n", r);
	return r;
}

TreePtr<Node> CouplingKeys::KeyAndSubstitute( TreePtr<Node> x,
											  TreePtr<Node> pattern,
											  const CompareReplace *sr,
											  bool can_key )
{
	shared_ptr<Key> key( new Key );
	if( x )
        key->root = x;
    else
        key = shared_ptr<Key>();
	return KeyAndSubstitute( key, pattern, sr, can_key );
}

// Note return is NULL in all cases unless we substituted in which case it is the result of the
// substitution, duplicated for our convenience. Always check the return value for NULL.
TreePtr<Node> CouplingKeys::KeyAndSubstitute( shared_ptr<Key> key, // key may be NULL meaning we are not allowed to key the node
		                                                           TreePtr<Node> pattern,
		                                                           const CompareReplace *sr,
		                                                           bool can_key )
{
	INDENT;
	ASSERT( this );
    ASSERT( pattern );
    
    if( key )
    {
        ASSERT( key->root );
        ASSERT( key->root != pattern )
              ("keyed to ")
              (*(key->root))
              (" apparently in the search or replace pattern\n"); // just a general usage check
    }
	// Find a coupling for this node. If the node is not in a coupling then there's
	// nothing for us to do, so return without restricting the search.	
	TRACE("Looking for coupling for ")(*pattern);
    if( !sr->couplings.IsExist(pattern) )
    {
        TRACE(" not found\n");
		return TreePtr<Node>();
    }
	TRACE(" found ");
#if 0    
	TRACE("coupling={");
	bool first=true;
	FOREACH( TreePtr<Node> n, coupling )
	{
		if( !first )
			TRACE(", ");
		if( pattern == n )
			TRACE("-->");
		TRACE(*n);
		first=false;
	}
   TRACE("} key ptr=%p\n", keys_map[coupling].get()); // TODO put this in as a common utility somewhere
#endif

	// If we're keying and we haven't keyed this node so far, key it now
	TRACE("can_key=%d\n", (int)can_key);
	if( can_key && key && !keys_map[pattern] )
	{
		TRACE("keying... coupling %p key ptr %p new value %p, presently %d keys out of %d couplings\n",
				&pattern, &keys_map[pattern], key.get(),
				keys_map.size(), sr->couplings.size() );
		keys_map[pattern] = key;

		return key->root;
	}

	if( keys_map[pattern] )
	{
		// Always substitute
		TRACE("substituting ");
		ASSERT( keys_map[pattern] );
		keys_map[pattern]->replace_pattern = pattern; // Only fill this in while substituting under the node
		TreePtr<Node> subs = sr->DuplicateSubtree( keys_map[pattern]->root, this, can_key, keys_map[pattern] ); // Enter substitution
		// TODO can_key should be false in the above?
		keys_map[pattern]->replace_pattern = TreePtr<Node>();
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

