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
	out_of_order_list.clear();
	XLink root = db->GetMainRootXLink();
	XLink last = db->GetLastDescendant(root);
	RunForRange( root_expr, root, last, false );
	
	for( shared_ptr<ZoneExpression> *expr : out_of_order_list )
	{
		auto ptz_op = dynamic_pointer_cast<DupMergeTreeZoneOperator>(*expr);
		ASSERT( ptz_op );
		*expr = ptz_op->DuplicateToFree();
	}
}


void TreeZoneOrderingHandler::Check( shared_ptr<ZoneExpression> &root_expr )
{
	XLink root = db->GetMainRootXLink();
	XLink last = db->GetLastDescendant(root);
	RunForRange( root_expr, root, last, true );
}


void TreeZoneOrderingHandler::RunForTreeZone( shared_ptr<DupMergeTreeZoneOperator> &ptz_op, 
                                              bool just_check )
{
	// We have a tree zone. For each of its terminii, find the acceptable
	// range of descendent tree zones and recurse.
	TreeZone tree_zone = ptz_op->GetZone();
	TreeZone::TerminusIterator it_t = tree_zone.GetTerminiiBegin();
	ptz_op->ForChildren( [&](shared_ptr<ZoneExpression> &child_expr)	
	{
		XLink range_first = *it_t; // inclusive (terminus XLink equals base XLink of attached tree zone)
		XLink range_last = db->GetLastDescendant(range_first); // inclusive (is same or child of range_first)
		RunForRange( child_expr, range_first, range_last, just_check );
	} );
}


void TreeZoneOrderingHandler::RunForRange( shared_ptr<ZoneExpression> &base, 
								 	  	   XLink range_first,
							 			   XLink range_last,
						 				   bool just_check )
{
	INDENT(just_check?"c":"C");
	TRACE("Starting ")(just_check ? "cross-check" : "transfomation")(" at ")(base)(" with range ")(range_first)(" to ")(range_last)(" inclusive\n");
	// Actions to take when we have a range. Use at root and for
	// terminii of tree zones.
	ZoneExprPtrList tree_zone_op_list;
	InsertTZsBypassingFZs( base, tree_zone_op_list, tree_zone_op_list.end() );
	
	int prev_size;
	do
	{
		prev_size = tree_zone_op_list.size();
		
		OOOItList out_of_order_its;
		ZoneExprPtrList::iterator next_it;
		for( ZoneExprPtrList::iterator it = tree_zone_op_list.begin();
			 it != tree_zone_op_list.end();
			 it = next_it )
		{
			next_it = next(it);
			shared_ptr<ZoneExpression> *expr = *it;
			auto ptz_op = dynamic_pointer_cast<DupMergeTreeZoneOperator>(*expr);
			ASSERT( ptz_op ); // should succeed due InsertTZsBypassingFZs()
					
			// Check in range supplied to us for root or parent TZ terminus
			XLink tz_base = ptz_op->GetZone().GetBaseXLink();
			TRACE("Checking ")(tz_base)("...\n");
			Orderable::Diff diff_begin = dfr.Compare3Way(tz_base, range_first);
			Orderable::Diff diff_end = dfr.Compare3Way(tz_base, range_last);
			bool ok = diff_begin >= 0 && diff_end <= 0; // both inclusive
			if( !ok )
			{     
				if( just_check )
				{
					FTRACE(db->GetOrderings().depth_first_ordering)("\n");
					ASSERT(diff_begin >= 0)("Tree zone base ")(tz_base)(" appears before limit ")(range_first)(" in X tree");
					ASSERT(diff_end <= 0)("Tree zone base ")(tz_base)(" appears after limit ")(range_last)(" in X tree");
					ASSERTFAIL(); // we aint goin nowhere
				}
//#define NEW
#ifdef NEW
				out_of_order_its.push_back(it);
#else
				// The current TZ op is in the wrong place. It will be turned into
				// a free zone. Free zones are "invisible" in this scheme, so we update
				// our current work list to include its children (bypassing FZs).			
				ptz_op->ForChildren( [&](shared_ptr<ZoneExpression> &child_expr)
				{
					InsertTZsBypassingFZs( child_expr, tree_zone_op_list, next_it );
				} );
				next_it = tree_zone_op_list.erase(it);
#endif				
				// Duplicate the TZ that's in the wrong place into a free zone. 
				// Hopefully because we already dealt with children, we could
				// just mark this in side data.
				TRACE("Out of sequence: duplicating ")(tz_base)("\n");
				out_of_order_list.push_back(expr);
				
				continue; // no range threshold update or recurse
			}
					
			TRACE("OK, updating start of range\n");
			
			// Narrow the acceptable range for the next tree zone
			range_first = tz_base;		
		}	
		
#ifdef NEW
		for( ZoneExprPtrList::iterator it : out_of_order_its )
		{
			shared_ptr<ZoneExpression> *expr = *it;
			auto ptz_op = dynamic_pointer_cast<DupMergeTreeZoneOperator>(*expr);
			ASSERT( ptz_op ); // should succeed due InsertTZsBypassingFZs()

			// The current TZ op is in the wrong place. It will be turned into
			// a free zone. Free zones are "invisible" in this scheme, so we update
			// our current work list to include its children (bypassing FZs).			
			ZoneExprPtrList::iterator next_it = next(it);
			ptz_op->ForChildren( [&](shared_ptr<ZoneExpression> &child_expr)
			{
				InsertTZsBypassingFZs( child_expr, tree_zone_op_list, next_it );
			} );
			tree_zone_op_list.erase(it);
		}		
#endif
		
	} while(tree_zone_op_list.size() > prev_size);
	
	// Recurse to check descendents of the tree zones. 
	for( shared_ptr<ZoneExpression> *expr : tree_zone_op_list )
	{
		auto ptz_op = dynamic_pointer_cast<DupMergeTreeZoneOperator>(*expr);
		ASSERT( ptz_op ); // should succeed due InsertTZsBypassingFZs()

		TRACE("Recursing on ")(ptz_op)("...\n");
		RunForTreeZone( ptz_op, false );
	}	
}
                                       

