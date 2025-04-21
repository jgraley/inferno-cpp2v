#include "ordering_pass.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

//#define DEBUG

#define NEW_THING

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
	
	shared_ptr<Patch> *r_pp = patch_records[r_key].patch_ptr;
    ASSERT( r_pp );
    ASSERT( *r_pp );
    auto r_tp = dynamic_pointer_cast<TreeZonePatch>(*r_pp);
    ASSERT( r_tp );
	XLink r_xlink = r_tp->GetZone()->GetBaseXLink();

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
	out_of_order_patches.clear();
	in_order_bases.clear();
	
    shared_ptr<Mutator> root = db->GetMainRootMutator();
    ConstrainAnyPatchToDescendants( layout, root, false );	
    ProcessOutOfOrder();
}


void OrderingPass::Check( shared_ptr<Patch> &layout )
{
	out_of_order_patches.clear();
	in_order_bases.clear();
	
	shared_ptr<Mutator> root = db->GetMainRootMutator();
    ConstrainAnyPatchToDescendants( layout, root, true );
}


void OrderingPass::ConstrainAnyPatchToDescendants( shared_ptr<Patch> &start_patch, 
												   shared_ptr<Mutator> base,
												   bool just_check )
{
    INDENT(just_check?"a":"A");
    TRACE("Starting ")(just_check ? "cross-check" : "transfomation")(" at ")(start_patch)(" with ancestor ")(base)("\n");
    // Actions to take when we have a range. Use at root and for
    // terminii of tree zones.
    PatchRecords patch_records;
    shared_ptr<Mutator> last_descendant = db->GetLastDescendantMutator(base);
    AppendNextDescendantTreePatches( start_patch, patch_records );
    ConstrainTreePatchesToRange(patch_records, base, last_descendant, just_check);
}


