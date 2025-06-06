#include "tree_update.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"
#include "misc_passes.hpp"
#include "overlap_pass.hpp"
#include "ordering_pass.hpp"
#include "merge_passes.hpp"
#include "inversion_pass.hpp"
#include "gap_finding_pass.hpp"
#include "boundary_pass.hpp"
#include "alt_ordering_checker.hpp"
#include "move_in_pass.hpp"
#include "scaffold_ops.hpp"

#include <iostream>

using namespace SR;


// ------------------------- Runners --------------------------

TreeUpdater::TreeUpdater(XTreeDatabase *x_tree_db) :
    db( x_tree_db )
{
}


void TreeUpdater::BuildMainTree( TreePtr<Node> main_tree_root )
{
	FreeZone main_zone = FreeZone::CreateSubtree(main_tree_root);
	db->BuildTree(DBCommon::TreeOrdinal::MAIN, main_zone);
	
	db->PerformDeferredActions();
	
    if( ReadArgs::test_db )
        db->CheckAssets();	
}


unique_ptr<FreeZone> TreeUpdater::TransformToSingleFreeZone( shared_ptr<Patch> source_layout )
{
    DuplicateAllPass duplicate_all_pass;
    duplicate_all_pass.Run(source_layout);  
    duplicate_all_pass.Check(source_layout);
    
    MergeFreesPass merge_frees_pass;
    merge_frees_pass.Run(source_layout);  
    merge_frees_pass.Check(source_layout);

    auto free_patch = dynamic_pointer_cast<FreeZonePatch>(source_layout);
    ASSERT( free_patch );
    ASSERT( free_patch->GetNumChildren() == 0 );
    FreeZone *free_zone = free_patch->GetZone();
    ASSERT( free_zone->GetNumTerminii() == 0 );
    return make_unique<FreeZone>(*free_zone);
}


void TreeUpdater::UpdateMainTree( XLink origin_xlink, shared_ptr<Patch> source_layout )
{
    ASSERT( db );
                
	ValidateTreeZones validate_zones(db);

    MergeWidesPass merge_wides_pass;
    merge_wides_pass.Run(source_layout);
    merge_wides_pass.Check(source_layout);                           
	validate_zones.Run(source_layout);

	ProtectDEPass protect_de_pass( db );
	protect_de_pass.Run(source_layout);                                  
	validate_zones.Run(source_layout);

    EmptyZonePass empty_zone_pass;
    empty_zone_pass.Run(source_layout);
    empty_zone_pass.Check(source_layout);
	validate_zones.Run(source_layout);

    BoundaryPass boundary_pass( db );
    boundary_pass.Run(source_layout);
	validate_zones.Run(source_layout);

    OrderingPass ordering_pass( db );
    ordering_pass.Run(source_layout);
    
    ChooseCopiesPass copies_pass;
    copies_pass.Run(source_layout);    
	
	
	
	ScaffoldOps sops( db );
	
	CopyingPass copying_pass;
	copying_pass.Run(source_layout);
	
	validate_zones.Run(source_layout);	

    MarkersPass markers_pass;
    markers_pass.Run(source_layout);
	validate_zones.Run(source_layout);

	MovesMap moves_map;

	MoveOutPass move_out_pass( db, &sops );
	move_out_pass.Run(source_layout, moves_map);
	validate_zones.Run(source_layout);       
    
    GapFindingPass gap_finding_pass;
    gap_finding_pass.Run(source_layout);    

    MergeFreesPass merge_frees_pass;
    merge_frees_pass.Run(source_layout);  
    merge_frees_pass.Check(source_layout);
    
    AltOrderingChecker alt_ordering_checker( db );
    alt_ordering_checker.Check(source_layout);

    InversionPass inversion_pass( db, &sops ); 
    inversion_pass.RunInversion(origin_xlink, &source_layout);      
	
	MoveInPass move_in_pass( db, &sops );
	move_in_pass.Run(moves_map);
	
	db->PerformDeferredActions();
   
    if( ReadArgs::test_db )
        db->CheckAssets();
}
