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
    auto l_tp = dynamic_pointer_cast<TreePatch>(*l_pp);
    ASSERT( l_tp );
	XLink l_xlink = l_tp->GetZone()->GetBaseXLink();
	ASSERT( l_xlink );
	ASSERT( l_xlink.GetChildTreePtr() );
	
	shared_ptr<Patch> *r_pp = patch_records[r_key].patch_ptr;
    ASSERT( r_pp );
    ASSERT( *r_pp );
    auto r_tp = dynamic_pointer_cast<TreePatch>(*r_pp);
    ASSERT( r_tp );
	XLink r_xlink = r_tp->GetZone()->GetBaseXLink();
	ASSERT( r_xlink );
	ASSERT( r_xlink.GetChildTreePtr() );

	return df.CompareHierarchical( l_xlink, r_xlink );
	ASSERTFAIL();
}


// ------------------------- OrderingPass --------------------------

OrderingPass::OrderingPass(XTreeDatabase *db_) :
    db( db_ ),
    dfr( db )
{
}
    

void OrderingPass::Run( shared_ptr<Patch> &layout )
{    
	INDENT("A");
	in_order_bases.clear();
	
    XLink root = db->GetMainRootXLink();
    ConstrainAnyPatchToDescendants( layout, root );	
}


void OrderingPass::ConstrainAnyPatchToDescendants( shared_ptr<Patch> &start_patch, 
												   XLink base )
{
    INDENT("A");
    TRACE("Starting at ")(start_patch)(" with ancestor ")(base)("\n");
    
    // Determine tree patches descending from starting patch.
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
		FindOutOfOrderTreePatches( patch_records, base );    
		
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
				dynamic_cast<TreePatch &>(**patch_record.patch_ptr).SetIntent( TreePatch::Intent::MOVEABLE );				
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
            ConstrainChildrenToTerminii( tree_patch );
   		}
	}
}
                                       

void OrderingPass::ConstrainChildrenToTerminii( shared_ptr<TreePatch> &tree_patch )
{
	INDENT("T");
	
    // We have a tree zone. For each of its terminii, find the acceptable
    // range of descendent tree zones and recurse.
	TreeZone tree_zone = tree_patch->GetXTreeZone();
    size_t i=0;
    Patch::ForChildren( tree_patch, [&](shared_ptr<Patch> &child_patch)    
    {
        XLink terminus = tree_zone.GetTerminusXLink(i++); 
        ConstrainAnyPatchToDescendants( child_patch, terminus );
    } );
}


void OrderingPass::AppendNextDescendantTreePatches( shared_ptr<Patch> &start_patch, 
                                                    PatchRecords &patch_records )
{
    // Insert descendent DEFAULT tree zones into a list for convenience.
    if( auto tree_patch = dynamic_pointer_cast<TreePatch>(start_patch) )
    {
		if( tree_patch->GetIntent() == TreePatch::Intent::DEFAULT )
		{
			TRACE("Saw ")(tree_patch->GetZone()->GetBaseXLink())("\n");
			patch_records.push_back( { &start_patch, false } );
			return;
		}
    }

	// Recurse through anything else
    Patch::ForChildren( start_patch, [&](shared_ptr<Patch> &child_patch)
    {
        AppendNextDescendantTreePatches( child_patch, patch_records );
    } );
}                                                                     


void OrderingPass::FindOutOfOrderTreePatches( PatchRecords &patch_records, 
 											  XLink base )
{          
	INDENT("Q");                                      
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


shared_ptr<TreePatch> OrderingPass::GetTreePatch(const PatchRecord &patch_record) const
{
    shared_ptr<Patch> *patch = patch_record.patch_ptr;
    ASSERT( patch );
    ASSERT( *patch );
    auto tree_patch = dynamic_pointer_cast<TreePatch>(*patch);
    ASSERT( tree_patch ); // Things should only be tree pointer ops
    return tree_patch;
}


XLink OrderingPass::GetBaseXLink(const PatchRecord &patch_record) const
{
    shared_ptr<TreePatch> tree_patch = GetTreePatch(patch_record);
    return tree_patch->GetZone()->GetBaseXLink();
}
