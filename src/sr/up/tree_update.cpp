#include "tree_update.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "zone_commands.hpp"
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


void TreeUpdater::TransformToIncrementalAndExecute( shared_ptr<Command> initial_cmd )
{
	ASSERT( db );
	
	shared_ptr<ZoneExpression> expr = dynamic_cast<const ReplaceCommand &>(*initial_cmd).GetExpression();
	
	EmptyZoneElider empty_zone_elider;
	empty_zone_elider.Run(expr);
	empty_zone_elider.Check(expr);
	
	TreeZoneOverlapHandler tree_zone_overlap_handler( db );
	tree_zone_overlap_handler.Run(expr);
	tree_zone_overlap_handler.Check(expr);
	
	TreeZoneOrderingHandler tree_zone_ordering_handler( db );
	tree_zone_ordering_handler.Run(expr);
	tree_zone_ordering_handler.Check(expr);
	
	FreeZoneMerger free_zone_merger;
	free_zone_merger.Run(expr);  
	free_zone_merger.Check(expr);
	
	AltTreeZoneOrderingChecker alt_free_zone_ordering_checker( db );
	alt_free_zone_ordering_checker.Check(expr);

	ZoneMarkEnacter zone_mark_enacter( db );
	TreeZoneInverter tree_zone_inverter( db ); 

	// Enact the tree zones that will stick around
	zone_mark_enacter.Run(expr);

	// Inversion generates sequence of separate "small" update commands 
	shared_ptr<Command> incremental_cmd = tree_zone_inverter.Run(initial_cmd);	
						
	// Execute it
	UpEvalExecKit kit { db };
	incremental_cmd->Execute(kit);   
}
