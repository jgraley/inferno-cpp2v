#include "sr/transform_of_agent.hpp"

using namespace SR;

PatternQueryResult TransformOfAgent::PatternQuery() const
{
    PatternQueryResult r;
	r.AddLink( false, AsAgent(pattern) );
    return r;
}


void TransformOfAgent::DecidedQuery( QueryAgentInterface &query,
                                     const TreePtrInterface *px ) const
{
    INDENT("T");
    query.Reset();
    
    // Transform the candidate expression, sharing the overall S&R context so that
    // things like GetDeclaration can work (they search the whole program tree).
    TreePtr<Node> xt = (*transformation)( *(engine->GetOverallMaster()->pcontext), *px );
	if( xt )
	{
	    // Punt it back into the search/replace engine
	    query.AddLocalLink( false, AsAgent(pattern), xt );
	}
	else
	{
	    // Transformation returned NULL, probably because the candidate was incompatible
        // with the transofrmation - a search MISS.
	    query.AddLocalMismatch();
	}
}


void TransformOfAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The TransformOf node appears as a slightly flattened hexagon, with the name of the specified 
	// kind of Transformation class inside it.
	*bold = true;
	*shape = "hexagon";
	*text = transformation->GetName();
}
