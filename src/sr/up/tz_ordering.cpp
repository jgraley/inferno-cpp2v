#include "tz_ordering.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

//#define NEW_OOO

using namespace SR;                   

// ------------------------- TreeZoneOrderingHandler --------------------------

TreeZoneOrderingHandler::TreeZoneOrderingHandler(XTreeDatabase *db_) :
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
	}
	//FTRACE(layout)("\n");
	
	for( shared_ptr<Patch> *patch : out_of_order_list )
	{
		// Get the tree zone
		auto from_patch = dynamic_pointer_cast<TreeZonePatch>(*patch);
		ASSERT( from_patch );
		TreeZone from_tz = from_patch->GetZone();
		
		// Make a scaffold of the type required by the tree zone's base
		auto scaffold_pair = from_tz.GetBaseXLink().GetTreePtrInterface()->MakeScaffold();		

#ifdef NEW_OOO
		// Loop over terminii, giving the scaffold children and creating
		// mutators on those chidren for the scaffold free zone
		list<shared_ptr<Mutator>> scaffold_terminii;
		for( XLink t : from_tz.GetTerminusXLinks() )
			scaffold_pair.second->push_back( TreePtr<Node>() );
			auto scaff_child_mut = make_shared<SingularMutator>(scaffold_pair.first, &scaffold_pair.second->back());
			scaffold_terminii.push_back( scaff_child_mut );
		
		// Create the free zone	containing the scaffolding
		FreeZone scaffold_fz( scaffold_pair.first, move(scaffold_terminii) );
		
		// Put the scaffold into the "from" part of the tree, and get back the original contents, which we shall move
		FreeZone moving_fz = db->MainTreeExchange( from_tz, scaffold_fz );
		
		// Make a new patch based on this moving free zone
		*patch = make_shared<FreeZonePatch>( moving_fz, from_patch->GetChildren() );
#else
		*patch = from_patch->DuplicateToFree();
#endif
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
		XLink range_front = *it_t++; // inclusive (terminus XLink equals base XLink of attached tree zone)
		XLink range_back = db->GetLastDescendant(range_front); // inclusive (is same or child of range_front)
		RunForRange( child_patch, range_front, range_back, just_check );
	} );
}


void TreeZoneOrderingHandler::RunForRange( shared_ptr<Patch> &base, 
								 	  	   XLink range_front,
							 			   XLink range_back,
						 				   bool just_check )
{
	INDENT(just_check?"c":"C");
	TRACE("Starting ")(just_check ? "cross-check" : "transfomation")(" at ")(base)(" with range ")(range_front)(" to ")(range_back)(" inclusive\n");
	// Actions to take when we have a range. Use at root and for
	// terminii of tree zones.
	PatchRecords patch_records;
	AddTZsBypassingFZs( base, patch_records );
	RunForRangeList(patch_records, range_front, range_back, just_check);
}


