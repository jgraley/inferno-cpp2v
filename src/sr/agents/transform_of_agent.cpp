#include "transform_of_agent.hpp"
#include "../scr_engine.hpp"
#include "link.hpp"

using namespace SR;

shared_ptr<PatternQuery> TransformOfAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>(this);
	pq->RegisterNormalLink( PatternLink(this, &pattern) );
    return pq;
}


void TransformOfAgent::RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                            XLink xlink ) const
{
    INDENT("T");
    query.Reset();
    
    auto op = [&](XLink xlink) -> XLink
    {
        // Transform the candidate expression, sharing the overall S&R context so that
        // things like GetDeclaration can work (they search the whole program tree).
        TreePtr<Node> x = xlink.GetChildX();
        TreePtr<Node> xt = (*transformation)( master_scr_engine->GetOverallMaster()->GetContext(), x );
        if( xt )
        {
            // Punt it back into the search/replace engine
            XLink new_xlink = XLink::CreateDistinct(xt);  // Cache will un-distinct
            return master_scr_engine->UniquifyDomainExtension(new_xlink);
        }
        else
        {
            // Transformation returned nullptr, probably because the candidate was incompatible
            // with the transofrmation - a search MISS.
            throw Mismatch();  
        }
    };
    query.RegisterNormalLink( PatternLink(this, &pattern), cache( xlink, op ) );    
}


Graphable::Block TransformOfAgent::GetGraphBlockInfo() const
{
    Block block;
	// The TransformOf node appears as a slightly flattened hexagon, with the name of the specified 
	// kind of Transformation class inside it.
	block.bold = true;
	block.title = transformation->GetName();
	block.shape = "hexagon";
    block.block_type = Graphable::NODE;
    block.sub_blocks = { { "pattern", 
                           "", 
                           { { pattern, 
                               SOLID, 
                               {},
                               {PatternLink(this, &pattern).GetShortName()} } } } };
    return block;
}
