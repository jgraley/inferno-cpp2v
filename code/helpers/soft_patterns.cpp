#include "soft_patterns.hpp"
#include "helpers/misc.hpp"
#include "tree/tree.hpp"

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
		// TODO no need for this, the pre-restriction will take care of wrong type. But maybe
		// want this for other invalid cases?
	    return NOT_FOUND;
	}
}



TreePtr<Node> SoftMakeIdentifier::DuplicateSubtree( const RootedSearchReplace *sr,
		                                               CouplingKeys *keys,
		                                               bool can_key )
{
	string newname;
	if( source )
	{
		TRACE("Begin SoftMakeIdentifier recurse\n");
		// We have a child identifier - let replace algorithm run in the expectation it will
		// get subsitituted with a SpecificIdentifier from the original program tree
	    TreePtr<Node> n = sr->DuplicateSubtree( TreePtr<Node>(source), keys, can_key );
		TRACE("End SoftMakeIdentifier recurse\n");
	    ASSERT( n );
	    TreePtr<SpecificIdentifier> si = dynamic_pointer_cast<SpecificIdentifier>( n );
	    ASSERT( si )("trying to make an identifier from ")(*n)(" which should be a kind of SpecificIdentifier");
	    // Use sprintf to build a new identifier based on the found one. Obviously %s
	    // becomes the old identifier's name.
	    newname = SSPrintf(format.c_str(), ((string)*si).c_str());
	}
	else
	{
		newname = format;
	}
	return TreePtr<SpecificInstanceIdentifier>( new SpecificInstanceIdentifier( newname ) );
}


