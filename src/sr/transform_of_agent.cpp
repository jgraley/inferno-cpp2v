#include "sr/transform_of_agent.hpp"

using namespace SR;

PatternQueryResult TransformOfAgent::PatternQuery() const
{
    PatternQueryResult r;
	r.AddLink( false, AsAgent(pattern) );
    return r;
}


DecidedQueryResult TransformOfAgent::DecidedQuery( const TreePtrInterface *px,
                                                   const AgentQuery::Choices &choices ) const
{
    INDENT("T");
    DecidedQueryResult r;
    
    // Transform the candidate expression, sharing the overall S&R context so that
    // things like GetDeclaration can work (they search the whole program tree).
    TreePtr<Node> xt = (*transformation)( *(engine->GetOverallMaster()->pcontext), *px );
	if( xt )
	{
	    // Punt it back into the search/replace engine
	    r.AddLocalLink( false, AsAgent(pattern), xt );
	}
	else
	{
	    // Transformation returned NULL, probably because the candidate was incompatible
        // with the transofrmation - a search MISS.
	    r.AddLocalMatch(false);
	}
    
    return r;
}


void TransformOfAgent::GetGraphAppearance( bool *bold, string *text, string *shape ) const
{
	// The TransformOf node appears as a slightly flattened hexagon, with the name of the specified 
	// kind of Transformation class inside it.
	*bold = true;
	*shape = "hexagon";
	*text = transformation->GetName();
}
