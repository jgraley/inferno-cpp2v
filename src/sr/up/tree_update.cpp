#include "tree_update.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "commands.hpp"
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


FreeZone TreeUpdater::Evaluate( const ZoneExpression *expr )
{
	unique_ptr<FreeZone> free_zone = expr->Evaluate();   
	return *free_zone;
}


void TreeUpdater::TransformToIncrementalAndExecute( shared_ptr<Command> initial_cmd )
{
	ASSERT( db );
	
	shared_ptr<ZoneExpression> expr = dynamic_cast<const UpdateTreeCommand &>(*initial_cmd).GetExpression();
	
	EmptyZoneElider().Run(expr);
	EmptyZoneElider().Check(expr);
	
	TreeZoneOverlapHandler( db ).Run(expr);
	TreeZoneOverlapHandler( db ).Check(expr);
	
	TreeZoneOrderingHandler( db ).Run(expr);
	TreeZoneOrderingHandler( db ).Check(expr);
	
	FreeZoneMerger().Run(expr);  // TODO fix me!!
	FreeZoneMerger().Check(expr);
	
	AltTreeZoneOrderingChecker( db ).Check(expr);

	if( ReadArgs::use_incremental )
	{
		// Enact the tree zones that will stick around
		ZoneMarkEnacter(db).Run(expr);

		// Inversion generates sequence of "small" update commands 
		TreeZoneInverter inverter( db ); 
		shared_ptr<Command> incremental_cmd = inverter.Run(initial_cmd);	
			
		FTRACE(incremental_cmd)("\n");		
			
		// Execute it
		incremental_cmd->Execute();   
	}
	else
	{
		initial_cmd->Execute();
	}
}
