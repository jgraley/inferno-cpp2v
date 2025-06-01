#include "ordering_pass.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"
#include "misc_passes.hpp"
#include "scaffold_ops.hpp"

#include <iostream>

//#define DEBUG

// I think preferring to move the ancestor is better for OOO since there 
// are fewer of them (typically) and we're more likely to avoid moving 
// the leaf zones which could be big. It's also more symmetrical, because 
// we begin at next descendants of start patch, and on OOO detection, we move 
// to the next descendants of the OOO patches.
// PREFER_TO_MOVE_DESCENDANT may be necessary to reproduce issues like #874
//#define PREFER_TO_MOVE_DESCENDANT

namespace SR 
{
	class DFPatchIndexRelation
{
public: 
	typedef size_t KeyType;

    explicit DFPatchIndexRelation(const XTreeDatabase *db, const vector<OrderingPass::PatchRecord> &patch_records);

    /// Less operator: for use with set, map etc
    bool operator()( KeyType l_key, KeyType r_key ) const;
    Orderable::Diff Compare3Way( KeyType l_key, KeyType r_key ) const;
    pair<Orderable::Diff, DepthFirstRelation::RelType> CompareHierarchical( KeyType l_key, KeyType r_key ) const;
    
private:
    DepthFirstRelation df;
    const vector<OrderingPass::PatchRecord> &patch_records;
}; 
}

using namespace SR;                   


DFPatchIndexRelation::DFPatchIndexRelation(const XTreeDatabase *db, const vector<OrderingPass::PatchRecord> &patch_records_) :
    df( db ),
    patch_records( patch_records_ )
{
}    

/// Less operator: for use with set, map etc
bool DFPatchIndexRelation::operator()( KeyType l_key, KeyType r_key ) const
{
    return Compare3Way(l_key, r_key) < 0;
}

Orderable::Diff DFPatchIndexRelation::Compare3Way( KeyType l_key, KeyType r_key ) const
{
    return CompareHierarchical( l_key, r_key ).first;	
}


pair<Orderable::Diff, DepthFirstRelation::RelType> DFPatchIndexRelation::CompareHierarchical( KeyType l_key, KeyType r_key ) const
{
	shared_ptr<Patch> *l_pp = patch_records[l_key].patch_ptr;
    ASSERT( l_pp );
    ASSERT( *l_pp );
    auto l_tp = dynamic_pointer_cast<TreeZonePatch>(*l_pp);
    ASSERT( l_tp );
	XLink l_xlink = l_tp->GetZone()->GetBaseXLink();
	ASSERT( l_xlink );
	ASSERT( l_xlink.GetChildTreePtr() );
	
	shared_ptr<Patch> *r_pp = patch_records[r_key].patch_ptr;
    ASSERT( r_pp );
    ASSERT( *r_pp );
    auto r_tp = dynamic_pointer_cast<TreeZonePatch>(*r_pp);
    ASSERT( r_tp );
	XLink r_xlink = r_tp->GetZone()->GetBaseXLink();
	ASSERT( r_xlink );
	ASSERT( r_xlink.GetChildTreePtr() );

	return df.CompareHierarchical( l_xlink, r_xlink );
	ASSERTFAIL();
}


// ------------------------- OrderingPass --------------------------

OrderingPass::OrderingPass(XTreeDatabase *db_, ScaffoldOps *sops_) :
    db( db_ ),
    sops( sops_ ),
    dfr( db )
{
}
    

void OrderingPass::RunAnalysis( shared_ptr<Patch> &layout )
{    
	INDENT("A");
	out_of_order_patches.clear();
	in_order_bases.clear();
	
    XLink root = db->GetMainRootXLink();
    ConstrainAnyPatchToDescendants( layout, root, false );	
}


void OrderingPass::Check( shared_ptr<Patch> &layout )
{
	out_of_order_patches.clear();
	in_order_bases.clear();
	
	XLink root = db->GetMainRootXLink();
    ConstrainAnyPatchToDescendants( layout, root, true );
}


