#include "tz_ordering.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

// ------------------------- TreeZoneOrderingHandler --------------------------


TreeZoneOrderingHandler::TreeZoneOrderingHandler(const XTreeDatabase *db_) :
	db( db_ ),
	dfr( db )
{
}
	

void TreeZoneOrderingHandler::Run( shared_ptr<Patch> &layout )
{
	out_of_order_list.clear();
	XLink root = db->GetMainRootXLink();
	XLink last = db->GetLastDescendant(root);
	RunForRange( layout, root, last, false );
	
	if( !out_of_order_list.empty() )
	{
		//FTRACE(out_of_order_list)("\n");
		//FTRACE(layout)("\n");
	}
	
	for( shared_ptr<Patch> *patch : out_of_order_list )
	{
		auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(*patch);
		ASSERT( tree_patch );
		*patch = tree_patch->DuplicateToFree();
	}
}


void TreeZoneOrderingHandler::Check( shared_ptr<Patch> &layout )
{
	XLink root = db->GetMainRootXLink();
	XLink last = db->GetLastDescendant(root);
	RunForRange( layout, root, last, true );
}


void TreeZoneOrderingHandler::RunForTreeZone( shared_ptr<TreeZonePatch> &tree_patch, 
                                              bool just_check )
{
	// We have a tree zone. For each of its terminii, find the acceptable
	// range of descendent tree zones and recurse.
	TreeZone tree_zone = tree_patch->GetZone();
	TreeZone::TerminusIterator it_t = tree_zone.GetTerminiiBegin();
	tree_patch->ForChildren( [&](shared_ptr<Patch> &child_patch)	
	{
		XLink range_first = *it_t++; // inclusive (terminus XLink equals base XLink of attached tree zone)
		XLink range_last = db->GetLastDescendant(range_first); // inclusive (is same or child of range_first)
		RunForRange( child_patch, range_first, range_last, just_check );
	} );
}


void TreeZoneOrderingHandler::RunForRange( shared_ptr<Patch> &base, 
								 	  	   XLink range_first,
							 			   XLink range_last,
						 				   bool just_check )
{
	INDENT(just_check?"c":"C");
	TRACE("Starting ")(just_check ? "cross-check" : "transfomation")(" at ")(base)(" with range ")(range_first)(" to ")(range_last)(" inclusive\n");
	// Actions to take when we have a range. Use at root and for
	// terminii of tree zones.
	PatchRecords patch_records;
	AddTZsBypassingFZs( base, patch_records );
	RunForRangeList(patch_records, range_first, range_last, just_check);
}


void TreeZoneOrderingHandler::RunForRangeList( PatchRecords &patch_records, 
								 	  	       XLink range_first,
							 			       XLink range_last,
						 				       bool just_check )
{						 				       
	// patch_records is updated in-place with correct out_of_range values
	FindOutOfOrder( patch_records, range_first, range_last, just_check );	
	
	PatchRecords children_list;
	PatchRecords::iterator prev_in_order_it = patch_records.end(); // really "off the beginning"
	PatchRecords::iterator prev_it = patch_records.end(); // really "off the beginning"
	for( PatchRecords::iterator it = patch_records.begin();
		 true ;
		 ++it )
	{
		// End-inclusive body starts here
		if( (it == patch_records.end() || !it->out_of_order) && 
			(prev_it != patch_records.end() && prev_it->out_of_order) )
		{
			// End of an OOO run. Book-end the children by the intersection
			// of the supplied range and the range implied by the nearest 
			// in-order TZs
			XLink child_first = range_first;
			if( prev_in_order_it != patch_records.end() )
				child_first = GetBaseXLink( *prev_in_order_it );				  
			
			XLink child_last = range_last;
			if( it != patch_records.end() )
				child_last = GetBaseXLink( *it );				  
							
			RunForRangeList( children_list, 
							 child_first, 
							 child_last, 
							 just_check );
			children_list.clear();
		}

		// end-inclusive body ends here
		if( it==patch_records.end() )
			break;

		if( it->out_of_order )
		{
			auto tree_patch = GetTreePatch(*it);
			tree_patch->ForChildren( [&](shared_ptr<Patch> &child_patch)
			{
				AddTZsBypassingFZs( child_patch, children_list );
			} );		

			// Mark as out of order. 
			TRACE("Out of sequence: marking ")(it->patch_ptr)("\n");
			out_of_order_list.push_back(it->patch_ptr);
		}
		else
		{
			auto tree_patch = GetTreePatch(*it);
			TRACE("Recursing on ")(tree_patch)("...\n");
			RunForTreeZone( tree_patch, just_check );
			
			prev_in_order_it = it;
		}
		prev_it = it;
	}
}
                                       

