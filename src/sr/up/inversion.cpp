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


shared_ptr<CommandSequence> TreeZoneInverter::Run(shared_ptr<Command> initial_cmd)
{
	incremental_seq = make_shared<CommandSequence>();
	
	auto root_update_cmd = dynamic_pointer_cast<ReplaceCommand>(initial_cmd);
	ASSERT( root_update_cmd ); // ASSUME command is ReplaceCommand (and not in a seq)
	shared_ptr<ZoneExpression> *root_expr_ptr = root_update_cmd->GetExpressionPtr();
	TreeZone root_target                      = root_update_cmd->GetTargetTreeZone();
	LocatedZoneExpression root_lze( root_target.GetBaseXLink(), root_expr_ptr );
	WalkFreeZoneExpr( root_lze );
	
	return incremental_seq;
}


void TreeZoneInverter::WalkFreeZoneExpr( LocatedZoneExpression lze )
{
	// Really just a search for MergeFreeZoneOperator that fills in the target base XLink from the 
	// enclosing thing (if it's root or a tree zone). 
	// Inversion strategy: this XLink is available for every free zone because we did free zone
	// merging (if parent was a free zone, we'd have no XLink)
	if( auto pfz_op = dynamic_pointer_cast<MergeFreeZoneOperator>(*lze.second) )
	{
		// Free zone: recurse and then invert locally
		pfz_op->ForChildren( [&](shared_ptr<ZoneExpression> &child_expr)	
		{
			// We don't know the base if we're coming from a free zone
			ASSERT( dynamic_pointer_cast<DupMergeTreeZoneOperator>(child_expr) )
			      ("FZ under another FZ (probably), cannot determine target XLink");
			LocatedZoneExpression child_lze( XLink(), &child_expr );
			WalkFreeZoneExpr( child_lze );
		} );
	
		// Invert the free zone while unwinding
		Invert(lze); 
	}
	else if( auto ptz_op = dynamic_pointer_cast<DupMergeTreeZoneOperator>(*lze.second) )
	{
		// Recurse, co-looping over the children/terminii so we can fill in target bases
		vector<XLink> terminii = ptz_op->GetZone().GetTerminusXLinks();
		MergeFreeZoneOperator::ChildExpressionIterator it_child = ptz_op->GetChildrenBegin();		
		for( XLink terminus_xlink : terminii )
		{
			ASSERT( it_child != ptz_op->GetChildrenEnd() ); // length mismatch
			
			LocatedZoneExpression child_lze( terminus_xlink, &*it_child );
			WalkFreeZoneExpr( child_lze );
						
			++it_child;
		}
		ASSERT( it_child == ptz_op->GetChildrenEnd() ); // length mismatch
	}
	else
		ASSERTFAIL();
}


void TreeZoneInverter::Invert( LocatedZoneExpression lze )
{
	// Checks
	ASSERT( lze.first && lze.second && *lze.second);
	XLink base_xlink = lze.first;
	ASSERT( base_xlink )("Got no base in our lze, perhaps parent was free zone?"); // FZ merging should prevent
	auto pfz_op = dynamic_pointer_cast<MergeFreeZoneOperator>( *lze.second );
	ASSERT( pfz_op )("Got LZE not a free zone: ")(lze); // WalkFreeZoneExpr() gave us wrong kind of expr
			
	// Collect base xlinks for child zones (which must be tree zones)
	vector<XLink> terminii_xlinks;
	pfz_op->ForChildren([&](shared_ptr<ZoneExpression> &child_expr)	
	{
		auto child_ptz_op = dynamic_pointer_cast<DupMergeTreeZoneOperator>( child_expr );		
		// Inversion strategy: we're based on a free zone and FZ merging should 
		// have ensured we'll see only tree zones as children. Each base is a terminus 
		// for the new tree zone.
		ASSERT( child_ptz_op ); 
		
		terminii_xlinks.push_back( child_ptz_op->GetZone().GetBaseXLink() );
	} );
		 
	// Make the inverted TZ
	TreeZone inverted_tree_zone = TreeZone( base_xlink, terminii_xlinks );	
	
	// NEW: Modify the expression to include inverted TZ as target
	*lze.second = make_shared<ReplaceOperator>( inverted_tree_zone,
	                                            make_shared<FreeZone>( pfz_op->GetZone() ),
	                                            pfz_op->MoveChildExpressions() );   	
	
	// OLD: Create and add a command to swap the FZ in over the inverted TZ
	auto pfz_op_no_children = make_shared<MergeFreeZoneOperator>( pfz_op->GetZone() ); // No children leaves terminii exposed, sort of.
	auto incremental_command = make_shared<ReplaceCommand>( inverted_tree_zone, pfz_op_no_children );	
	incremental_seq->Add(incremental_command);
}
