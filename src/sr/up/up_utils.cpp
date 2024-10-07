#include "up_utils.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "commands.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

// ------------------------- EmptyZoneElider --------------------------

EmptyZoneElider::EmptyZoneElider()
{
}
	

void EmptyZoneElider::Run( shared_ptr<ZoneExpression> &root_expr )
{
	ZoneExpression::ForDepthFirstWalk( root_expr, nullptr, [&](shared_ptr<ZoneExpression> &expr)
	{
		if( auto pz_op = dynamic_pointer_cast<PopulateZoneOperator>(expr) )
            if( pz_op->GetZone().IsEmpty() )
            {
				shared_ptr<ZoneExpression> child_expr = OnlyElementOf( pz_op->GetChildExpressions() );
				if( auto child_pz_op = dynamic_pointer_cast<PopulateZoneOperator>(child_expr) )
					child_pz_op->AddEmbeddedMarkers( pz_op->GetEmbeddedMarkers() );
				expr = child_expr;
			}
	} );	
}


void EmptyZoneElider::Check( shared_ptr<ZoneExpression> &root_expr )
{
	ZoneExpression::ForDepthFirstWalk( root_expr, nullptr, [&](shared_ptr<ZoneExpression> &expr)
	{
		if( auto pz_op = dynamic_pointer_cast<PopulateZoneOperator>(expr) )
            ASSERT( !pz_op->GetZone().IsEmpty() )("Found empty zone in populate op: ")(pz_op->GetZone());
	} );	
}

// ------------------------- ZoneMarkEnacter --------------------------

ZoneMarkEnacter::ZoneMarkEnacter( const XTreeDatabase *db_ ) :
	db( db_ )
{
}


void ZoneMarkEnacter::Run( shared_ptr<ZoneExpression> &root_expr )
{
	TreeZoneRelation tz_relation( db );

	// Inner and outer loops only look at PopulateTreeZoneOperator exprs
	ZoneExpression::ForDepthFirstWalk( root_expr, nullptr, [&](shared_ptr<ZoneExpression> &expr)
	{
		if( auto pz_op = dynamic_pointer_cast<PopulateZoneOperator>(expr) )
		{	
			for( RequiresSubordinateSCREngine *agent : pz_op->GetEmbeddedMarkers() )
			{
				pz_op->GetZone().MarkBaseForEmbedded(agent);
			}
			pz_op->ClearEmbeddedMarkers();
		}
	} );
}
