#include "tz_ordering.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

//#define DEBUG

using namespace SR;                   

// ------------------------- TreeZoneOrderingHandler --------------------------

TreeZoneOrderingHandler::TreeZoneOrderingHandler(XTreeDatabase *db_) :
    db( db_ ),
    dfr( db )
{
}
    

void TreeZoneOrderingHandler::Run( shared_ptr<Patch> &layout )
{
    out_of_order_patch_ptrs.clear();
    shared_ptr<Mutator> root = db->GetMainRootMutator();
    shared_ptr<Mutator> last = db->GetLastDescendantMutator(root);
    ConstrainAnyPatchToRange( layout, root, last, false );
    
    if( !out_of_order_patch_ptrs.empty() )
    {
        //FTRACE(out_of_order_patch_ptrs)("\n");
    }
    //FTRACE(layout)("\n");
    
    for( shared_ptr<Patch> *target_patch : out_of_order_patch_ptrs )
    {
        TRACE("patch: ")(*target_patch)("\n");
        // Get the tree zone
        auto target_tree_patch = dynamic_pointer_cast<TreeZonePatch>(*target_patch);
        ASSERT( target_tree_patch );
        MutableTreeZone *target_tree_zone = dynamic_cast<MutableTreeZone *>(target_tree_patch->GetZone());
        ASSERT( target_tree_zone );
        
        // Create the scaffold in a free zone
        auto free_zone = FreeZone::CreateScaffold( target_tree_zone->GetBaseXLink().GetTreePtrInterface(), 
                                                   target_tree_zone->GetNumTerminii() );
        //FTRACE("Scaffold free zone: ")(scaffold_fz)("\n");
        
        TRACE("target_tree_zone: ")(*target_tree_zone)("\nfree_zone: ")(*free_zone)("\n");
        // Put the scaffold into the "from" part of the tree, displacing 
        // the original contents, which we shall move
        db->MainTreeExchange( target_tree_zone, free_zone.get() );
        
        // free_zone is the part of the tree that we just displaced. Make 
        // a new patch based on it.
        auto free_patch = make_shared<FreeZonePatch>( move(free_zone), target_tree_patch->GetChildren() );
        
        // Install the new patch into the layout
        free_patch->AddEmbeddedMarkers( target_tree_patch->GetEmbeddedMarkers() );               
        *target_patch = free_patch;
        
        // How does the scaffold not end up in the updated tree?
        // The best argument is that, after this pass, none of the
        // scaffold nodes are inside any of the patches in our layout.
        // So, if subsequent passes and the DB act correctly, they 
        // will be deleted from the tree.                
    }
}


void TreeZoneOrderingHandler::Check( shared_ptr<Patch> &layout )
{
    shared_ptr<Mutator> root = db->GetMainRootMutator();
    shared_ptr<Mutator> last = db->GetLastDescendantMutator(root);
    ConstrainAnyPatchToRange( layout, root, last, true );
}


void TreeZoneOrderingHandler::ConstrainAnyPatchToRange( shared_ptr<Patch> &start_patch, 
													    shared_ptr<Mutator> range_front,
													    shared_ptr<Mutator> range_back,
													    bool just_check )
{
    INDENT(just_check?"c":"C");
    TRACE("Starting ")(just_check ? "cross-check" : "transfomation")(" at ")(start_patch)(" with range ")(range_front)(" to ")(range_back)(" inclusive\n");
    // Actions to take when we have a range. Use at root and for
    // terminii of tree zones.
    PatchRecords patch_records;
    AppendNextDescendantTreePatches( start_patch, patch_records );
    ConstrainTreePatchesToRange(patch_records, range_front, range_back, just_check);
}


