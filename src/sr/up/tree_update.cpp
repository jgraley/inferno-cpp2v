#include "tree_update.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"
#include "up_utils.hpp"
#include "tz_overlap.hpp"
#include "tz_ordering.hpp"
#include "fz_merge.hpp"
#include "inversion.hpp"
#include "complement.hpp"
#include "gap_finder.hpp"

#include <iostream>


using namespace SR;

// ------------------------- Runners --------------------------

TreeUpdater::TreeUpdater(XTreeDatabase *x_tree_db) :
    db( x_tree_db )
{
}


unique_ptr<FreeZone> TreeUpdater::TransformToSingleFreeZone( shared_ptr<Patch> source_layout )
{
	DuplicateAllToFree all_to_free;
	all_to_free.Run(source_layout);  
	all_to_free.Check(source_layout);
	
	FreeZoneMerger free_zone_merger;
	free_zone_merger.Run(source_layout);  
	free_zone_merger.Check(source_layout);

	auto free_patch = dynamic_pointer_cast<FreeZonePatch>(source_layout);
	ASSERT( free_patch );
	ASSERT( free_patch->GetNumChildExpressions() == 0 );
	FreeZone free_zone = free_patch->GetZone();
	ASSERT( free_zone.GetNumTerminii() == 0 );
	return make_unique<FreeZone>(free_zone);
}


void TreeUpdater::TransformToIncrementalAndExecute( XLink target_origin, shared_ptr<Patch> source_layout )
{
	ASSERT( db );
		
	//FTRACE("Starting with layout:\n")(source_layout)("\n");
		
	EmptyZoneElider empty_zone_elider;
	empty_zone_elider.Run(source_layout);
	empty_zone_elider.Check(source_layout);
	
	TreeZoneComplementer tree_zone_complementor( db );
	tree_zone_complementor.Run(target_origin, source_layout);

	TreeZoneOverlapHandler tree_zone_overlap_handler( db );
	tree_zone_overlap_handler.Run(source_layout);
	tree_zone_overlap_handler.Check(source_layout);
	
	TreeZoneOrderingHandler tree_zone_ordering_handler( db );
	tree_zone_ordering_handler.Run(source_layout);
	tree_zone_ordering_handler.Check(source_layout);
	
	TreeZoneGapFinder tree_zone_gap_finder( db );
	tree_zone_gap_finder.Run(source_layout);	
	if( !tree_zone_gap_finder.GetGaps().empty() )
		FTRACE(tree_zone_gap_finder.GetGaps());

	FreeZoneMerger free_zone_merger;
	free_zone_merger.Run(source_layout);  
	free_zone_merger.Check(source_layout);
	
	AltTreeZoneOrderingChecker alt_free_zone_ordering_checker( db );
	alt_free_zone_ordering_checker.Check(source_layout);

	// Enact the tree zones that will stick around
	BaseForEmbeddedMarkPropagation bfe_mark_propagation( db );
	bfe_mark_propagation.Run(source_layout);

	// Inversion generates sequence of separate "small" update commands 
	TreeZoneInverter tree_zone_inverter( db ); 
	tree_zone_inverter.Run(target_origin, &source_layout);	
			
	// For each targetted patch in the layout, perform replace operation on the DB
	Patch::ForDepthFirstWalk( source_layout, nullptr, [&](shared_ptr<Patch> &part)
	{
		if( auto replace_part = dynamic_pointer_cast<TargettedPatch>(part) )
		{
			auto source_free_zone = dynamic_pointer_cast<FreeZone>(replace_part->GetSourceZone());
			ASSERT( source_free_zone );
			db->MainTreeReplace( replace_part->GetTargetTreeZone(), *source_free_zone );
		}
	} );
}
