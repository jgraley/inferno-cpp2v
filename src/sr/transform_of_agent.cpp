#include "sr/transform_of_agent.hpp"

using namespace SR;

shared_ptr<PatternQuery> TransformOfAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
	pq->RegisterNormalLink( &pattern );
    return pq;
}


void TransformOfAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                            TreePtr<Node> x ) const
{
    INDENT("T");
    query.Reset();
    
    // Transform the candidate expression, sharing the overall S&R context so that
    // things like GetDeclaration can work (they search the whole program tree).
    TreePtr<Node> xt = (*transformation)( master_scr_engine->GetOverallMaster()->GetContext(), x );
	if( xt )
	{
	    // Punt it back into the search/replace engine
	    query.RegisterNormalLink( &pattern, xt );  // Link to Generated (could be elsewhere in x)
	}
	else
	{
	    // Transformation returned NULL, probably because the candidate was incompatible
        // with the transofrmation - a search MISS.
	    throw Mismatch();  
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