void TreeZoneOrderingHandler::RunForRangeList( PatchRecords &patch_records, 
								 	  	       XLink range_front,
							 			       XLink range_back,
						 				       bool just_check )
{						 				       
	if( patch_records.empty() )
		return;
		
	// patch_records is updated in-place with correct out_of_range values
	FindOutOfOrder( patch_records, range_front, range_back, just_check );	
	
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
			XLink child_first = range_front;
			if( prev_in_order_it != patch_records.end() )
				child_first = GetBaseXLink( *prev_in_order_it );				  
			
			XLink child_last = range_back;
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
											  XLink range_front,
											  XLink range_back,
											  bool just_check )
{						 				       
	ASSERT( !patch_records.empty() );
	
	// Data structures for runs of things that are consecutive wrt DF ordering
	
	// set of pairs: run length to front index. Will be ordered:
	// - primarily by run length
	// - secondarily by index into vector (via preserved insertion order)
	// Note: multimap makes it hard to remove a single element if you don't
	// already have the required iterator.
	set<pair<int, int>> runs_by_length; 
	
	// Map from the fronts of runs to the run lengths
	map<int, int> runs_by_front_index; // TODO first->front etc
	
	// Get the patch records in depth-first order
	set<XLink, DepthFirstRelation> xlinks_dfo(dfr);
	vector<XLink> v;
	for( PatchRecord &patch_record : patch_records )
	{
		xlinks_dfo.insert( GetBaseXLink( patch_record ) );
		v.push_back( GetBaseXLink( patch_record ) );
	}
	
	//FTRACE("Depth-first order: ")(xlinks_dfo)("\n");

	// Find runs of patch records that are consecutive both in the layout and
	// the DF ordering, breaking on patch records outside the supplied overall range.
	XLink prev_tz_base;      
	int i=0, run_start_i=0;
	bool first = true;			
	set<XLink, DepthFirstRelation>::iterator prev_tz_base_dfo_it;
	for( PatchRecord &patch_record : patch_records )
	{
		XLink tz_base = GetBaseXLink( patch_record );
		set<XLink, DepthFirstRelation>::iterator tz_base_dfo_it = xlinks_dfo.find(tz_base);
		//FTRACE(tz_base)("  ");

		// Check the tree zone base is in overall range supplied to us for root or parent TZ terminus
		TRACE("Checking ")(tz_base)("...\n");
		Orderable::Diff diff_front = dfr.Compare3Way(tz_base, range_front);
		Orderable::Diff diff_back = dfr.Compare3Way(tz_base, range_back);
		bool ok = diff_front >= 0 && diff_back <= 0; // both inclusive		

		if( just_check && !ok )
		{
			//FTRACE(db->GetOrderings().depth_first_ordering)("\n");
			ASSERT(diff_front >= 0)("Tree zone base ")(tz_base)(" appears before limit ")(range_front)(" in X tree");
			ASSERT(diff_back <= 0)("Tree zone base ")(tz_base)(" appears after limit ")(range_back)(" in X tree");
			ASSERTFAIL(); // we aint goin nowhere
		}			
		
		if( ok )
		{
			// First patch record gets it for free, then we have to check the DF ordering
			bool consecutive = first || next(prev_tz_base_dfo_it) == tz_base_dfo_it;
			
			if( just_check && !consecutive )
			{
				FTRACE("AS SUPPLIED\n")(v)("\nORDERED DEPTH FIRST\n")(xlinks_dfo)("\nWERE AT: ")(tz_base)("\n");
				ASSERT(diff_front >= 0)("Tree zone base ")(tz_base)(" appears before limit ")(range_front)(" in X tree");
				ASSERT(diff_back <= 0)("Tree zone base ")(tz_base)(" appears after limit ")(range_back)(" in X tree");
				ASSERTFAIL(); // we aint goin nowhere
			}						
			
			// Completed run if:
			// - seen at least one patch record since start, and
			// - not consecutive wrt DF ordering
			if( !first && !consecutive )
			{
				int length = i - run_start_i;
				runs_by_length.insert( make_pair(length, run_start_i) );
				runs_by_front_index.insert( make_pair(run_start_i, length) );
			}
			
			// Need new run if:
			// - starting up, or
			// - not consecutive wrt DF ordering
			if( first || !consecutive )
				run_start_i = i; // start new run here
						
			prev_tz_base = tz_base;
			prev_tz_base_dfo_it = tz_base_dfo_it;
			first = false;
		}
		else
		{
			// Outside overall range. But we don't need to break up the 
			// current run: it's enough to remove from local DF ordering 
			// and set out_of_order flag.
			EraseSolo( xlinks_dfo, *tz_base_dfo_it );
			patch_record.out_of_order = true;
		}	
		i++;		
	}
	
	// Complete the final run.
	int length = i - run_start_i;
	runs_by_length.insert( make_pair(length, run_start_i) );
	runs_by_front_index.insert( make_pair(run_start_i, length) );

	// Solve ordering problem reductively
	while(runs_by_length.size() > 1)
	{
		//FTRACE("Runs by length ")(runs_by_length)("\n");
		// We have more than one run, which means the things are disordered.
		// Policy is to define the shortest run as out-of-order.
		// We will try to remove it and maybe concatenate the neighbouring runs,
		// as well as any consecutive runs that are now consecutive in the DF ordering.
		// This reduces the number of runs, so should terminate.
		auto it_rbl_shortest = runs_by_length.begin(); // (joint) smallest run
		auto it_rbl_ooo = it_rbl_shortest; // policy		
		
		// Find out some stuff about the run, and the next one in index order
		int l_ooo, i_ooo_front; 
		tie(l_ooo, i_ooo_front) = *it_rbl_ooo; 
		
		// Mark patch records in this run as out of order and remove from:
		// - local DF ordering
		// - runs data structures
		//FTRACE("Removing run of %d OOO:\n", l_ooo);
		for( int i=i_ooo_front; i<i_ooo_front+l_ooo; i++ )
		{
			XLink i_tz_base = GetBaseXLink( patch_records[i] );
			if( patch_records[i].out_of_order )
				ASSERT( xlinks_dfo.count( i_tz_base ) == 0 ); // should already be gone
			else
				EraseSolo( xlinks_dfo, i_tz_base ); // remove it exactly once
				
			patch_records[i].out_of_order = true;
			//FTRACE(i)(": ")(patch_records[i].patch_ptr)("\n");
		}
		EraseSolo( runs_by_length, make_pair(l_ooo, i_ooo_front) ); 
		EraseSolo( runs_by_front_index, i_ooo_front );

		// Look for concatenation opportunties
		// We could do two concatenations after a removal:
		// - where it was removed from (OOO location), and
		// - where it would have been according to DF ordering (now consecutive)
		//FTRACE("Looking for concatenations:\n", l_ooo);
		bool first = true;
		int prev_i_front, prev_i_back;
		for( map<const int, int>::iterator it = runs_by_front_index.begin();
		     it != runs_by_front_index.end();
		     ++it )
		{
			int i_front = it->first;
			int i_back = it->first + it->second - 1;

			if( !first )
			{
				//FTRACE("prev back: %d of %d, ", prev_i_back, patch_records.size());
				XLink prev_tz_base_back = GetBaseXLink( patch_records[prev_i_back] );
				set<XLink, DepthFirstRelation>::iterator prev_tz_base_back_dfo_it = xlinks_dfo.find(prev_tz_base_back);
				//FTRACE("front: %d of %d\n", i_front, patch_records.size());
				XLink tz_base_front = GetBaseXLink( patch_records[i_front] );
				set<XLink, DepthFirstRelation>::iterator tz_base_front_dfo_it = xlinks_dfo.find(tz_base_front);
						
				if( next(prev_tz_base_back_dfo_it) == tz_base_front_dfo_it ) // Does local DF ordering say we can concatenate?
				{
					//FTRACE("concatenating\n");
					// Deduce stuff about the new concatenated run
					int concatenated_i_front = prev_i_front;
					int concatenated_i_back  = i_back; 
						
					// Drop the original neighbouring runs
					runs_by_length.erase(make_pair(prev_i_back - prev_i_front + 1, prev_i_front)); 
					runs_by_front_index.erase(prev_i_front);
					runs_by_length.erase(make_pair(i_back - i_front + 1, i_front)); 
					runs_by_front_index.erase(i_front);
					
					// Insert concatenated run and pretend the concatenated one is the current one (becomes "prev" on next iteration)
					runs_by_length.insert( make_pair(concatenated_i_back - concatenated_i_front + 1, concatenated_i_front) );
					it = InsertSolo( runs_by_front_index, make_pair(concatenated_i_front, concatenated_i_back - concatenated_i_front + 1) );		
					i_front = concatenated_i_front;
					i_back = concatenated_i_back;
				}
			}
			
			prev_i_front = i_front;
			prev_i_back = i_back;
			first = false;
		}		
	}
}


shared_ptr<TreeZonePatch> TreeZoneOrderingHandler::GetTreePatch(const PatchRecord &patch_record) const
{
	shared_ptr<Patch> *patch = patch_record.patch_ptr;
	ASSERT( patch );
	ASSERT( *patch );
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
		INDENT("T");
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
			CheckXlink( terminus_xlink, tree_zone.IsEmpty() );
			
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
		INDENT("F");
		free_patch->ForChildren( [&](shared_ptr<Patch> &child_patch)
		{
		    // Got a FreeZone - recurse looking for tree zones to check. But this FZ
		    // provides "padding" so do not expect to see terminus again.
			Worker( child_patch, free_patch->GetZone().IsEmpty() );
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
			ASSERT( dfr.Compare3Way( prev_xlink, x ) <= 0 )("prev_xlink=")(prev_xlink)(" x=")(x);
		else
			ASSERT( dfr.Compare3Way( prev_xlink, x ) < 0 )("prev_xlink=")(prev_xlink)(" x=")(x);
	}
	
	prev_xlink = x;
}

