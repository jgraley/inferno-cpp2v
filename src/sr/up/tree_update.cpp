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
	db->BuildTree(DBCommon::TreeType::MAIN, main_zone);
	
	db->PerformDeferredActions();	
}


void TreeUpdater::TeardownMainTree()
{
	db->TeardownTree( db->GetMainTreeOrdinal() );
}


unique_ptr<FreeZone> TreeUpdater::TransformToSingleFreeZone( shared_ptr<Patch> layout )
{
    DuplicateAllPass duplicate_all_pass;
    duplicate_all_pass.Run(layout);  
    duplicate_all_pass.Check(layout);
    
    MergeFreesPass merge_frees_pass;
    merge_frees_pass.Run(layout);  
    merge_frees_pass.Check(layout);

    auto free_patch = dynamic_pointer_cast<FreePatch>(layout);
    ASSERT( free_patch );
    ASSERT( free_patch->GetNumChildren() == 0 );
    FreeZone *free_zone = free_patch->GetZone();
    ASSERT( free_zone->GetNumTerminii() == 0 );
    return make_unique<FreeZone>(*free_zone);
}


void TreeUpdater::UpdateMainTree( XLink origin_xlink, shared_ptr<Patch> layout )
{
    ASSERT( db );
                
	Analysis(origin_xlink, layout);
	ApplyUpdate(origin_xlink, layout);
}


void TreeUpdater::Analysis(XLink origin_xlink, shared_ptr<Patch> &layout)
{	
	ValidateTreeZones validate_zones(db);

	SetTreeOrdinals set_ordinals( db );
	set_ordinals.Run(layout);

    MergeWidesPass merge_wides_pass;
    merge_wides_pass.Run(layout);
    //merge_wides_pass.Check(layout);                           
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

    OrderingPass ordering_pass( db );
    ordering_pass.Run(layout);
    
    ChooseCopiesPass copies_pass;
    copies_pass.Run(layout);    
    
    GapFindingPass gap_finding_pass;
    gap_finding_pass.Run(layout);      
}
	
	
void TreeUpdater::ApplyUpdate(XLink origin_xlink, shared_ptr<Patch> &layout)
{	
	ValidateTreeZones validate_zones(db);
	ScaffoldOps sops( db );
	MovesMap moves_map;
	
	CopyingPass copying_pass;
	copying_pass.Run(layout);	
	//validate_zones.Run(layout);	

    MarkersPass markers_pass;
    markers_pass.Run(layout);
	//validate_zones.Run(layout);

	MoveOutPass move_out_pass( db, &sops );
	move_out_pass.Run(layout, moves_map);
	//validate_zones.Run(layout);         

    MergeFreesPass merge_frees_pass;
    merge_frees_pass.Run(layout);  
    //merge_frees_pass.Check(layout);
    
    //AltOrderingChecker alt_ordering_checker( db );
    //alt_ordering_checker.Check(layout);

    InversionPass inversion_pass( db, &sops ); 
    inversion_pass.RunInversion(origin_xlink, &layout);      
	
	MoveInPass move_in_pass( db, &sops );
	move_in_pass.Run(moves_map);
	
	db->PerformDeferredActions();  
}
