#include "tz_ordering.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "zone_commands.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

// ------------------------- TreeZoneOrderingHandler --------------------------


TreeZoneOrderingHandler::TreeZoneOrderingHandler(const XTreeDatabase *db_) :
	db( db_ ),
	dfr( db )
{
}
	

void TreeZoneOrderingHandler::Run( shared_ptr<ZoneExpression> &root_expr )
{
	XLink root = db->GetMainRootXLink();
	XLink last = db->GetLastDescendant(root);
	CheckRange( root_expr, root, last, false );
}


void TreeZoneOrderingHandler::Check( shared_ptr<ZoneExpression> &root_expr )
{
	XLink root = db->GetMainRootXLink();
	XLink last = db->GetLastDescendant(root);
	CheckRange( root_expr, root, last, true );
}


void TreeZoneOrderingHandler::CheckRange( shared_ptr<ZoneExpression> &base, 
									  	  XLink range_begin,
										  XLink range_end,
										  bool just_check )
{
	TRACE("CheckRange() at ")(base)(" with range ")(range_begin)(" to ")(range_end)("\n");
	// Actions to take when we have a range. Use at root and for
	// terminii of tree zones.
	list<shared_ptr<ZoneExpression> *> tree_zone_op_list;
	GatherTreeZoneOps( base, tree_zone_op_list );
	
	for( shared_ptr<ZoneExpression> *expr : tree_zone_op_list )
	{
		auto ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(*expr);
		ASSERT( ptz_op ); // should succeed due GatherTreeZoneOps()
		XLink tz_base = ptz_op->GetZone().GetBaseXLink();
		
		// Check in range supplied to us for root or parent TZ terminus
		Orderable::Diff diff_begin = dfr.Compare3Way(tz_base, range_begin);
		Orderable::Diff diff_end = dfr.Compare3Way(tz_base, range_end);
		bool ok = diff_begin >= 0 && diff_end <= 0; // both inclusive
		if( !ok )
		{     
			if( just_check )
			{
				FTRACE(db->GetOrderings().depth_first_ordering)("\n");
				ASSERT(diff_begin >= 0)("Tree zone base ")(tz_base)(" appears before limit ")(range_begin)(" in X tree");
				ASSERT(diff_end <= 0)("Tree zone base ")(tz_base)(" appears after limit ")(range_end)(" in X tree");
			}
			else
			{
				// Action: minimal is to duplicate ptz_op->GetZone() and
				// every TZ under it into free zones. Could be improved later 
				TRACE("Duplicating ")(tz_base)("\n");
				DuplicateTreeZone( *expr );
				continue; // no range threshold update or recurse
			}
		}
		TRACE(tz_base)(" OK\n");
		
	    // Narrow the acceptable range for the next tree zone
		range_begin = tz_base;

		// Recurse to check descendents of the tree zone
		RunForTreeZone( ptz_op, false );		
	}
}


void TreeZoneOrderingHandler::RunForTreeZone( shared_ptr<PopulateTreeZoneOperator> &ptz_op, 
                                              bool just_check )
{
	TRACE("RunForTreeZone() looking for free zones under ")(ptz_op)("\n");
	
	// We have a tree zone. For each of its terminii, find the acceptable
	// range of descendent tree zones and recurse.
	TreeZone tree_zone = ptz_op->GetZone();
	TreeZone::TerminusIterator it_t = tree_zone.GetTerminiiBegin();
	ptz_op->ForChildren( [&](shared_ptr<ZoneExpression> &child_expr)	
	{
		XLink range_begin = *it_t; // inclusive (terminus XLink equals base XLink of attached tree zone)
		XLink range_end = db->GetLastDescendant(range_begin); // inclusive (is same or child of range_begin)
		CheckRange( child_expr, range_begin, range_end, just_check );
	} );
}
                                       

