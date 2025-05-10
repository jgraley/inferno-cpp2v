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
#include "complement_pass.hpp"
#include "gap_finding_pass.hpp"
#include "boundary_pass.hpp"
#include "alt_ordering_checker.hpp"

#include <iostream>

using namespace SR;


// ------------------------- Runners --------------------------

TreeUpdater::TreeUpdater(XTreeDatabase *x_tree_db) :
    db( x_tree_db )
{
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


void TreeUpdater::TransformToIncrementalAndExecute( XLink origin_xlink, shared_ptr<Patch> source_layout )
{
    ASSERT( db );
                
	ToMutablePass to_mutable_pass( db );
	to_mutable_pass.Run(source_layout);
	Mutator origin_mutator = db->CreateTreeMutator(origin_xlink);
	ValidateTreeZones validate_zones(db);
	validate_zones.Run(source_layout);

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
    ordering_pass.RunAnalysis(source_layout);
    ordering_pass.RunDuplicates(source_layout);
	validate_zones.Run(source_layout);	

    MarkersPass markers_pass( db );
    markers_pass.Run(source_layout);

#ifndef NEW_THING
    InsertIntrinsicPass insert_intrinsic_pass( db );
    insert_intrinsic_pass.Run(source_layout);
	validate_zones.Run(source_layout);
#endif

    ordering_pass.RunMoves(source_layout);
    ordering_pass.Check(source_layout);
	validate_zones.Run(source_layout);       
    ScaffoldChecker().Run(source_layout);
    
    GapFindingPass gap_finding_pass;
    gap_finding_pass.Run(source_layout);    

    MergeFreesPass merge_frees_pass;
    merge_frees_pass.Run(source_layout);  
    merge_frees_pass.Check(source_layout);
    
    AltOrderingChecker alt_ordering_checker( db );
    alt_ordering_checker.Check(source_layout);

    InversionPass inversion_pass( db ); 
    inversion_pass.RunInversion(origin_mutator, &source_layout);      
#ifndef NEW_THING
    inversion_pass.RunDeleteIntrinsic();
#endif   
	db->PerformDeferredActions();
   
    if( ReadArgs::test_db )
        db->CheckIntrinsic();
}
