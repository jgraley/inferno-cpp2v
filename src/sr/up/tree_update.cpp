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


unique_ptr<FreeZone> TreeUpdater::Evaluate( shared_ptr<const ZoneExpression> expr )
{
	UpEvalExecKit kit { nullptr };
	return expr->Evaluate(kit);   
}


void TreeUpdater::TransformToIncrementalAndExecute( TreeZone target_tree_zone, shared_ptr<ZoneExpression> source_layout )
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
	UpEvalExecKit kit { db };
	ZoneExpression::ForDepthFirstWalk( source_layout, nullptr, [&](shared_ptr<ZoneExpression> &part)
	{
		if( auto replace_part = dynamic_pointer_cast<ReplaceOperator>(part) )
			replace_part->Execute(kit);
	} );
}
