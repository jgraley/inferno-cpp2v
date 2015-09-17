#include "soft_patterns.hpp"

using namespace SR;

deque< TreePtr<Node> > TransformOfBase::MyPatternQuery() const
{
	deque< TreePtr<Node> > child_patterns;
	child_patterns.push_back( TreePtr<Node>(pattern) );
	return child_patterns;
}


bool TransformOfBase::MyCompare( const TreePtrInterface &x ) const
{
    INDENT("T");
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

