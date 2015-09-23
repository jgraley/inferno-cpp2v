#include "sr/transform_of_agent.hpp"

using namespace SR;

void TransformOfAgent::PatternQueryImpl() const
{
	RememberLink( false, AsAgent(pattern) );
}


bool TransformOfAgent::DecidedQueryImpl( const TreePtrInterface &x ) const
{
    INDENT("T");
    // Transform the candidate expression, sharing the overall S&R context so that
    // things like GetDeclaration can work (they search the whole program tree).
    TreePtr<Node> xt = (*transformation)( *(engine->GetOverallMaster()->pcontext), x );
	if( xt )
	{
	    // Punt it back into the search/replace engine
	    RememberLocalLink( false, AsAgent(pattern), xt );
	    return true;
	}
	else
	{
	    // Transformation returned NULL, probably because the candidate was incompatible
        // with the transofrmation - a search MISS.
	    return false;
	}
}

