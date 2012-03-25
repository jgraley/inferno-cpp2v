#include "soft_patterns.hpp"

bool TransformOfBase::DecidedCompare( const CompareReplace *sr,
		                                                     const TreePtrInterface &x,
		                                                     bool can_key,
		                                                     Conjecture &conj )
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
	    return false;
	}
}


