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
		XLink range_first = *it_t++; // inclusive (terminus XLink equals base XLink of attached tree zone)
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
	ThingVector things;
	AddTZsBypassingFZs( base, things );
	RunForRangeList(things, range_first, range_last, just_check);
}


void TreeZoneOrderingHandler::RunForRangeList( ThingVector &things, 
								 	  	       XLink range_first,
							 			       XLink range_last,
						 				       bool just_check )
{						 				       
	// things is updated in-place with correct out_of_range values
	FindOutOfOrder( things, range_first, range_last, just_check );	
	
	ThingVector children_list;
	ThingVector::iterator prev_in_order_it = things.end(); // really "off the beginning"
	ThingVector::iterator prev_it = things.end(); // really "off the beginning"
	for( ThingVector::iterator it = things.begin();
		 true ;
		 ++it )
	{
		// End-inclusive body starts here
		if( (it == things.end() || !it->out_of_order) && 
			(prev_it != things.end() && prev_it->out_of_order) )
		{
			// End of an OOO run. Book-end the children by the intersection
			// of the supplied range and the range implied by the nearest 
			// in-order TZs
			XLink child_first = range_first;
			if( prev_in_order_it != things.end() )
				child_first = GetBaseXLink( *prev_in_order_it );				  
			
			XLink child_last = range_last;
			if( it != things.end() )
				child_last = GetBaseXLink( *it );				  
							
			RunForRangeList( children_list, 
							 child_first, 
							 child_last, 
							 just_check );
			children_list.clear();
		}

		// end-inclusive body ends here
		if( it==things.end() )
			break;

		if( it->out_of_order )
		{
			auto ptz_op = GetOperator(*it);
			ptz_op->ForChildren( [&](shared_ptr<ZoneExpression> &child_expr)
			{
				AddTZsBypassingFZs( child_expr, children_list );
			} );		

			// Mark as out of order. 
			TRACE("Out of sequence: marking ")(it->expr_ptr)("\n");
			out_of_order_list.push_back(it->expr_ptr);
		}
		else
		{
			auto ptz_op = GetOperator(*it);
			TRACE("Recursing on ")(ptz_op)("...\n");
			RunForTreeZone( ptz_op, just_check );
			
			prev_in_order_it = it;
		}
		prev_it = it;
	}
}
                                       

void TreeZoneOrderingHandler::AddTZsBypassingFZs( shared_ptr<ZoneExpression> &expr, 
              				  		              ThingVector &tree_zones )
{
	// Insert descendent tree zones, skipping over free zones, into a list for
	// convenience.
	if( auto ptz_op = dynamic_pointer_cast<DupMergeTreeZoneOperator>(expr) )
	{
		tree_zones.push_back( { &expr, false } );
	}
	else if( auto pfz_op = dynamic_pointer_cast<MergeFreeZoneOperator>(expr) )
	{
		pfz_op->ForChildren( [&](shared_ptr<ZoneExpression> &child_expr)
		{
			AddTZsBypassingFZs( child_expr, tree_zones );
		} );
	}
	else
	{
		ASSERTFAIL();
	}
}              						               				


