#include "soft_patterns.hpp"
#include "coupling.hpp"

using namespace SR;

bool TransformOfBase::MyCompare( const TreePtrInterface &x )
{
    INDENT;
    // Transform the candidate expression
    TreePtr<Node> xt = (*transformation)( *GetContext(), x );
	if( xt )
	{
	    // Punt it back into the search/replace engine
	    return NormalCompare( xt, TreePtr<Node>(pattern) );
	}
	else
	{
	    // Transformation returned NULL, probably because the candidate was incompatible
        // with the transofrmation - a search MISS.
	    return false;
	}
}

