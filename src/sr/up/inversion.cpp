#include "inversion.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "commands.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

// TODO


TreeZoneInverter::TreeZoneInverter( const Command *initial_cmd, const XTreeDatabase *db_ ) :
	db( db_ ),
	root_update_cmd( dynamic_cast<const UpdateTreeCommand *>(initial_cmd) ),
	root_expr( root_update_cmd->GetExpression() ),
	root_tree_zone( root_update_cmd->GetTargetTreeZone() )
{
}


void TreeZoneInverter::Run()
{
	
	while( TryInvertOne() ) {}	
}


const CommandSequence &TreeZoneInverter::GetSplitSeq()
{
	return split_seq;
}


bool TreeZoneInverter::TryInvertOne()
{
	return false;
}

