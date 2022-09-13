#include "transform_of_agent.hpp"
#include "../scr_engine.hpp"
#include "link.hpp"
#include "the_knowledge.hpp"

using namespace SR;

shared_ptr<PatternQuery> TransformOfAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
	pq->RegisterNormalLink( PatternLink(this, &pattern) );
    return pq;
}


LocatedLink TransformOfAgent::RunTeleportQuery( const TheKnowledge &knowledge, XLink keyer_xlink ) const
{
    // Transform the candidate expression, sharing the knowledge as a TreeKit
    // so that implementations can use handy features without needing to search
    // the tree. Note that transformations work on nodes, not XLinks, so some
    // precision is lost.
    
    // Policy: Don't convert MMAX link to a node (will evaluate to NOT_A_SYMBOL)
    if( keyer_xlink == XLink::MMAX_Link )
         return LocatedLink(); 
         
    TreePtr<Node> keyer_x = keyer_xlink.GetChildX();

    try
    {
		TreePtr<Node> trans_x = (*transformation)( knowledge, keyer_x );      
		if( trans_x )
		{
			ASSERT( trans_x->IsFinal() )(*this)(" computed non-final ")(*trans_x)(" from ")(keyer_x)("\n");             
			XLink tp_xlink = XLink::CreateDistinct(trans_x);  // Cache will un-distinct
			return LocatedLink(PatternLink(this, &pattern), tp_xlink);
		}
		else
		{
			// Transformation returned nullptr, probably because the candidate was incompatible
			// with the transformation - a search MISS.
			TRACE("Got NULL; query fails\n");
			return LocatedLink();  
		}
	}
    catch( const ::Mismatch &e )
    {
		TRACE("Caught ")(e)("; query fails\n");
		return LocatedLink();  
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
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(pattern.get()),
              list<string>{},
              list<string>{},
              phase,
              &pattern );
    block.sub_blocks = { { "pattern", 
                           "", 
                           true,
                           { link } } };
    return block;
}


string TransformOfAgent::GetName() const
{
	return transformation->GetName() + GetSerialString();
}