void OrderingPass::ConstrainAnyPatchToDescendants( shared_ptr<Patch> &start_patch, 
												   XLink base,
												   bool just_check )
{
    INDENT(just_check?"a":"A");
    TRACE("Starting ")(just_check ? "cross-check" : "transfomation")(" at ")(start_patch)(" with ancestor ")(base)("\n");
    
    // Determine tree zone patches descending from starting patch.
    PatchRecords patch_records;
    AppendNextDescendantTreePatches( start_patch, patch_records );
                 
    // Keep trying to find the in-order subset of patches until we settle.
    // Settling means for every OOO patch, we've tried going to the 
    // descendant patches (bypassing FZ) and either there aren't any
    // or they are in-order.
	while(true)
	{
		// No patches, or all are OOO
		if( patch_records.empty() )
			break;
        
		// patch_records is updated in-place with correct out_of_range values
		FindOutOfOrderTreePatches( patch_records, base, just_check );    
		
		bool more_to_check = false;
		
		// Loop over patches, with their associated out-of-order flags
		PatchRecords next_patch_records;
		for( const PatchRecord &patch_record : patch_records )
		{
			if( patch_record.out_of_order ) // out-of-order patch
			{
				// The tree-zone descendants of this patch still need to be checked for OOO.
				// Accumulate a list of patch records for them. 
				auto tree_patch = GetTreePatch(patch_record);
				TRACE("Out-of-order patch: ")(tree_patch)("\nso gathering first descendants...\n");
				size_t size_before = next_patch_records.size();
				Patch::ForChildren( tree_patch, [&](shared_ptr<Patch> &child_patch)
				{
					AppendNextDescendantTreePatches( child_patch, next_patch_records );
				} );        
				size_t size_after = next_patch_records.size();
				more_to_check = more_to_check || (size_after > size_before);

				// Mark as out of order so that the patch itself will be 
				// switched to a free zone patch.
				out_of_order_patches.push_back(patch_record.patch_ptr);
			}
			else // in-order patch
			{          
				next_patch_records.push_back(patch_record);
			}
		}
		
		// We've settled
		if( !more_to_check )
			break;
		
		patch_records = next_patch_records;
	}
    
    // Recurse to check our successes
    for( const PatchRecord &patch_record : patch_records )
    {
		if( !patch_record.out_of_order )
		{
            auto tree_patch = GetTreePatch(patch_record);

            // InsertSolo is used here because we should only find each TZ as being in order once
            //TRACE("In-order: index=%d, patch=", i++)(tree_patch)("\nset is:\n")(in_order_bases)("\n");
            InsertSolo(in_order_bases, tree_patch->GetZone()->GetBaseXLink());

			// In-order patch will remain a tree patch, so use the terminii to establish 
			// new ranges and recurse.
            //TRACE("In-order patch, so big recursion on ")(tree_patch)("...\n");
            ConstrainChildrenToTerminii( tree_patch, just_check );
   		}
	}
}
                                       

void OrderingPass::ConstrainChildrenToTerminii( shared_ptr<TreeZonePatch> &tree_patch, 
                                                bool just_check )
{
	INDENT(just_check?"t":"T");
	
    // We have a tree zone. For each of its terminii, find the acceptable
    // range of descendent tree zones and recurse.
	auto mutable_tree_zone = dynamic_cast<MutableTreeZone *>(tree_patch->GetZone());
	ASSERT( mutable_tree_zone );
    size_t i=0;
    Patch::ForChildren( tree_patch, [&](shared_ptr<Patch> &child_patch)    
    {
        XLink terminus = mutable_tree_zone->GetTerminusXLink(i++); 
        ConstrainAnyPatchToDescendants( child_patch, terminus, just_check );
    } );
}


void OrderingPass::AppendNextDescendantTreePatches( shared_ptr<Patch> &start_patch, 
                                                    PatchRecords &patch_records )
{
    // Insert descendent tree zones, skipping over free zones, into a list for
    // convenience.
    if( auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(start_patch) )
    {
		TRACE("Saw ")(tree_patch->GetZone()->GetBaseXLink())("\n");
        patch_records.push_back( { &start_patch, false } );
    }
    else if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(start_patch) )
    {
        Patch::ForChildren( free_patch, [&](shared_ptr<Patch> &child_patch)
        {
            AppendNextDescendantTreePatches( child_patch, patch_records );
        } );
    }
    else
    {
        ASSERTFAIL();
    }
}                                                                     


