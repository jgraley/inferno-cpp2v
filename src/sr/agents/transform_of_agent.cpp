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


TeleportAgent::TeleportResult TransformOfAgent::RunTeleportQuery( const XTreeDatabase *db, DependencyReporter *dep_rep, XLink start_xlink ) const
{
    // Transform the candidate expression, sharing the x_tree_db as a TreeKit
    // so that implementations can use handy features without needing to search
    // the tree. Note that transformations work on nodes, not XLinks, so some
    // precision is lost.
    
    // Policy: Don't convert MMAX link to a node (will evaluate to NOT_A_SYMBOL)
    if( start_xlink == XLink::MMAX_Link )
         return TeleportResult(); 
         
    TreePtr<Node> start_x = start_xlink.GetChildX();

    TreeKit kit { db, dep_rep };

    try
    {
		AugTreePtr<Node> trans = transformation->ApplyTransformation( kit, start_x );   // TODO use AugTreePtr result, turn into pair<Xlink, TreePtr<Node>>   
		ASSERT( !trans || ((TreePtr<Node>)trans)->IsFinal() )(*this)(" computed non-final ")((TreePtr<Node>)trans)(" from ")(start_x)("\n");                             
		if( !(TreePtr<Node>)trans ) // NULL
            return TeleportResult(); 
        else if( trans.p_tree_ptr == nullptr ) // no parent specified
            return make_pair(XLink(), (TreePtr<Node>)trans); 
        else // parent was specified
        {
            XLink xlink((TreePtr<Node>)trans, trans.p_tree_ptr);
            return make_pair(xlink, (TreePtr<Node>)trans);             
        }
	}
    catch( const ::Mismatch &e )
    {
		TRACE("Caught ")(e)("; query fails\n");
		return TeleportResult(); // NULL
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


bool TransformOfAgent::IsExtenderLess( const Extender &r ) const
{
	// If comparing two TransformOfAgent, secondary onto the transformation object's type
	// TODO transformation object's state might matter, so should call into it
	if( auto rto = dynamic_cast<const TransformOfAgent *>(&r) )
		return typeid(*transformation).before(typeid(*rto->transformation));
	
	// Otherwise resort to the default compare
	return TeleportAgent::IsExtenderLess(r);
}


int TransformOfAgent::GetExtenderOrdinal() const
{
	return 1;
}