void TreeZoneOrderingHandler::AddTZsBypassingFZs( shared_ptr<Patch> &patch, 
              				  		              PatchRecords &patch_records )
{
	// Insert descendent tree zones, skipping over free zones, into a list for
	// convenience.
	if( auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(patch) )
	{
		patch_records.push_back( { &patch, false } );
	}
	else if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(patch) )
	{
		free_patch->ForChildren( [&](shared_ptr<Patch> &child_patch)
		{
			AddTZsBypassingFZs( child_patch, patch_records );
		} );
	}
	else
	{
		ASSERTFAIL();
	}
}              						               				


void TreeZoneOrderingHandler::FindOutOfOrder( PatchRecords &patch_records, 
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
	
	// Get the patch records in depth-first order
	set<XLink, DepthFirstRelation> xlinks_dfo(dfr);
	vector<XLink> v;
	for( PatchRecord &patch_record : patch_records )
	{
		xlinks_dfo.insert( GetBaseXLink( patch_record ) );
		v.push_back( GetBaseXLink( patch_record ) );
	}

	// Find runs of in-order things, breaking on things outside the supplied range
	XLink prev_tz_base;      
	int i=0, run_start_i=0;
	bool first = true;			
	set<XLink, DepthFirstRelation>::iterator prev_tz_base_dfo_it;
	for( PatchRecord &patch_record : patch_records )
	{
		XLink tz_base = GetBaseXLink( patch_record );
		set<XLink, DepthFirstRelation>::iterator tz_base_dfo_it = xlinks_dfo.find(tz_base);
		//FTRACE(tz_base)("  ");

		// Check in range supplied to us for root or parent TZ terminus
		TRACE("Checking ")(tz_base)("...\n");
		Orderable::Diff diff_begin = dfr.Compare3Way(tz_base, range_first);
		Orderable::Diff diff_end = dfr.Compare3Way(tz_base, range_last);
		bool ok = diff_begin >= 0 && diff_end <= 0; // both inclusive
		patch_record.out_of_order = !ok;

		if( just_check && !ok )
		{
			//FTRACE(db->GetOrderings().depth_first_ordering)("\n");
			ASSERT(diff_begin >= 0)("Tree zone base ")(tz_base)(" appears before limit ")(range_first)(" in X tree");
			ASSERT(diff_end <= 0)("Tree zone base ")(tz_base)(" appears after limit ")(range_last)(" in X tree");
			ASSERTFAIL(); // we aint goin nowhere
		}			
		
		if( ok )
		{
			bool consecutive = first || next(prev_tz_base_dfo_it) == tz_base_dfo_it;
			if( just_check && !consecutive )
			{
				FTRACE("AS SUPPLIED\n")(v)("\nORDERED DEPTH FIRST\n")(xlinks_dfo)("\nWERE AT: ")(tz_base)("\n");
				ASSERT(diff_begin >= 0)("Tree zone base ")(tz_base)(" appears before limit ")(range_first)(" in X tree");
				ASSERT(diff_end <= 0)("Tree zone base ")(tz_base)(" appears after limit ")(range_last)(" in X tree");
				ASSERTFAIL(); // we aint goin nowhere
			}						
			
			// Completed run if:
			// - seen a node since startup and not broken run, and
			// - not consecutive wrt DF ordering
			if( !first && !consecutive )
			{
				int length = i - run_start_i;
				runs_by_length.insert( make_pair(length, run_start_i) );
				runs_by_first_index.insert( make_pair(run_start_i, length) );
			}
			
			// Need new run if:
			// - starting up, or
			// - run broken by going out of supplied range, or
			// - not consecutive wrt DF ordering
			if( first || !consecutive )
				run_start_i = i; // start new run here
						
			prev_tz_base = tz_base;
			prev_tz_base_dfo_it = tz_base_dfo_it;
			first = false;
		}
		else
		{
			first = true;
		}	
		i++;		
	}
	
	// Completed final run if:
	// - seen a node since startup and not broken run
	if(!first)
	{
		int length = i - run_start_i;
		runs_by_length.insert( make_pair(length, run_start_i) );
		runs_by_first_index.insert( make_pair(run_start_i, length) );
	}

	//FTRACE("Runs by length ")(runs_by_length)("\n");

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
		
		// Mark patch records in this run as out of order and remove from runs info
		//FTRACE("Run of %d OOO\n", l_ooo);
		for( int i=i_ooo_first; i<i_ooo_first+l_ooo; i++ )
		{
			patch_records[i].out_of_order = true;
			//FTRACE(i)(" ")(patch_records[i].patch_ptr)("\n");
		}
		runs_by_length.erase(make_pair(l_ooo, i_ooo_first)); 
		runs_by_first_index.erase(i_ooo_first);

		// Maybe we can concatenate the two neighbouring runs now?
		if( it_rbfi_next==runs_by_first_index.begin() || it_rbfi_next==runs_by_first_index.end() )
			continue; // Don't concatenate: was the first or last run
		
		// Deduce stuff about the first thing in the next indexed run		
		int i_next_first = it_rbfi_next->first;
		XLink tz_base_next_first = GetBaseXLink( patch_records[i_next_first] );
		
		// Deduce stuff about the last thing in the previous indexed run		
		auto it_rbfi_prev = prev(it_rbfi_next);
		int i_prev_last = it_rbfi_prev->first + it_rbfi_prev->second - 1;
		XLink tz_base_prev_last = GetBaseXLink( patch_records[i_prev_last] );
		
		//FTRACE("i_nf=%d i_pl=%d ", i_next_first, i_prev_last)(tz_base_next_first)(" ")(tz_base_prev_last)("\n");
		// Check that the two neighbouring runs would form a run if concatenated
		set<XLink, DepthFirstRelation>::iterator tz_base_prev_last_dfo_it = xlinks_dfo.find(tz_base_prev_last);
		set<XLink, DepthFirstRelation>::iterator tz_base_next_first_dfo_it = xlinks_dfo.find(tz_base_next_first);
		if( next(tz_base_prev_last_dfo_it) != tz_base_next_first_dfo_it )
			continue; // Don't concatenate: would not be consecutive

		// Deduce stuff about the new concatenated run
		int i_concatenated_first = it_rbfi_prev->first;
		int l_concatenated = it_rbfi_prev->second + it_rbfi_next->second + l_ooo; // Remember that ooo is still there in the array!
			
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


shared_ptr<TreeZonePatch> TreeZoneOrderingHandler::GetTreePatch(const PatchRecord &patch_record) const
{
	shared_ptr<Patch> *patch = patch_record.patch_ptr;
	auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(*patch);
	ASSERT( tree_patch ); // Things should only be tree pointer ops
	return tree_patch;
}


XLink TreeZoneOrderingHandler::GetBaseXLink(const PatchRecord &patch_record) const
{
	shared_ptr<TreeZonePatch> tree_patch = GetTreePatch(patch_record);
	return tree_patch->GetZone().GetBaseXLink();
}

// ------------------------- AltTreeZoneOrderingChecker --------------------------

// Different algo from that in TreeZoneOrderingHandler, just for checking

AltTreeZoneOrderingChecker::AltTreeZoneOrderingChecker(const XTreeDatabase *db_) :
	db( db_ ),
	dfr( db )
{
}
	

void AltTreeZoneOrderingChecker::Check( shared_ptr<Patch> layout )
{
	prev_xlink = XLink();
	Worker(layout, true);
}


void AltTreeZoneOrderingChecker::Worker( shared_ptr<Patch> patch, bool base_equal_ok )
{
	if( auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(patch) )
	{
		INDENT(" T");
		// Got a TreeZone - check ordering of its base, strictness depending on who called us
		const TreeZone &tree_zone = tree_patch->GetZone();
		CheckXlink( tree_zone.GetBaseXLink(), base_equal_ok );
		
		// Co-loop over the chidren/terminii
		vector<XLink> terminii = tree_zone.GetTerminusXLinks();
		FreeZonePatch::ChildExpressionIterator it_child = tree_patch->GetChildrenBegin();		
		for( XLink terminus_xlink : terminii )
		{
			ASSERT( it_child != tree_patch->GetChildrenEnd() ); // length mismatch
			
			// Check this terminus start point is in order. Should not have seen the terminus before.
			CheckXlink( terminus_xlink, false );
			
			// Check everything under the corresponding child is in order. Could see terminus again.
			Worker( *it_child, true );
			
			// Check last xlink under terminus is in order. Could have seen last xlink
			// during recurse. 
			CheckXlink( db->GetLastDescendant(terminus_xlink), true );
			
			++it_child;
		}
		ASSERT( it_child == tree_patch->GetChildrenEnd() ); // length mismatch
	}
	else if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(patch) )
	{
		INDENT(" F");
		free_patch->ForChildren( [&](shared_ptr<Patch> &child_patch)
		{
		    // Got a FreeZone - recurse looking for tree zones to check. But this FZ
		    // provides "padding" so do not expect to see terminus again.
			Worker( child_patch, false );
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