void TreeZoneOrderingHandler::GatherTreeZoneOps( shared_ptr<ZoneExpression> &expr, 
              				  		             list<shared_ptr<ZoneExpression> *> &tree_zones )
{
	// Get descendant tree zones, skipping over free zones, into a list for
	// convenience.
	if( auto ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(expr) )
	{
		tree_zones.push_back( &expr );
	}
	else if( auto pfz_op = dynamic_pointer_cast<PopulateFreeZoneOperator>(expr) )
	{
		pfz_op->ForChildren( [&](shared_ptr<ZoneExpression> &child_expr)
		{
			GatherTreeZoneOps( child_expr, tree_zones );
		} );
	}
	else
	{
		ASSERTFAIL();
	}
}
              						  
              						  
void TreeZoneOrderingHandler::DuplicateTreeZone( shared_ptr<ZoneExpression> &expr )
{
	// At present, for simplest algo, we DO recurse the entire subtree and 
	// duplicate everything. But it shouldn't be too hard to just duplicate the
	// supplied one and let checks continue on the children of the bad TZ. Would
	// require CheckRange() to be stated-out so it can be invoked from a walk though.
	ZoneExpression::ForDepthFirstWalk( expr, nullptr, [&](shared_ptr<ZoneExpression> &child_expr)
	{
		if( auto ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(child_expr) )
		{		
			TRACE("Duplicate ")(ptz_op)("\n");
			child_expr = ptz_op->DuplicateToFree(db);
		}
	} );	
}

// ------------------------- AltTreeZoneOrderingChecker --------------------------

// Different algo from that in TreeZoneOrderingHandler, just for checking

AltTreeZoneOrderingChecker::AltTreeZoneOrderingChecker(const XTreeDatabase *db_) :
	db( db_ ),
	dfr( db )
{
}
	

void AltTreeZoneOrderingChecker::Check( shared_ptr<ZoneExpression> root_expr )
{
	prev_xlink = XLink();
	Worker(root_expr, true);
}


void AltTreeZoneOrderingChecker::Worker( shared_ptr<ZoneExpression> expr, bool base_equal_ok )
{
	if( auto ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(expr) )
	{
		INDENT(" T");
		// Got a TreeZone - check ordering of its base, strictness depending on who called us
		const TreeZone &tree_zone = ptz_op->GetZone();
		CheckXlink( tree_zone.GetBaseXLink(), base_equal_ok );
		
		// Co-loop over the chidren/terminii
		vector<XLink> terminii = tree_zone.GetTerminusXLinks();
		PopulateFreeZoneOperator::ChildExpressionIterator it_child = ptz_op->GetChildrenBegin();		
		for( XLink terminus_xlink : terminii )
		{
			ASSERT( it_child != ptz_op->GetChildrenEnd() ); // length mismatch
			
			// Check this terminus start point is in order. Should not have seen the terminus before.
			CheckXlink( terminus_xlink, false );
			
			// Check everything under the corresponding child is in order. Could see terminus again.
			Worker( *it_child, true );
			
			// Check last xlink under terminus is in order. Could have seen last xlink
			// during recurse. 
			CheckXlink( db->GetLastDescendant(terminus_xlink), true );
			
			++it_child;
		}
		ASSERT( it_child == ptz_op->GetChildrenEnd() ); // length mismatch
	}
	else if( auto pfz_op = dynamic_pointer_cast<PopulateFreeZoneOperator>(expr) )
	{
		INDENT(" F");
		pfz_op->ForChildren( [&](shared_ptr<ZoneExpression> &child_expr)
		{
		    // Got a FreeZone - recurse looking for tree zones to check. But this FZ
		    // provides "padding" so do not expect to see terminus again.
			Worker( child_expr, false );
		} );
	}
	else
	{
		ASSERTFAIL();
	}
}


void AltTreeZoneOrderingChecker::CheckXlink( XLink x, bool equal_ok )
{	
	if( prev_xlink )
	{
		TRACE("Check ")(x)(equal_ok?" non-strict":" strict")("\n");
		if( equal_ok )
			ASSERT( dfr.Compare3Way( prev_xlink, x ) <= 0 );
		else
			ASSERT( dfr.Compare3Way( prev_xlink, x ) < 0 );
	}
	
	prev_xlink = x;
}

