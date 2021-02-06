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


map<PatternLink, XLink> TransformOfAgent::RunTeleportQuery( XLink base_xlink ) const
{
    // Transform the candidate expression, sharing the overall S&R context so that
    // things like GetDeclaration can work (they search the whole program tree).
    TreePtr<Node> base_x = base_xlink.GetChildX();
    TreePtr<Node> trans_x = (*transformation)( master_scr_engine->GetOverallMaster()->GetContext(), base_x );
    if( trans_x )
    {
        ASSERT( trans_x->IsFinal() )(*this)(" computed non-final ")(*trans_x)(" from ")(base_x)("\n");             
        XLink tp_xlink = XLink::CreateDistinct(trans_x);  // Cache will un-distinct
        return { { PatternLink(this, &pattern), tp_xlink } };
    }
    else
    {
        // Transformation returned nullptr, probably because the candidate was incompatible
        // with the transformation - a search MISS.
        throw TransformationFailedMismatch();  
    }
}


Graphable::Block TransformOfAgent::GetGraphBlockInfo() const
{
    Block block;
	// The TransformOf node appears as a slightly flattened octagon, with the name of the specified 
	// kind of Transformation class inside it.
	block.bold = true;
	block.title = transformation->GetName();
	block.shape = "octagon";
    block.block_type = Graphable::NODE;
    block.sub_blocks = { { "pattern", 
                           "", 
                           true,
                           { { pattern, 
                               &pattern,
                               THROUGH, 
                               {},
                               {PatternLink(this, &pattern).GetShortName()} } } } };
    return block;
}
