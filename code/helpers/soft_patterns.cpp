#include "soft_patterns.hpp"
#include "helpers/misc.hpp"
#include "tree/tree.hpp"


shared_ptr<Node> SoftMakeIdentifier::DuplicateSubtree( const RootedSearchReplace *sr,
		                                               RootedSearchReplace::CouplingKeys *keys,
		                                               bool can_key )
{
	string newname;
	if( source )
	{
		TRACE("Begin SoftMakeIdentifier recurse\n");
		// We have a child identifier - let replace algorithm run in the expectation it will
		// get subsitituted with a SpecificIdentifier from the original program tree
	    shared_ptr<Node> n = sr->DuplicateSubtree( shared_ptr<Node>(source), keys, can_key );
		TRACE("End SoftMakeIdentifier recurse\n");
	    ASSERT( n );
	    shared_ptr<SpecificIdentifier> si = dynamic_pointer_cast<SpecificIdentifier>( n );
	    ASSERT( si )("trying to make an identifier from ")(*n)(" which should be a kind of SpecificIdentifier");
	    // Use sprintf to build a new identifier based on the found one. Obviously %s
	    // becomes the old identifier's name.
	    newname = SSPrintf(format.c_str(), ((string)*si).c_str());
	}
	else
	{
		newname = format;
	}
	return shared_ptr<SpecificInstanceIdentifier>( new SpecificInstanceIdentifier( newname ) );
}


