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


TeleportAgent::QueryReturnType TransformOfAgent::RunTeleportQuery( const XTreeDatabase *db, DependencyReporter *dep_rep, XLink stimulus_xlink ) const
{
    // Transform the candidate expression, sharing the x_tree_db as a TreeKit
    // so that implementations can use handy features without needing to search
    // the tree. Note that transformations work on nodes, not XLinks, so some
    // precision is lost.
    
    // Policy: Don't convert MMAX link to a node (will evaluate to EmptyResult)
    if( stimulus_xlink == XLink::MMAX_Link )
         return QueryReturnType(); 
         
    AugTreePtr<Node> stimulus_x = AugTreePtr<Node>(stimulus_xlink.GetChildX()); // TODO don't assume "free" mkind

	TreeUtils utils(db, dep_rep);
    TreeKit kit { &utils };

    try
    {
		AugTreePtr<Node> atp = transformation->ApplyTransformation( kit, stimulus_x );  
		             
		if( !atp ) // NULL
            return QueryReturnType(); 		
		
		const TreePtrInterface *ptp = utils.GetPTreePtr(atp);
		TreePtr<Node> tp = utils.GetTreePtr(atp);
		ASSERT( tp->IsFinal() )(*this)(" computed non-final ")(tp)(" from ")(stimulus_x)("\n");                
		
        if( ptp ) 
            return make_pair(db->GetXLink( ptp ), nullptr);  // parent was specified      
		else
            return make_pair(XLink(), tp);  // no parent specified 
	}
    catch( const ::Mismatch &e )
    {
		TRACE("Caught ")(e)("; query fails\n");
		return QueryReturnType(); // NULL
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


bool TransformOfAgent::IsExtenderChannelLess( const Extender &r ) const
{
	// If comparing two TransformOfAgent, secondary onto the transformation object's type
	// TODO transformation object's state might matter, so should call into it
	if( auto rto = dynamic_cast<const TransformOfAgent *>(&r) )
		return typeid(*transformation).before(typeid(*rto->transformation));
	
	// Otherwise resort to the default compare
	return TeleportAgent::IsExtenderChannelLess(r);
}


int TransformOfAgent::GetExtenderChannelOrdinal() const
{
	return 1; // TODO class id as an ordinal?
}