void OrderingPass::FindOutOfOrderTreePatches( PatchRecords &patch_records, 
 											  XLink base,
 											  bool just_check )
{          
	INDENT(just_check?"q":"Q");                                      
 	ASSERT( !patch_records.empty() );
    ASSERT( base );
    ASSERT( base.GetChildTreePtr() );        

    DFPatchIndexRelation dfpir( db, patch_records );   
                         	  
    // Get the patch records in depth-first order. Omit:
    // - outside the supplied range
    // - duplicated
    PatchIndicesDFO indices_dfo(dfpir);
    vector<XLink> v;
    for( size_t i=0; i<patch_records.size(); i++ )
    {
        XLink tz_base = GetBaseXLink( patch_records[i] );
        ASSERT( tz_base );
        ASSERT( tz_base.GetChildTreePtr() );        

        // Check the tree zone base is in overall range supplied to us for root or parent TZ terminus.
        // Inclusive: straightforward depth-first inclusive check
        // Exclusive: excludes equality AND ancestor/descendant relationship
		TRACE("Patch #%u base: ", i)(tz_base)("\n");
        auto p = dfr.CompareHierarchical(base, tz_base);
		bool in_range = (p.second == DepthFirstRelation::EQUAL || 
		                 p.second==DepthFirstRelation::LEFT_IS_ANCESTOR);		        
 
        if( !in_range )
        {
			if( just_check )
			{				
				FTRACE(db->GetOrderings().depth_first_ordering)("\n");
				ASSERTFAIL(); // we aint goin nowhere
			}            

            // Outside overall range. But we don't need to break up any
            // run: it's enough to omit from local DF ordering 
            continue;
        }    

		if( in_order_bases.count(tz_base) > 0 )
		{
			// This TZ is known to have been accepted as in-order somewhere else in the layout
			TRACE("Global duplicate rejected: index=%d patch=", i)(patch_records[i].patch_ptr)("\n");
			continue;
		}

        auto p2 = indices_dfo.insert(i);
        if( !p2.second )
 	    {
			// Fail, already there. TODO what if both inserted due eg multiset and 
			// always the "wrong" way around so one will be kicked out...
			TRACE("Local duplicate rejected: index=%d patch=", i)(patch_records[i].patch_ptr)("\n");
			continue;
 		}
    }
 
    TRACE("Provisional depth-first order: ")(indices_dfo)("\n");

    // Discover runs of ancestor-descendant related nodes
	map<size_t, set<size_t>> run_d_map, run_a_map; 
	size_t run_i=0;
	size_t prev_i=0;
	bool da = false;
	bool prev_da = false;
	bool first = true;
	for( size_t i : indices_dfo )
	{
		if( !first )
		{
			auto p = dfpir.CompareHierarchical( prev_i, i );
			TRACE("Compare #%u vs #%u: ", run_i, i)(p)("\n");
			ASSERT( p.first < 0 ); // should be given by the DFO ordering
			// so LEFT_IS_ANCESTOR is what we'd get
			da = (p.second == DepthFirstRelation::LEFT_IS_ANCESTOR);
			if( da && !prev_da )
				run_i = prev_i;
			
			if( da )
			{
				run_d_map[run_i].insert(i);
				run_a_map[run_i].insert(prev_i);
			}
		}
		
		first = false;
		prev_i = i;
		prev_da = da;
	}
  
#ifdef PREFER_TO_MOVE_DESCENDANT
	// discard the descendents
	for( auto p : run_d_map )
	{
		set<size_t> &descendents_i = p.second;
		for( size_t i : descendents_i )
		{
			TRACE("Removing descendant #%u\n", i);
			indices_dfo.erase( i );
		}
	}
#else
	// discard the parents
	for( auto p : run_a_map )
	{
		set<size_t> &ancestors_i = p.second;
		for( size_t i : ancestors_i )
		{
			TRACE("Removing ancestor #%u\n", i);
			indices_dfo.erase( i );
		}
	}
#endif	
		
    TRACE("Final depth-first order: ")(indices_dfo)("\n");         
    MaximalIncreasingSubsequence( indices_dfo );
    
    // We cannot use indices_dfo.count() generally, because depth-first isn't a
    // total ordering in the case where the same TZ appears more than once
    // in patch_records. This is OK since we refused to even insert the 
    // duplicates, but we must switch to a different ordering here.
    set<size_t> indices_simple;
    for( size_t i : indices_dfo )
		indices_simple.insert( i );
		
    // Patches are out of order if their index is missing from the ordering
    for( size_t i=0; i<patch_records.size(); i++ )   
		patch_records[i].out_of_order = (indices_simple.count( i ) == 0);
}
 
 
// TODO drop max_val
void OrderingPass::MaximalIncreasingSubsequence( PatchIndicesDFO &indices_dfo )
{
	size_t N = indices_dfo.size();
	vector<size_t> X;
	X.reserve( N ); // so we can use push_back() efficiently
	for( size_t x : indices_dfo )
		X.push_back( x );		
		
	vector<size_t> P(N);
	vector<size_t> M(N+1);
	M[0] = -1;

	size_t L = 0;
	for( size_t i=0; i<N; i++ )
	{
		// Binary search for the smallest positive l ≤ L
		// such that X[M[l]] >= X[i]
		size_t lo = 1;
		size_t hi = L + 1;
		while( lo < hi )
		{
			size_t mid = lo + (hi-lo)/2; // lo <= mid < hi
			if( X[M[mid]] >= X[i] )
				hi = mid;
			else // if X[M[mid]] < X[i]
				lo = mid + 1;
		}
		
		// After searching, lo == hi is 1 greater than the
		// length of the longest prefix of X[i]
		size_t newL = lo;

		// The predecessor of X[i] is the last index of 
		// the subsequence of length newL-1
		P[i] = M[newL-1];
		M[newL] = i;
		
		if( newL > L )
		{
			// If we found a subsequence longer than any we've
			// found yet, update L
			L = newL;
		}
	}

	// Reconstruct the longest increasing subsequence
	// It consists of the values of X at the L indices:
	// ...,  P[P[M[L]]], P[M[L]], M[L]
	indices_dfo.clear();
	size_t k = M[L];
	for( int j=L-1; j>=0; j-- ) //0 included
	{
		indices_dfo.insert(X[k]); // we don't mind that we're going backwards because indices_dfo is ordered
		k = P[k];
	}
}

 
void OrderingPass::RunDuplicate(shared_ptr<Patch> &layout)
{
	INDENT("D");

	multiset<XLink> out_of_order_bases;
    for( shared_ptr<Patch> *ooo_patch_ptr : out_of_order_patches )
    {
		auto ooo_tree_patch = dynamic_pointer_cast<TreeZonePatch>(*ooo_patch_ptr);
		ASSERT( ooo_tree_patch );
		XLink base_xlink = ooo_tree_patch->GetZone()->GetBaseXLink();
		ASSERT( base_xlink );
		// Unlike with in-order, there can be multiple OOO.
		out_of_order_bases.insert( base_xlink );
	}

	// Process duplications first, because we wouldn't want to duplicate
	// a TZ that aliasses a TZ that has had scaffolding put in.
    for( shared_ptr<Patch> *ooo_patch_ptr : out_of_order_patches )
    {
		auto ooo_tree_patch = dynamic_pointer_cast<TreeZonePatch>(*ooo_patch_ptr);
		ASSERT( ooo_tree_patch );
		XLink base_xlink = ooo_tree_patch->GetZone()->GetBaseXLink();
		ASSERT( base_xlink );
		ASSERT( out_of_order_bases.count(base_xlink) >= 1 ); // this one!
		if( out_of_order_bases.count(base_xlink) >= 2 || // Other out-of-orders, we should dup all but one
		    in_order_bases.count(base_xlink) >= 1 ) // An in-order, we should dup so it can be left alone
		{
			// This TZ is aliassed by other TZs
			TRACE("Duplicating ")(ooo_patch_ptr)("\n");

			// We'll have to duplicate. Best to duplicate the OOO one so we don't have to do a move
			shared_ptr<FreeZonePatch> new_free_patch = ooo_tree_patch->DuplicateToFree();
			*ooo_patch_ptr = new_free_patch;

			out_of_order_bases.erase(out_of_order_bases.lower_bound(base_xlink));
		}
	}
}