void OrderingPass::ConstrainTreePatchesToRange( PatchRecords patch_records, 
                                                shared_ptr<Mutator> lower,
                                                shared_ptr<Mutator> upper,
                                                bool just_check )
{                               
	INDENT(just_check?"r":"R");

	while(true)
	{
		if( patch_records.empty() )
			return;
        
		// patch_records is updated in-place with correct out_of_range values
		FindOutOfOrderTreePatches( patch_records, lower->GetXLink(), upper->GetXLink(), just_check );    
		
		bool more_to_check = false;
		
		// Loop over patches, with their associated out-of-order flags
		PatchRecords next_descendant_tree_patches;
		for( const PatchRecord &patch_record : patch_records )
		{
			if( patch_record.out_of_order ) // out-of-order patch
			{
				// The tree-zone descendants of this patch still need to be checked for OOO.
				// Accumulate a list of patch records for them. 
				auto tree_patch = GetTreePatch(patch_record);
				TRACE("Out-of-order patch: ")(tree_patch)("\nso gathering first descendants...\n");
				size_t size_before = next_descendant_tree_patches.size();
				Patch::ForChildren( tree_patch, [&](shared_ptr<Patch> &child_patch)
				{
					AppendNextDescendantTreePatches( child_patch, next_descendant_tree_patches );
				} );        
				size_t size_after = next_descendant_tree_patches.size();
				more_to_check = more_to_check || (size_after > size_before);

				// Mark as out of order so that the patch itself will be 
				// switched to a free zone patch.
				out_of_order_patches.push_back(patch_record.patch_ptr);
			}
			else // in-order patch
			{          
				next_descendant_tree_patches.push_back(patch_record);
			}
		}
		
		if( !more_to_check )
			break;
		
		patch_records = next_descendant_tree_patches;
	}
    
    // Recurse to check our successes
    for( const PatchRecord &patch_record : patch_records )
    {
		if( !patch_record.out_of_order )
		{
            auto tree_patch = GetTreePatch(patch_record);

            // InsertSolo is used here because we should only find each TZ as being in order once
            //FTRACE(tree_patch)("\nSet is:\n")(in_order_bases)("\n");
            InsertSolo(in_order_bases, tree_patch->GetZone()->GetBaseXLink());

			// In-order patch will remain a tree patch, so use the terminii to establish 
			// new ranges and recurse.
            TRACE("In-order patch, so big recursion on ")(tree_patch)("...\n");
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
        shared_ptr<Mutator> terminus = mutable_tree_zone->GetTerminusMutator(i++); 
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
 											  XLink lower,
 											  XLink upper,
 											  bool just_check )
{          
	INDENT(just_check?"f":"F");                                      
 	ASSERT( !patch_records.empty() );
    DFPatchIndexRelation dfpir( db, patch_records );   
                        
 	ASSERT( db->HasRow(upper) );
 	
 	TRACE("Range: ")(lower)(" to ")(upper)(" inclusive\n");
 
    // Data structures for runs of things that are consecutive wrt DF ordering
 
    // set of pairs: run length to front index. Will be ordered:
    // - primarily by run length
    // - secondarily by index into vector (via preserved insertion order)
    // Note: multimap makes it hard to remove a single element if you don't
    // already have the required iterator.
    set<pair<int, int>> runs_by_length; 
 
    // Map from the fronts of runs to the run lengths
    map<int, int> runs_by_front_index; // TODO first->front etc
 
    // Get the patch records in depth-first order. Omit:
    // - outside the supplied range
    // - duplicated
    IndicesDFO indices_dfo_pre(dfpir);
    vector<XLink> v;
    for( size_t i=0; i<patch_records.size(); i++ )
    {
        XLink tz_base = GetBaseXLink( patch_records[i] );

        // Check the tree zone base is in overall range supplied to us for root or parent TZ terminus.
        // Inclusive: straightforward depth-first inclusive check
        // Exclusive: excludes equality AND ancestor/descendant relationship
		TRACE("Patch #%u base: ", i)(tz_base)("\n");
        bool in_range = true;
        auto p_lower = dfr.CompareHierarchical(lower, tz_base);
		in_range = in_range && (p_lower.first <= 0);		        
        auto p_upper = dfr.CompareHierarchical(tz_base, upper);
		in_range = in_range && (p_upper.first <= 0);
 
        if( !in_range )
        {
			if( just_check )
			{				
				FTRACE(db->GetOrderings().depth_first_ordering)("\n");
				FTRACE("LOWER: ")(lower)(" compare result: ")(p_lower)("\n");
				FTRACE("UPPER: ")(upper)(" compare result: ")(p_upper)("\n");
				ASSERTFAIL(); // we aint goin nowhere
			}            

            // Outside overall range. But we don't need to break up any
            // run: it's enough to omit from local DF ordering 
            // and set out_of_order flag.
            patch_records[i].out_of_order = true;
            continue;
        }    

		if( in_order_bases.count(tz_base) > 0 )
		{
			// This TZ is known to have been accepted as in-order somewhere else in the layout
			patch_records[i].out_of_order = true;
			continue;
		}

        auto p = indices_dfo_pre.insert(i);
        if( !p.second )
 	    {
			// Fail, already there. The DFO element that's already there
			// might get kicked out later, so possibly do this filtering later TODO
			patch_records[i].out_of_order = true;
			continue;
 		}
    }
 
    TRACE("Provisional depth-first order: ")(indices_dfo_pre)("\n");

 
	IndicesDFO indices_dfo(dfpir);
	size_t prev_i=0;
	bool first = true;
	for( size_t i : indices_dfo_pre )
	{
		if( !first )
		{
			auto p = dfpir.CompareHierarchical( prev_i, i );
			TRACE("Compare #%u vs #%u: ", prev_i, i)(p)("\n");
			ASSERT( p.first < 0 ); // should be given by the DFO ordering
			// so LEFT_IS_ANCESTOR is what we'd get
			if( p.second == DepthFirstRelation::LEFT_IS_ANCESTOR )
			{ 
				TRACE("Removing #%u\n", i);
				patch_records[i].out_of_order = true;
				continue; 
			}
		}
		
		indices_dfo.insert( i );
		
		first = false;
		prev_i = i;
	}
 
    TRACE("Final depth-first order: ")(indices_dfo)("\n");
 
    // Find runs of patch records that are consecutive both in the layout and
    // the DF ordering, breaking on patch records outside the supplied overall range.
    int run_start_i=0;
    prev_i=0;
    first = true;     
    for( size_t i=0; i<patch_records.size(); i++ )
    {
 		if( patch_records[i].out_of_order )
			continue;

		// First patch record gets it for free, then we have to check the DF ordering
		bool consecutive = first || AreLinksConsecutive(prev_i, i, indices_dfo, dfpir);
		TRACE(i)(consecutive?"":" NOT")(" consecutive\n");

		if( just_check && !consecutive )
		{
			FTRACE("DFO: \n")(indices_dfo)("\nprev: ")(prev_i)(" current: ")(i)("\n");
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

		prev_i = i;
		first = false;
    }
     
    // Complete the final run.
    int length = patch_records.size() - run_start_i;
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
        TRACE("Removing run of %d OOO:\n", l_ooo);
        for( int i=i_ooo_front; i<i_ooo_front+l_ooo; i++ )
        {
            if( patch_records[i].out_of_order )
                ASSERT( indices_dfo.count( i ) == 0 ); // should already be gone
            else
                EraseSolo( indices_dfo, i ); 
 
            patch_records[i].out_of_order = true;
            TRACE(i)(": ")(patch_records[i].patch_ptr)("\n");
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
                //FTRACE("front: %d of %d\n", i_front, patch_records.size());
                XLink tz_base_front = GetBaseXLink( patch_records[i_front] );
 
                if( AreLinksConsecutive(prev_i_back, i_front, indices_dfo, dfpir) ) // Can we concatenate?
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
 
 
bool OrderingPass::AreLinksConsecutive(size_t left, size_t right, set<size_t, DFPatchIndexRelation> &indices_dfo, DFPatchIndexRelation &dfpir) const
{
    set<size_t, DFPatchIndexRelation>::iterator left_it = indices_dfo.find(left);
    set<size_t, DFPatchIndexRelation>::iterator right_it = indices_dfo.find(right);
          
    // If either is missing from DFO, we mustn't assume consecutivity
	if( left_it == indices_dfo.end() || right_it == indices_dfo.end() )
		return false;
          
    // If we're up against either end of the DFO, they can't be consecutive
	if( next(left_it) == indices_dfo.end() || right_it == indices_dfo.begin() )
		return false;
          
    // They have to be consecutive in the DFO
    if( next(left_it) != right_it )
	    return false;
          
    // A ancestor-descendent pair cannot be contguous because during inversion
    // the resulting tree zone terminii would break zone rules
	auto p = dfpir.CompareHierarchical( left, right );
	ASSERT( p.first < 0 ); // should be given by the DFO check
	ASSERT( p.second != DepthFirstRelation::LEFT_IS_ANCESTOR );
     
    return true;
}
 
 
void OrderingPass::ProcessOutOfOrder()
{
	INDENT("P");

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
	// a TZ that aliasses a TZ that has had scffolding put in.
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

			// Add to intrinsic tables in DB because we missed InsertIntrinsicPass
			db->MainTreeInsertIntrinsic( new_free_patch->GetZone() );     

			out_of_order_bases.erase(out_of_order_bases.lower_bound(base_xlink));
		}
	}
	
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
			MoveTreeZoneToFreePatch(ooo_patch_ptr);

			// But any further appearances must be duplicated
			InsertSolo(in_order_bases, base_xlink); 
		}
	}
}


void OrderingPass::MoveTreeZoneToFreePatch( shared_ptr<Patch> *target_patch)
{
	INDENT("M");

	// Get the tree zone
	auto target_tree_patch = dynamic_pointer_cast<TreeZonePatch>(*target_patch);
	ASSERT( target_tree_patch );
	MutableTreeZone *target_tree_zone = dynamic_cast<MutableTreeZone *>(target_tree_patch->GetZone());
	ASSERT( target_tree_zone );
	ASSERT( !target_tree_zone->IsEmpty() ); // See #784
	
	// Create the scaffold in a free zone
	auto free_zone = FreeZone::CreateScaffold( target_tree_zone->GetBaseMutator()->GetTreePtrInterface(), 
											   target_tree_zone->GetNumTerminii() );
	
	//FTRACE("target_tree_zone: ")(*target_tree_zone)("\nfree_zone: ")(*free_zone)("\n");
	// Put the scaffold into the "from" part of the tree, displacing 
	// the original contents, which we shall move
	db->MainTreeExchange( target_tree_zone, free_zone.get() );
	// free_zone is the part of the tree that we just displaced. Make 
	// a new patch based on it.
	auto free_patch = make_shared<FreeZonePatch>( move(free_zone), target_tree_patch->MoveChildren() );
	
	// Install the new patch into the layout
	free_patch->AddEmbeddedMarkers( target_tree_patch->GetEmbeddedMarkers() );               
	*target_patch = free_patch;
	
	// How does the scaffold not end up in the updated tree?
	// The best argument is that, after this pass, none of the
	// scaffold nodes are inside any of the patches in our layout.
	// The layout is intended contents of the update tree. So, if 
	// subsequent passes and the DB act correctly, the scaffolds 
	// will be deleted from the tree.                
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


shared_ptr<Mutator> OrderingPass::GetBaseMutator(const PatchRecord &patch_record) const
{
    shared_ptr<TreeZonePatch> tree_patch = GetTreePatch(patch_record);
	auto mutable_tree_zone = dynamic_cast<MutableTreeZone *>(tree_patch->GetZone());
	ASSERT( mutable_tree_zone );
    return mutable_tree_zone->GetBaseMutator();
}

// ------------------------- AltOrderingChecker --------------------------

// Different algo from that in OrderingPass, just for checking

AltOrderingChecker::AltOrderingChecker(const XTreeDatabase *db_) :
    db( db_ ),
    dfr( db )
{
}
    

void AltOrderingChecker::Check( shared_ptr<Patch> layout )
{
    Worker(layout, db->GetMainRootXLink(), false);
}


void AltOrderingChecker::Worker( shared_ptr<Patch> patch, XLink x_sub_base, bool should_touch )
{	
    if( auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(patch) )
    {
        INDENT("t");
		TRACE("Checking x_sub_base: ")(x_sub_base)(" against patch:\n")(patch)("\nshould_touch: ")(should_touch)("\n");

        // Got a TreeZone - check ordering of its base, strictness depending on who called us
        const TreeZone *tree_zone = tree_patch->GetZone();
        XLink base = tree_zone->GetBaseXLink();
        auto p = dfr.CompareHierarchical( x_sub_base, base );
        if( should_touch )
			ASSERT( p.second==DepthFirstRelation::EQUAL )(x_sub_base)(" vs ")(base)(" got ")(p);
		else
			ASSERT( p.second==DepthFirstRelation::EQUAL ||
			        p.second==DepthFirstRelation::LEFT_IS_ANCESTOR )(x_sub_base)(" vs ")(base)(" got ")(p);
        
        if( tree_zone->IsEmpty() )
		{
			TRACE("TZ is empty\n");
			ASSERT( base==OnlyElementOf(tree_zone->GetTerminusXLinks())); // Definition of empty tree zone
			Worker(OnlyElementOf(patch->GetChildren()), OnlyElementOf(tree_zone->GetTerminusXLinks()), true);
			return;
		}
        
        // TODO this loop is similar to the one in the FZ case. Pop them both out into 
        // a new method. First call tree_patch->GetChildren() to get a list<shared_ptr<Patch>>
        // and co-loop with that. Then pop out into new function. Then have 
        // AppendNextDescendantTreePatches() also use a list. Then the FZ case 
        // populates vector<XLink> with the ndt_bases and use the new method.
        // Prefactor: combine the two loops in FZ case.
        // Co-loop over the children/terminii
		XLink prev = XLink();
        vector<XLink> terminii = tree_zone->GetTerminusXLinks();
        FreeZonePatch::ChildPatchIterator it_child = tree_patch->GetChildrenBegin();        
        for( XLink terminus : terminii )
        {
            ASSERT( it_child != tree_patch->GetChildrenEnd() ); // length mismatch

			if( prev )
			{
				auto p = dfr.CompareHierarchical( prev, terminus );
				ASSERT( p.first < 0 ); // strict: no repeated XLinks
				// Terminii should not be in parent/child relationships
				ASSERT( p.second != DepthFirstRelation::LEFT_IS_ANCESTOR )(prev)(" vs ")(terminus)(" got ")(p);
			}

			auto p2 = dfr.CompareHierarchical( base, terminus );
			ASSERT( p2.second == DepthFirstRelation::LEFT_IS_ANCESTOR )(base)(" vs ")(terminus)(" got ")(p2); 
				
			prev = terminus;        

            // Check everything under the corresponding child is in order. 
            Worker( *it_child, terminus, false );
            
            ++it_child;
        }
        ASSERT( it_child == tree_patch->GetChildrenEnd() ); // length mismatch
    }
    else if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(patch) )
    {
        INDENT("f");
		TRACE("Checking x_sub_base: ")(x_sub_base)(" against patch:\n")(patch)("\nshould_touch: ")(should_touch)("\n");
        ASSERT( !free_patch->GetZone()->IsEmpty() ); // TODO handle this case as with TZs, above
        
		vector<shared_ptr<TreeZonePatch>> ndt_patches;
		AppendNextDescendantTreePatches( patch, &ndt_patches );
		// We're finding tree patches, so they will all have XLinks as bases.
		
		TRACE("TZ patches found by AppendNextDescendantTreePatches():\n", ndt_patches.size())(ndt_patches)("\n");
		
		XLink prev = XLink();
        for( shared_ptr<TreeZonePatch> ndt_patch : ndt_patches )
        {
			// ndt_base would become a terminus during inversion, so must
			// obey the same rules as TZ terminii, relative to sub_base
			XLink ndt_base = ndt_patch->GetZone()->GetBaseXLink();
			TRACE("Checking ndt_base: ")(ndt_base)("\n");
			
			if( prev )
			{
				auto p = dfr.CompareHierarchical( prev, ndt_base );
				ASSERT( p.first < 0 )(prev)(" vs ")(ndt_base)(" got ")(p); // strict: no repeated XLinks
				// Terminii should not be in parent/child relationships
				ASSERT( p.second != DepthFirstRelation::LEFT_IS_ANCESTOR )(prev)(" vs ")(ndt_base)(" got ")(p); 
			}

			auto p2 = dfr.CompareHierarchical( x_sub_base, ndt_base );
			ASSERT( p2.second == DepthFirstRelation::LEFT_IS_ANCESTOR ||
			        p2.second == DepthFirstRelation::EQUAL )(x_sub_base)(" vs ")(ndt_base)(" got ")(p2);  

			prev = ndt_base;        
		}				

        for( shared_ptr<TreeZonePatch> ndt_patch : ndt_patches )
        {
			XLink ndt_base = ndt_patch->GetZone()->GetBaseXLink();
            Worker( ndt_patch, ndt_base, false );
        }
    }
    else
    {
        ASSERTFAIL();
    }
}


void AltOrderingChecker::AppendNextDescendantTreePatches( shared_ptr<Patch> start_patch, 
                                                          vector<shared_ptr<TreeZonePatch>> *ndt_patches )
{
    // Insert descendent tree zones, skipping over free zones, into a list for
    // convenience.
    if( auto tree_patch = dynamic_pointer_cast<TreeZonePatch>(start_patch) )
    {
        ndt_patches->push_back( tree_patch );
    }
    else if( auto free_patch = dynamic_pointer_cast<FreeZonePatch>(start_patch) )
    {
        Patch::ForChildren( free_patch, [&](shared_ptr<Patch> &child_patch)
        {
            AppendNextDescendantTreePatches( child_patch, ndt_patches );
        } );
    }
    else
    {
        ASSERTFAIL();
    }	
}                                                          


