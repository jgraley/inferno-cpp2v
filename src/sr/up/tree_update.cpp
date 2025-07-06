#include "tree_update.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"
#include "misc_passes.hpp"
#include "ordering_pass.hpp"
#include "merge_passes.hpp"
#include "inversion_pass.hpp"
#include "gap_finding_pass.hpp"
#include "boundary_pass.hpp"
#include "alt_ordering_checker.hpp"
#include "move_in_pass.hpp"
#include "move_out_pass.hpp"
#include "copy_passes.hpp"
#include "scaffold_ops.hpp"
#include "up_common.hpp"

#include <iostream>

using namespace SR;


// ------------------------- Runners --------------------------

TreeUpdater::TreeUpdater(XTreeDatabase *x_tree_db) :
    db( x_tree_db )
{
	ASSERT( x_tree_db );
}


void TreeUpdater::BuildMainTree( TreePtr<Node> main_tree_root )
{
	FreeZone main_zone = FreeZone::CreateSubtree(main_tree_root);
	db->BuildTree(DBCommon::TreeType::MAIN, main_zone);
	
	db->PerformDeferredActions();	
}


void TreeUpdater::TeardownMainTree()
{
	db->TeardownTree( db->GetMainTreeOrdinal() );
}


unique_ptr<FreeZone> TreeUpdater::TransformToSingleFreeZone( shared_ptr<Patch> layout )
{
	// We just want to make one big free zone from the layout. We don't touch the 
	// database, so method is static.
	 	
    DuplicateAllPass duplicate_all_pass;
    duplicate_all_pass.Run(layout);  
    duplicate_all_pass.Check(layout);
    
    MergeFreesPass merge_frees_pass;
    merge_frees_pass.Run(layout, nullptr);  
    merge_frees_pass.Check(layout);

    auto free_patch = dynamic_pointer_cast<FreePatch>(layout);
    ASSERT( free_patch );
    ASSERT( free_patch->GetNumChildren() == 0 );
    FreeZone *free_zone = free_patch->GetZone();
    ASSERT( free_zone->GetNumTerminii() == 0 );
    return make_unique<FreeZone>(*free_zone);
}


ReplaceAssignments TreeUpdater::UpdateMainTree( XLink origin_xlink, shared_ptr<Patch> layout )
{               
	assignments.clear();
	
    // Figure out what we should do, then do it. See comments in these 
    // functions and the pass class headers.
	Analysis(origin_xlink, layout);
	ApplyUpdate(origin_xlink, layout);

	// Act on replace assignements info
	// TODO
	// - fix NEWS in SCREngine
	// - checks: did we get an assignmant for every originator? Any extra?
	// - checks: are all the assignment XLinks valid and in main tree?
	// - route the assignemtns out though to SCR engine and don't call 
	//   the originators directly incl. 
	// - TODOs in FreeZoneMergeImpl::Run()
	// - Drop rule #726
	return assignments;
}


void TreeUpdater::Analysis(XLink origin_xlink, shared_ptr<Patch> &layout)
{	
	// Roughtly an analysis phase. We can modify the layout but no
	// changes to the db or zone duplications. The input layout is a
	// mixture of free and DEFAULT tree zones. After this returns:
	// - Free zones should be duplicated and added to the db
	// - Tree zones with COPYABLE should be duplicated and added to the db
	// - Tree zones with MOVABLE should be moved within the db
	//   (from TZ position to position implied by patch)
	// - Tree zones with DEFAULT can be left alone
	
	ValidateTreeZones validate_zones(db);

	SetTreeOrdinals set_ordinals( db );
	set_ordinals.Run(layout);

    MergeSubcontainerBasePass merge_wides_pass;
    merge_wides_pass.Run(layout);
	//validate_zones.Run(layout);

	ProtectDEPass protect_de_pass( db );
	protect_de_pass.Run(layout);                                  
	//validate_zones.Run(layout);

    EmptyZonePass empty_zone_pass;
    empty_zone_pass.Run(layout);
    //empty_zone_pass.Check(layout);
	//validate_zones.Run(layout);

    BoundaryPass boundary_pass( db );
    boundary_pass.Run(layout);
	//validate_zones.Run(layout);

	// INVARIANT: our boundaries are unambiguous: zones either overflap
	// fully or not at all.

    OrderingPass ordering_pass( db );
    ordering_pass.Run(layout);
    
    // Choice of copies takes into accout the choice of moves we just made.
    
    ChooseCopiesPass copies_pass;
    copies_pass.Run(layout);    
    
    // Fix issue with gaps missing inversion.    
    GapFindingPass gap_finding_pass;
    gap_finding_pass.Run(layout);      
}
	
	
void TreeUpdater::ApplyUpdate(XLink origin_xlink, shared_ptr<Patch> &layout)
{		
	// To implement the changes: we will duplicate COPYABLEs into free, move
	// MOVABLES out of main tree, insert all new stuff into main tree, and
	// then move MOVABLES into new tree at new location. We only need 
	// DEFAULT tree zones for inversion.
	
	ValidateTreeZones validate_zones(db);
	ScaffoldOps sops( db );
	
	CopyingPass copying_pass;
	copying_pass.Run(layout);	
	//validate_zones.Run(layout);	

	// INVARIANT: by now, all true NEW content is in free patches; no COPYABLE TZs

    MovesMap moves_map;
	MoveOutPass move_out_pass( db, &sops );
	move_out_pass.Run(layout, moves_map); // This pass will populate the moves map
	//validate_zones.Run(layout);         

	// INVARIANT: by now, everything we need to invert is in a free patch;
	// all tree patches are DEFAULT and this means we can leave them alone.

    MergeFreesPass merge_frees_pass;
    merge_frees_pass.Run(layout, &assignments);  
    //merge_frees_pass.Check(layout);   
    //AltOrderingChecker alt_ordering_checker( db );
    //alt_ordering_checker.Check(layout);    
 
	// INVARIANT: free patches touch root and/or tree patches (which are all 
	// leave-alone); inversion now possible

    InversionPass inversion_pass( db, &sops ); 
    inversion_pass.RunInversion(origin_xlink, &layout);      

	// After inversion, we're done with the layout (and it might be invalid, see
	// comment in code). We will work from the moves map.
	
	MoveInPass move_in_pass( db, &sops );
	move_in_pass.Run(moves_map, &assignments);

    GetTreePatchAssignmentsPass get_assigns_pass;
    get_assigns_pass.Run(layout, &assignments);

	// Delayed actions in DB i.e. new/invalidated stimulus checks for
	// domain extenstion.
	db->PerformDeferredActions();      	
}
