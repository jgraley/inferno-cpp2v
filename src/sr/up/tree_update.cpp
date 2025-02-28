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

#include <iostream>


using namespace SR;

// ------------------------- Runners --------------------------

TreeUpdater::TreeUpdater(XTreeDatabase *x_tree_db) :
    db( x_tree_db )
{
}


unique_ptr<FreeZone> TreeUpdater::TransformToSingleFreeZone( shared_ptr<Layout> source_layout )
{
	DuplicateAllToFree all_to_free;
	all_to_free.Run(source_layout);  
	all_to_free.Check(source_layout);
	
	FreeZoneMerger free_zone_merger;
	free_zone_merger.Run(source_layout);  
	free_zone_merger.Check(source_layout);

	auto pfz_op = dynamic_pointer_cast<MergeFreeZoneOperator>(source_layout);
	ASSERT( pfz_op );
	ASSERT( pfz_op->GetNumChildExpressions() == 0 );
	FreeZone free_zone = pfz_op->GetZone();
	ASSERT( free_zone.GetNumTerminii() == 0 );
	return make_unique<FreeZone>(free_zone);
}


void TreeUpdater::TransformToIncrementalAndExecute( TreeZone target_tree_zone, shared_ptr<Layout> source_layout )
{
	ASSERT( db );
		
	EmptyZoneElider empty_zone_elider;
	empty_zone_elider.Run(source_layout);
	empty_zone_elider.Check(source_layout);
	
	TreeZoneOverlapHandler tree_zone_overlap_handler( db );
	tree_zone_overlap_handler.Run(source_layout);
	tree_zone_overlap_handler.Check(source_layout);
	
	TreeZoneOrderingHandler tree_zone_ordering_handler( db );
	tree_zone_ordering_handler.Run(source_layout);
	tree_zone_ordering_handler.Check(source_layout);
	
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
	tree_zone_inverter.Run(target_tree_zone, &source_layout);	
			
	// Execute it
	Layout::ForDepthFirstWalk( source_layout, nullptr, [&](shared_ptr<Layout> &part)
	{
		if( auto replace_part = dynamic_pointer_cast<ReplaceOperator>(part) )
		{
			auto source_free_zone = dynamic_pointer_cast<FreeZone>(replace_part->GetSourceZone());
			ASSERT( source_free_zone );
			db->MainTreeReplace( replace_part->GetTargetTreeZone(), *source_free_zone );
		}
	} );
}
