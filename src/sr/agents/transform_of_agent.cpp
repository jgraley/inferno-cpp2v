#include "transform_of_agent.hpp"
#include "../scr_engine.hpp"
#include "link.hpp"
#include "db/x_tree_database.hpp"

using namespace SR;

shared_ptr<PatternQuery> TransformOfAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
	pq->RegisterNormalLink( PatternLink(this, &pattern) );
    return pq;
}


TreePtr<Node> TransformOfAgent::RunTeleportQuery( const TreeKit &kit, XLink keyer_xlink ) const
{
    // Transform the candidate expression, sharing the x_tree_db as a TreeKit
    // so that implementations can use handy features without needing to search
    // the tree. Note that transformations work on nodes, not XLinks, so some
    // precision is lost.
    
    // Policy: Don't convert MMAX link to a node (will evaluate to NOT_A_SYMBOL)
    if( keyer_xlink == XLink::MMAX_Link )
         return TreePtr<Node>(); 
         
    TreePtr<Node> keyer_x = keyer_xlink.GetChildX();

    try
    {
		TreePtr<Node> trans_x = GET_NODE( (*transformation)( kit, keyer_x ) );      
		ASSERT( !trans_x || trans_x->IsFinal() )(*this)(" computed non-final ")(*trans_x)(" from ")(keyer_x)("\n");             
		return trans_x; // can be NULL
	}
    catch( const ::Mismatch &e )
    {
		TRACE("Caught ")(e)("; query fails\n");
		return TreePtr<Node>();  
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

