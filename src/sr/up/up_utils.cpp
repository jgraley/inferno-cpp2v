#include "up_utils.hpp"

#include "zone_expressions.hpp"
#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

// ------------------------- EmptyZoneElider --------------------------

EmptyZoneElider::EmptyZoneElider()
{
}
	

void EmptyZoneElider::Run( shared_ptr<Layout> &root_expr )
{
	Layout::ForDepthFirstWalk( root_expr, nullptr, [&](shared_ptr<Layout> &expr)
	{
		if( auto pz_op = dynamic_pointer_cast<MergeZoneOperator>(expr) )
            if( pz_op->GetZone().IsEmpty() )
            {
				shared_ptr<Layout> child_expr = OnlyElementOf( pz_op->GetChildExpressions() );
				if( auto child_pz_op = dynamic_pointer_cast<MergeZoneOperator>(child_expr) )
					child_pz_op->AddEmbeddedMarkers( pz_op->GetEmbeddedMarkers() );
				expr = child_expr;
			}
	} );	
}


void EmptyZoneElider::Check( shared_ptr<Layout> &root_expr )
{
	Layout::ForDepthFirstWalk( root_expr, nullptr, [&](shared_ptr<Layout> &expr)
	{
		if( auto pz_op = dynamic_pointer_cast<MergeZoneOperator>(expr) )
            ASSERT( !pz_op->GetZone().IsEmpty() )("Found empty zone in populate op: ")(pz_op->GetZone());
	} );	
}

// ------------------------- BaseForEmbeddedMarkPropagation --------------------------

BaseForEmbeddedMarkPropagation::BaseForEmbeddedMarkPropagation( const XTreeDatabase *db_ ) :
	db( db_ )
{
}


void BaseForEmbeddedMarkPropagation::Run( shared_ptr<Layout> &root_expr )
{
	TreeZoneRelation tz_relation( db );

	// Inner and outer loops only look at DupMergeTreeZoneOperator exprs
	Layout::ForDepthFirstWalk( root_expr, nullptr, [&](shared_ptr<Layout> &expr)
	{
		if( auto pz_op = dynamic_pointer_cast<MergeZoneOperator>(expr) )
		{	
			for( RequiresSubordinateSCREngine *agent : pz_op->GetEmbeddedMarkers() )
			{
				pz_op->GetZone().MarkBaseForEmbedded(agent);
			}
			pz_op->ClearEmbeddedMarkers();
		}
	} );
}

// ------------------------- DuplicateAllToFree --------------------------

DuplicateAllToFree::DuplicateAllToFree()
{
}
	

void DuplicateAllToFree::Run( shared_ptr<Layout> &root_expr )
{
	Layout::ForDepthFirstWalk( root_expr, nullptr, [&](shared_ptr<Layout> &expr)
	{
		if( auto ptz_op = dynamic_pointer_cast<DupMergeTreeZoneOperator>(expr) )
        {
			expr = ptz_op->DuplicateToFree();
		}
	} );	
}


void DuplicateAllToFree::Check( shared_ptr<Layout> &root_expr )
{
	Layout::ForDepthFirstWalk( root_expr, nullptr, [&](shared_ptr<Layout> &expr)
	{
		ASSERT( dynamic_pointer_cast<MergeFreeZoneOperator>(expr) );
	} );	
}


