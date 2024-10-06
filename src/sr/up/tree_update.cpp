#include "tree_update.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "commands.hpp"
#include "tz_relation.hpp"
#include "empty_zone.hpp"
#include "tz_overlap.hpp"
#include "tz_ordering.hpp"
#include "fz_merge.hpp"
#include "inversion.hpp"

#include <iostream>

using namespace SR;

// ------------------------- Runners --------------------------

FreeZone SR::RunForBuilder( const ZoneExpression *expr )
{
	unique_ptr<FreeZone> free_zone = expr->Evaluate();   
	return *free_zone;
}


void SR::RunForReplace( const Command *initial_cmd, XTreeDatabase *x_tree_db )
{
	shared_ptr<ZoneExpression> expr = dynamic_cast<const UpdateTreeCommand &>(*initial_cmd).GetExpression();
	
	EmptyZoneElider().Run(expr);
	EmptyZoneElider().Check(expr);
	
	TreeZoneOverlapHandler( x_tree_db ).Run(expr);
	TreeZoneOverlapHandler( x_tree_db ).Check(expr);
	
	TreeZoneOrderingHandler( x_tree_db ).Run(expr);
	TreeZoneOrderingHandler( x_tree_db ).Check(expr);
	
	FreeZoneMerger().Run(expr);  // TODO fix me!!
	FreeZoneMerger().Check(expr);
	
	AltTreeZoneOrderingChecker( x_tree_db ).Check(expr);

	// TODO enact tree zone markers (here or in DB)

	TreeZoneInverter inverter( initial_cmd, x_tree_db );
	inverter.Run();
	// TODO reductive inversion using Quark algo	

	// TODO merge tree zones and check initial update command is now trivial
	
	// Execute it
	initial_cmd->Execute();   
}
