#include "inversion.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "commands.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

TreeZoneInverter::TreeZoneInverter( Command *initial_cmd, XTreeDatabase *db_ ) :
	db( db_ ),
	root_update_cmd( dynamic_cast<UpdateTreeCommand *>(initial_cmd) ),
	root_expr( &root_update_cmd->GetExpression() ),
	root_tree_zone( root_update_cmd->GetTargetTreeZone() )
{
}


void TreeZoneInverter::Run()
{
	incremental_seq = make_shared<CommandSequence>();
	
	LocatedZoneExpression root_lze( root_tree_zone.GetBaseXLink(), root_expr );
	while(true)
	{
		LocatedZoneExpression lfze = TryFindFreeZoneExpr( root_lze );
		if( lfze.first && lfze.second )
			Invert( lfze );
		else
			break;
	}	
	
	FTRACE(incremental_seq)("\n");
}


shared_ptr<CommandSequence> TreeZoneInverter::GetIncrementalSeq()
{
	return incremental_seq;
}


TreeZoneInverter::LocatedZoneExpression TreeZoneInverter::TryFindFreeZoneExpr( LocatedZoneExpression lze )
{
	// Really just a search for PopulateFreeZoneOperator that returns the base XLink from the 
	// enclosing thing (root or a tree zone). 
	// Inversion strategy: this XLink is available for every free zone because we did free zone
	// merging (if parent was a free zone, we'd have no XLink)
	if( auto pfz_op = dynamic_pointer_cast<PopulateFreeZoneOperator>(*lze.second) )
	{
		return lze; 
	}
	else if( auto ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(*lze.second) )
	{
		// Co-loop over the chidren/terminii looking for a free zone
		set<XLink, DepthFirstRelation> terminii = ptz_op->GetZone().GetTerminusXLinks();
		PopulateFreeZoneOperator::ChildExpressionIterator it_child = ptz_op->GetChildrenBegin();		
		for( XLink terminus_xlink : terminii )
		{
			ASSERT( it_child != ptz_op->GetChildrenEnd() ); // length mismatch
			
			LocatedZoneExpression lze_child = TryFindFreeZoneExpr( make_pair( terminus_xlink, &*it_child ) );
			if( lze_child.first && lze_child.second )
				return lze_child;
						
			++it_child;
		}
		ASSERT( it_child == ptz_op->GetChildrenEnd() ); // length mismatch
	}
	else
		ASSERTFAIL();
		
	// We didn't find any free zones
	return LocatedZoneExpression(XLink(), nullptr);
}


void TreeZoneInverter::Invert( LocatedZoneExpression lze )
{
	ASSERT( lze.first && lze.second );
	
	XLink base_xlink = lze.first;
	shared_ptr<ZoneExpression> *base_expr_p = lze.second;
	auto pfz_op = dynamic_pointer_cast<PopulateFreeZoneOperator>( *base_expr_p );
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
	
	// Quark algo: trying to remove the FreeZone from the expression tree under the
	// initial command causes TWO copes of the inverted tree zone to appear.
	// The first is patched back into the initial tree (reducing FZ count by 1 in
	// that tree) and the other is paired up with the free zone in a localised update
	// tree command.
	
	// 1st copy of TZ: switch initial update command in place 
	TreeZone inverted_tree_zone = TreeZone( db, base_xlink, terminii_xlinks );	
	*base_expr_p = make_shared<PopulateTreeZoneOperator>( inverted_tree_zone, pfz_op->MoveChildExpressions() );
	
	// 2nd copy of TZ: make new, localised, update command and add to sequence
	auto pfz_op_no_children = make_shared<PopulateFreeZoneOperator>( pfz_op->GetZone() ); // No children leaves terminii exposed, sort of.
	auto incremental_command = make_shared<UpdateTreeCommand>( inverted_tree_zone, pfz_op_no_children );	
	incremental_seq->Add(incremental_command);
}