void TreeZoneOrderingHandler::ConstrainTreePatchesToRange( PatchRecords &patch_records, 
                                                           shared_ptr<Mutator> range_front,
                                                           shared_ptr<Mutator> range_back,
                                                           bool just_check )
{                                                
    if( patch_records.empty() )
        return;
        
    // patch_records is updated in-place with correct out_of_range values
    FindOutOfOrderTreePatches( patch_records, range_front->GetXLink(), range_back->GetXLink(), just_check );    
    
    // Loop over patches, with their associated out-of-order flags
    PatchRecords next_descendant_tree_patches;
    PatchRecords::iterator prev_in_order_it = patch_records.end(); 
    PatchRecords::iterator prev_it = patch_records.begin(); 
    bool seen_in_order = false;
    bool first = true;
    for( PatchRecords::iterator it = patch_records.begin();
         first || prev_it != patch_records.end(); // gets us an extra iteration i.e. end-inclusive
         ++it )
    {
		bool last = (it == patch_records.end());
		
        if( last )
        {
			// Nothing
		}
        else if( it->out_of_order ) // out-of-order patch
        {
            // Mark as out of order so that the patch itself will be 
            // switched to a free zone patch.
            TRACE("Out of sequence: marking ")(it->patch_ptr)("\n");
            out_of_order_patch_ptrs.push_back(it->patch_ptr);

			// The tree-zone descendants of this patch still need to be checked for OOO.
			// Accumulate a list of patch records for them. 
            auto tree_patch = GetTreePatch(*it);
            tree_patch->ForChildren( [&](shared_ptr<Patch> &child_patch)
            {
                AppendNextDescendantTreePatches( child_patch, next_descendant_tree_patches );
            } );        
        }
        else // in-order patch
        {
			// In-order patch will remain a tree patch, so use the terminii to establish 
			// new ranges and recurse.
            auto tree_patch = GetTreePatch(*it);
            TRACE("Recursing on ")(tree_patch)("...\n");
            ConstrainChildrenToTerminii( tree_patch, just_check );
        }

        // Are we just past the end of an out-of-order run?
        if( !first && prev_it->out_of_order && (last || !it->out_of_order) )
        {
            // Book-end the descendants by the intersection of the supplied 
            // range and the range implied by the bases of the nearest in-order tree patches. 
            // We want to use the in-order ones because they will remain as tree zones.
            shared_ptr<Mutator> before_first_ooo = seen_in_order ? GetBaseMutator( *prev_in_order_it ) : range_front;                       
            shared_ptr<Mutator> after_last_ooo = last ? range_back : GetBaseMutator( *it );                 
                            
			// Use these to constrain the range for our descendants. 
			// Since the OOO patches will become free zones, we don't 
			// have a structural constraint but sill have the DF ordering
			// to satisfy so we can check the run together (weaker) 
			ConstrainTreePatchesToRange( next_descendant_tree_patches, 
										 before_first_ooo, 
										 after_last_ooo, 
										 just_check );
            next_descendant_tree_patches.clear();
        }

		if( !last && !it->out_of_order )
		{
            prev_in_order_it = it;
            seen_in_order = true;
		}
        prev_it = it;
        first = false;
    }
}
                                       

void TreeZoneOrderingHandler::ConstrainChildrenToTerminii( shared_ptr<TreeZonePatch> &tree_patch, 
                                                           bool just_check )
{
    // We have a tree zone. For each of its terminii, find the acceptable
    // range of descendent tree zones and recurse.
	auto mutable_tree_zone = dynamic_cast<MutableTreeZone *>(tree_patch->GetZone());
	ASSERT( mutable_tree_zone );
    size_t i=0;
    tree_patch->ForChildren( [&](shared_ptr<Patch> &child_patch)    
    {
        shared_ptr<Mutator> range_front = mutable_tree_zone->GetTerminusMutator(i++); // inclusive (terminus XLink equals base XLink of attached tree zone)
        shared_ptr<Mutator> range_back = db->GetLastDescendantMutator(range_front); // inclusive (is same or child of range_front)
        ConstrainAnyPatchToRange( child_patch, range_front, range_back, just_check );
    } );
}


void TreeZoneOrderingHandler::AppendNextDescendantTreePatches( shared_ptr<Patch> &start_patch, 
                                                           PatchRecords &patch_records )
{
    // Insert descendent tree zones, skipping over free zones, into a list for
    // convenience.
    if( auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(start_patch) )
    {
        patch_records.push_back( { &start_patch, false } );
    }
    else if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(start_patch) )
    {
        free_patch->ForChildren( [&](shared_ptr<Patch> &child_patch)
        {
            AppendNextDescendantTreePatches( child_patch, patch_records );
        } );
    }
    else
    {
        ASSERTFAIL();
    }
}                                                                     


void TreeZoneOrderingHandler::FindOutOfOrderTreePatches( PatchRecords &patch_records, 
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
        int prev_i_front=0, prev_i_back=0;
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
    return tree_patch->GetZone()->GetBaseXLink();
}


shared_ptr<Mutator> TreeZoneOrderingHandler::GetBaseMutator(const PatchRecord &patch_record) const
{
    shared_ptr<TreeZonePatch> tree_patch = GetTreePatch(patch_record);
	auto mutable_tree_zone = dynamic_cast<MutableTreeZone *>(tree_patch->GetZone());
	ASSERT( mutable_tree_zone );
    return mutable_tree_zone->GetBaseMutator();
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
        const TreeZone *tree_zone = tree_patch->GetZone();
        CheckXlink( tree_zone->GetBaseXLink(), base_equal_ok );
        
        // Co-loop over the chidren/terminii
        vector<XLink> terminii = tree_zone->GetTerminusXLinks();
        FreeZonePatch::ChildExpressionIterator it_child = tree_patch->GetChildrenBegin();        
        for( XLink terminus_xlink : terminii )
        {
            ASSERT( it_child != tree_patch->GetChildrenEnd() ); // length mismatch
            
            // Check this terminus start point is in order. Should not have seen the terminus before.
            CheckXlink( terminus_xlink, tree_zone->IsEmpty() );
            
            // Check everything under the corresponding child is in order. Could see terminus again.
            Worker( *it_child, true );
            
            // Check last xlink under terminus is in order. Could have seen last xlink
            // during recurse. 
            CheckXlink( XTreeDatabase::GetLastDescendantXLink(terminus_xlink), true );
            
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
            Worker( child_patch, free_patch->GetZone()->IsEmpty() );
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