void TreeZoneOrderingHandler::InsertTZsBypassingFZs( shared_ptr<ZoneExpression> &expr, 
              				  		                 list<shared_ptr<ZoneExpression> *> &tree_zones,
              				  		                 list<shared_ptr<ZoneExpression> *>::iterator pos )
{
	// Insert descendent tree zones, skipping over free zones, into a list for
	// convenience.
	if( auto ptz_op = dynamic_pointer_cast<DupMergeTreeZoneOperator>(expr) )
	{
		tree_zones.insert( pos, &expr );
	}
	else if( auto pfz_op = dynamic_pointer_cast<MergeFreeZoneOperator>(expr) )
	{
		pfz_op->ForChildren( [&](shared_ptr<ZoneExpression> &child_expr)
		{
			InsertTZsBypassingFZs( child_expr, tree_zones, pos );
		} );
	}
	else
	{
		ASSERTFAIL();
	}
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
	if( auto ptz_op = dynamic_pointer_cast<DupMergeTreeZoneOperator>(expr) )
	{
		INDENT(" T");
		// Got a TreeZone - check ordering of its base, strictness depending on who called us
		const TreeZone &tree_zone = ptz_op->GetZone();
		CheckXlink( tree_zone.GetBaseXLink(), base_equal_ok );
		
		// Co-loop over the chidren/terminii
		vector<XLink> terminii = tree_zone.GetTerminusXLinks();
		MergeFreeZoneOperator::ChildExpressionIterator it_child = ptz_op->GetChildrenBegin();		
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
	else if( auto pfz_op = dynamic_pointer_cast<MergeFreeZoneOperator>(expr) )
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