void TreeZoneOrderingHandler::FindOutOfOrder( ThingVector &things, 
											  XLink range_first,
											  XLink range_last,
											  bool just_check )
{						 				       
	// runs of things that are monotonic wrt DF ordering
	
	// set of pairs: run length to start index. Will be ordered:
	// - primarily by run length
	// - secondarily by index into vector (via preserved insertion order)
	// Note: multimap makes it hard to remove a single element if you don't
	// already have the required iterator.
	set<pair<int, int>> runs_by_length; 
	
	// Map from starts to run lengths
	map<int, int> runs_by_first_index;
	
	// Find runs of in-order things, breaking on things outside the supplied range
	XLink prev_tz_base;      
	int i=0, run_start_i=0;
	for( Thing &thing : things )
	{
		XLink tz_base = GetBaseXLink( thing );

		// Check in range supplied to us for root or parent TZ terminus
		TRACE("Checking ")(tz_base)("...\n");
		Orderable::Diff diff_begin = dfr.Compare3Way(tz_base, range_first);
		Orderable::Diff diff_end = dfr.Compare3Way(tz_base, range_last);
		bool ok = diff_begin >= 0 && diff_end <= 0; // both inclusive
		thing.out_of_order = !ok;

		if( just_check && !ok )
		{
			FTRACE(db->GetOrderings().depth_first_ordering)("\n");
			ASSERT(diff_begin >= 0)("Tree zone base ")(tz_base)(" appears before limit ")(range_first)(" in X tree");
			ASSERT(diff_end <= 0)("Tree zone base ")(tz_base)(" appears after limit ")(range_last)(" in X tree");
			ASSERTFAIL(); // we aint goin nowhere
		}			
		
		if( ok )
		{
			bool monotonic = prev_tz_base ? dfr.Compare3Way(tz_base, prev_tz_base) >= 0 : true;
			if( just_check && !monotonic )
			{
				FTRACE(db->GetOrderings().depth_first_ordering)("\n");
				ASSERT(diff_begin >= 0)("Tree zone base ")(tz_base)(" appears before limit ")(range_first)(" in X tree");
				ASSERT(diff_end <= 0)("Tree zone base ")(tz_base)(" appears after limit ")(range_last)(" in X tree");
				ASSERTFAIL(); // we aint goin nowhere
			}						
			
			// Completed run if:
			// - seen a node since startup and not broken run, and
			// - not monotonic wrt DF ordering
			if( prev_tz_base && !monotonic )
			{
				int length = i - run_start_i;
				runs_by_length.insert( make_pair(length, run_start_i) );
				runs_by_first_index.insert( make_pair(run_start_i, length) );
			}
			
			// Need new run if:
			// - starting up, or
			// - run broken by going out of supplied range, or
			// - not monotonic wrt DF ordering
			if( !prev_tz_base || !monotonic )
				run_start_i = i; // start new run here
						
			prev_tz_base = tz_base;
		}
		else
		{
			prev_tz_base = XLink();
		}	
		i++;		
	}
	
	// Completed final run if:
	// - seen a node since startup and not broken run
	if(prev_tz_base)
	{
		int length = i - run_start_i;
		runs_by_length.insert( make_pair(length, run_start_i) );
		runs_by_first_index.insert( make_pair(run_start_i, length) );
	}

	// Solve ordering problem
	while(runs_by_length.size() > 1)
	{
		// We have more than one run, which means the things are not ordered.
		// Policy is to define the shortest run as out-of-order.
		// We will remove it and maybe concatenate the neighbouring runs.
		// This reduces the number of runs, so should terminate.
		auto it_rbl_shortest = runs_by_length.begin(); // (joint) smallest run
		auto it_rbl_ooo = it_rbl_shortest; // policy		
		
		// Find out some stuff about the run, and the next one in index order
		int l_ooo, i_ooo_first; 
		tie(l_ooo, i_ooo_first) = *it_rbl_ooo; 
		auto it_rbfi_ooo = runs_by_first_index.find(i_ooo_first);
		auto it_rbfi_next = next(it_rbfi_ooo);
		
		// Mark thnigs in this run as out of order and remove from runs info
		for( int i=i_ooo_first; i<i_ooo_first+l_ooo; i++ )
			things[i].out_of_order = true;
		runs_by_length.erase(make_pair(l_ooo, i_ooo_first)); 
		runs_by_first_index.erase(i_ooo_first);

		// Maybe we can concatenate the two neighbouring runs now?
		if( it_rbfi_next==runs_by_first_index.begin() || it_rbfi_next==runs_by_first_index.end() )
			continue; // Don't concatenate: was the first or last run
		
		// Deduce stuff about the first thing in the next indexed run		
		int i_next_first = it_rbfi_next->first;
		XLink tz_base_next_first = GetBaseXLink( things[i_next_first] );
		
		// Deduce stuff about the last thing in the previous indexed run		
		auto it_rbfi_prev = prev(it_rbfi_next);
		int i_prev_last = it_rbfi_prev->first + it_rbfi_prev->second - 1;
		XLink tz_base_prev_last = GetBaseXLink( things[i_prev_last] );
		
		// Check that the two neighbouring runs would form a run if concatenated
		if( dfr.Compare3Way(tz_base_next_first, tz_base_prev_last) < 0 )
			continue; // Don't concatenate: would not be monotonic

		// Deduce stuff about the new concatenated run
		int i_concatenated_first = it_rbfi_prev->first;
		int l_concatenated = it_rbfi_prev->second + it_rbfi_next->second;
			
		// Drop the original neighbouring runs
		runs_by_length.erase(make_pair(it_rbfi_next->second, it_rbfi_next->first)); 
		runs_by_first_index.erase(it_rbfi_next);
		runs_by_length.erase(make_pair(it_rbfi_prev->second, it_rbfi_prev->first)); 
		runs_by_first_index.erase(it_rbfi_prev);
		
		// Insert concatenated run
		runs_by_length.insert( make_pair(l_concatenated, i_concatenated_first) );
		runs_by_first_index.insert( make_pair(i_concatenated_first, l_concatenated) );
	}
}


shared_ptr<DupMergeTreeZoneOperator> TreeZoneOrderingHandler::GetOperator(const Thing &thing) const
{
	shared_ptr<ZoneExpression> *expr = thing.expr_ptr;
	auto ptz_op = dynamic_pointer_cast<DupMergeTreeZoneOperator>(*expr);
	ASSERT( ptz_op ); // Things should only be tree pointer ops
	return ptz_op;
}


XLink TreeZoneOrderingHandler::GetBaseXLink(const Thing &thing) const
{
	shared_ptr<DupMergeTreeZoneOperator> ptz_op = GetOperator(thing);
	return ptz_op->GetZone().GetBaseXLink();
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

