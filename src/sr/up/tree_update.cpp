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
	shared_ptr<Mutator> origin_mutator = db->GetTreeMutator(origin_xlink);

    MergeWidesPass merge_wides_pass;
    merge_wides_pass.Run(source_layout);
    merge_wides_pass.Check(source_layout);
                           
	ProtectDEPass protect_de_pass( db );
	protect_de_pass.Run(source_layout);
                                    
    EmptyZonePass empty_zone_pass;
    empty_zone_pass.Run(source_layout);
    empty_zone_pass.Check(source_layout);
    
    BoundaryPass boundary_pass( db );
    boundary_pass.Run(source_layout);

    OverlapPass overlap_pass( db );
    overlap_pass.Run(source_layout);
    overlap_pass.Check(source_layout);

    ComplementPass complement_pass( db );
    complement_pass.Run(origin_mutator, source_layout);

    InsertIntrinsicPass insert_intrinsic_pass( db );
    insert_intrinsic_pass.Run(source_layout);

    OrderingPass ordering_pass( db );
    ordering_pass.Run(source_layout);
    ordering_pass.Check(source_layout);
    
    GapFindingPass gap_finding_pass;
    gap_finding_pass.Run(source_layout);    

    MergeFreesPass merge_frees_pass;
    merge_frees_pass.Run(source_layout);  
    merge_frees_pass.Check(source_layout);
    
    AltOrderingChecker alt_ordering_checker( db );
    alt_ordering_checker.Check(source_layout);

    MarkersPass markers_pass( db );
    markers_pass.Run(source_layout);

    InversionPass inversion_pass( db ); 
    inversion_pass.Run(origin_mutator, &source_layout);     
    
    // Avoid ginormous memory leak
    db->ClearMutatorCache();           
}