void OrderingPass::RunMoveOut(shared_ptr<Patch> &layout, MovesMap &moves_map)
{
	INDENT("M");
	// Now we can do the moves and insert scaffolding		
    for( shared_ptr<Patch> *ooo_patch_ptr : out_of_order_patches )
    {
		auto ooo_tree_patch = dynamic_pointer_cast<TreeZonePatch>(*ooo_patch_ptr);
		if( ooo_tree_patch )
		{
			XLink base_xlink = ooo_tree_patch->GetZone()->GetBaseXLink();
			ASSERT( base_xlink );
		
			TRACE("Moving ")(ooo_patch_ptr)("\n");
			
			// We can move it to the new place, avoiding the need for duplication
			MoveTreeZoneOut(ooo_patch_ptr, layout, moves_map);

			// But any further appearances must be duplicated TODO remove this? we're done duplicating!
			InsertSolo(in_order_bases, base_xlink); 
		}
	}
}


void OrderingPass::MoveTreeZoneOut( shared_ptr<Patch> *ooo_patch_ptr, shared_ptr<Patch> &layout, MovesMap &moves_map)
{
	// Out-of-order patch is located at the "to" location, but contains the "from" tree zone.
	auto ooo_tree_patch = dynamic_pointer_cast<TreeZonePatch>(*ooo_patch_ptr);
	ASSERT( ooo_tree_patch );
	MutableTreeZone &main_tree_zone_from_m = dynamic_cast<MutableTreeZone &>(*ooo_tree_patch->GetZone());
	ASSERT( !main_tree_zone_from_m.IsEmpty() ); // See #784
	XTreeZone main_tree_zone_from = main_tree_zone_from_m.GetXTreeZone();
	
	// ------------------------- Create extra tree with plugged scaffold ---------------------------
	// Make scaffold free zones that fit in place of the moving zone
	auto scaffold_zone_from = sops->CreateSimilarScaffoldZone(main_tree_zone_from);
	TRACE("\"From\" scaffold: ")(scaffold_zone_from)("\n");
	XTreeZone tree_zone_in_extra = sops->FreeZoneIntoExtraTree( scaffold_zone_from, main_tree_zone_from_m ).GetXTreeZone();
		
	// ------------------------- Swap "from" zone into our extra tree ---------------------------
	//FTRACE("main_tree_zone_from: ")(main_tree_zone_from)("\nfree_zone: ")(*free_zone)("\n");
	// Put the scaffold into the "from" part of the tree, displacing 
	// the original contents, which we shall move
	main_tree_zone_from.Validate(db);
	
	// Determine the fix-sops we'll need to do for tree zones in neighbouring patches
    vector<TreeZone *> fixups;	
    for( size_t i=0; i<main_tree_zone_from.GetNumTerminii(); i++ )
	{				
		MutableTreeZone *found = nullptr;
		TreeZonePatch::ForTreeDepthFirstWalk(layout, [&](shared_ptr<TreeZonePatch> &patch)
		{
			MutableTreeZone *candidate = dynamic_cast<MutableTreeZone *>(patch->GetZone());
			if( candidate->GetBaseMutator() == main_tree_zone_from_m.GetTerminusMutator(i) )
			{
				ASSERT( !found );
				found = candidate;
			}
		}, nullptr );
		
		fixups.push_back( found ); // does not have to be found; TZs can be disconnected
	}
			
	//db->DumpTables();

	// Swap in the true moving zone. Names become misleading because contents swap:
	// tree_zone_in_extra <- the actual moving zone now in extra tree
	// main_tree_zone_from <- the "from" scaffold now in main tree, to be killed by inversion
	db->XTreeDatabase::SwapTreeToTree( main_tree_zone_from, fixups,
		    						   tree_zone_in_extra, vector<TreeZone *>() );

	// ------------------------- Add "To" scaffolding patch to tree for inversion ---------------------------
	// tree_zone_in_extra now contains the moving zone	
	FreeZone scaffold_zone_to = sops->CreateSimilarScaffoldZone(tree_zone_in_extra); 
	TreePtr<Node> scaffold_base_to = scaffold_zone_to.GetBaseNode();
	TRACE("\"To\" scaffold base: ")(scaffold_zone_to)("\n");

	// Rememeber the association between the "to" scaffold node and the moving zone
	TRACE("Making map entry, \"to\" scaffold node: ")(scaffold_base_to)("\n moving zone: \n")(tree_zone_in_extra)("\n");
	InsertSolo( moves_map.mm, make_pair(scaffold_base_to, tree_zone_in_extra) );

	// Store the scaffold in the layout so it goes into inversion as a free zone, so it survives, 
	// and ands up in the tree at the "to" location.
	auto free_patch_to = make_shared<FreeZonePatch>( scaffold_zone_to, ooo_tree_patch->MoveChildren() );
	*ooo_patch_ptr = free_patch_to;
	
	ValidateTreeZones(db).Run(layout);
	
	// How does the scaffold not end up in the updated tree?
	// The best argument is that, after this pass, scaffold_zone_from is not
	// in any of the patches in our layout. The layout is the intended eventual 
	// contents of the update tree. So, if inversion acts correctly, scaffold_zone_from 
	// will be deleted from the tree. OTOH scaffold_zone_to will make it into
	// inversion as a free zone and enter the tree, but we're tracking it in moves_map
	// and will swap it out and delete it during MoveInPass.
}


shared_ptr<TreeZonePatch> OrderingPass::GetTreePatch(const PatchRecord &patch_record) const
{
    shared_ptr<Patch> *patch = patch_record.patch_ptr;
    ASSERT( patch );
    ASSERT( *patch );
    auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(*patch);
    ASSERT( tree_patch ); // Things should only be tree pointer ops
    return tree_patch;
}


XLink OrderingPass::GetBaseXLink(const PatchRecord &patch_record) const
{
    shared_ptr<TreeZonePatch> tree_patch = GetTreePatch(patch_record);
    return tree_patch->GetZone()->GetBaseXLink();
}
