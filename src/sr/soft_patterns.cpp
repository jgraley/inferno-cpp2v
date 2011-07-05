#include "soft_patterns.hpp"
#include "tree/tree.hpp"

Result TransformOfBase::DecidedCompare( const CompareReplace *sr,
		                                                     TreePtr<Node> x,
		                                                     bool can_key,
		                                                     Conjecture &conj ) const
{
    INDENT;
    // Transform the candidate expression
    TreePtr<Node> xt = (*transformation)( sr->GetContext(), x );
	if( xt )
	{
	    // Punt it back into the search/replace engine
	    return sr->DecidedCompare( xt, TreePtr<Node>(pattern), can_key, conj );
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


string BuildIdentifierBase::GetNewName( const CompareReplace *sr )
{
    INDENT;
    TRACE("Begin SoftMakeIdentifier recurse for \"")(format)("\"\n");
    vector<string> vs;
    bool all_same = true;
    FOREACH( TreePtr<Node> source, sources )
    {
        ASSERT( source );
        // We have a child identifier - let replace algorithm run in the expectation it will
        // get subsitituted with a SpecificIdentifier from the original program tree
        TreePtr<Node> n = sr->DuplicateSubtreePattern( TreePtr<Node>(source) );
        TRACE("End SoftMakeIdentifier recurse\n");
        ASSERT( n );
        TreePtr<SpecificIdentifier> si = dynamic_pointer_cast<SpecificIdentifier>( n );
        ASSERT( si )("trying to make an identifier from ")(*n)(" which should be a kind of SpecificIdentifier");
        string s = si->GetName();
        if( !vs.empty() )
            all_same = all_same && (s == vs.back());
        vs.push_back( s );
    }

    // Optional functionality: when every identifier has the same name, just return that
    // name. Handy for "merging" operations.
    if( (flags & BYPASS_WHEN_IDENTICAL) && all_same )
        return vs[0];  

    // Use sprintf to build a new identifier based on the found one. Obviously %s
    // becomes the old identifier's name.
    switch( vs.size() )
    {
        case 0:
            return SSPrintf( format.c_str() );
        case 1:
            return SSPrintf( format.c_str(), vs[0].c_str() );
        case 2:
            return SSPrintf( format.c_str(), vs[0].c_str(), vs[1].c_str() );
        default:
            ASSERTFAIL("Please add more cases to GetNewName()");
    }
}

