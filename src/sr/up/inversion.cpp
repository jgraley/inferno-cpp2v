#include "inversion.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "zone_commands.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

TreeZoneInverter::TreeZoneInverter( XTreeDatabase *db_ ) :
	db( db_ )
{
}


shared_ptr<Command> TreeZoneInverter::Run(shared_ptr<Command> initial_cmd)
{
	incremental_seq = make_shared<CommandSequence>();
	
	auto root_update_cmd = dynamic_pointer_cast<UpdateTreeCommand>(initial_cmd);
	ASSERT( root_update_cmd ); // ASSUME command is UpdateTreeCommand (and not in a seq)
	shared_ptr<ZoneExpression> root_expr = root_update_cmd->GetExpression();
	TreeZone root_target = root_update_cmd->GetTargetTreeZone();
	LocatedZoneExpression root_lze( root_target.GetBaseXLink(), root_expr );
	WalkFreeZoneExpr( root_lze );
	
	return incremental_seq;
}


void TreeZoneInverter::WalkFreeZoneExpr( LocatedZoneExpression lze )
{
	// Really just a search for PopulateFreeZoneOperator that returns the base XLink from the 
	// enclosing thing (root or a tree zone). 
	// Inversion strategy: this XLink is available for every free zone because we did free zone
	// merging (if parent was a free zone, we'd have no XLink)
	if( auto pfz_op = dynamic_pointer_cast<PopulateFreeZoneOperator>(lze.second) )
	{
		pfz_op->ForChildren( [&](shared_ptr<ZoneExpression> &child_expr)	
		{
			// We don't know the base if we're coming from a free zone
			WalkFreeZoneExpr( make_pair( XLink(), child_expr ) );
		} );
	
		// Invert the free zone while unwinding
		Invert(lze); 
	}
	else if( auto ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(lze.second) )
	{
		// Co-loop over the chidren/terminii looking for a free zone
		vector<XLink> terminii = ptz_op->GetZone().GetTerminusXLinks();
		PopulateFreeZoneOperator::ChildExpressionIterator it_child = ptz_op->GetChildrenBegin();		
		for( XLink terminus_xlink : terminii )
		{
			ASSERT( it_child != ptz_op->GetChildrenEnd() ); // length mismatch
			
			WalkFreeZoneExpr( make_pair( terminus_xlink, *it_child ) );
						
			++it_child;
		}
		ASSERT( it_child == ptz_op->GetChildrenEnd() ); // length mismatch
	}
	else
		ASSERTFAIL();
}


void TreeZoneInverter::Invert( LocatedZoneExpression lze )
{
	ASSERT( lze.first && lze.second );
	
	XLink base_xlink = lze.first;
	ASSERT( base_xlink )("Got no base in our lze, perhaps parent was free zone?"); // FZ merging should prevent
	auto pfz_op = dynamic_pointer_cast<PopulateFreeZoneOperator>( lze.second );
	ASSERT( pfz_op )("Got LZE not a free zone: ")(lze); // TryFindFreeZoneExpr() found wrong kind of expr
			
	vector<XLink> terminii_xlinks;
	pfz_op->ForChildren([&](shared_ptr<ZoneExpression> &child_expr)	
	{
		auto child_ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>( child_expr );		
		// Inversion strategy: we're based on a free zone and FZ merging should 
		// have ensured we'll see only tree zones as children. Each base is a terminus 
		// for the new tree zone.
		ASSERT( child_ptz_op ); 
		
		terminii_xlinks.push_back( child_ptz_op->GetZone().GetBaseXLink() );
	} );
		 
	TreeZone inverted_tree_zone = TreeZone( base_xlink, terminii_xlinks );	
	auto pfz_op_no_children = make_shared<PopulateFreeZoneOperator>( pfz_op->GetZone() ); // No children leaves terminii exposed, sort of.
	auto incremental_command = make_shared<UpdateTreeCommand>( inverted_tree_zone, pfz_op_no_children );	
	incremental_seq->Add(incremental_command);
}
