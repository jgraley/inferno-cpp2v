#include "tree_update.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "commands.hpp"
#include "tz_relation.hpp"

#include <iostream>

using namespace SR;

// ------------------------- Runners --------------------------

FreeZone SR::RunForBuilder( const FreeZoneExpression *expr )
{
    UP::ExecKit exec_kit {nullptr, nullptr};
	unique_ptr<FreeZone> free_zone = expr->Evaluate( exec_kit );   
	return *free_zone;
}


void SR::RunForReplace( const Command *initial_cmd, const SCREngine *scr_engine, XTreeDatabase *x_tree_db )
{

	shared_ptr<FreeZoneExpression> expr = dynamic_cast<const UpdateTreeCommand &>(*initial_cmd).GetExpression();
	
	EmptyZoneElider().Run(expr);
	EmptyZoneElider().Check(expr);
	
	TreeZoneOverlapFinder( x_tree_db ).Run(expr);
	TreeZoneOverlapFinder( x_tree_db ).Check(expr);
	
	// TODO deal with out-of-sequence (DF) tree zones
	
	// TODO enact the markers (I think)
	
	// TODO maximally merge free zones and tree zones
		
	// TODO reductive inversion using Quark algo
	
	// TODO merge TZs again and check initial update command is now trivial
	
	// Execute it
    UP::ExecKit exec_kit {x_tree_db, scr_engine}; 
	initial_cmd->Execute( exec_kit );   
}

// ------------------------- TreeZoneOverlapFinder --------------------------

TreeZoneOverlapFinder::TreeZoneOverlapFinder( const XTreeDatabase *db_ ) :
	db( db_ )
{
}


void TreeZoneOverlapFinder::Run( shared_ptr<FreeZoneExpression> &base )
{
	TreeZoneRelation tz_relation( db );

	// Inner and outer loops only look at PopulateTreeZoneOperator exprs
	FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &l_expr)
	{
		if( auto l_ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(l_expr) )
		{			
			// We will establish an increasing region of known non-overlapping tree zones. Detect
			// when the new l has an overlap in that zone.
			bool l_has_an_overlap = false;
			FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &r_expr)
			{
				if( auto r_ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(r_expr) )
				{			
					if( l_expr == r_expr ) // inner "r" loop stops before catching up with outer "l" loop
						Break();
					
					auto p = tz_relation.CompareHierarchical( l_ptz_op->GetZone(), r_ptz_op->GetZone() );

					// Act on any overlap including equality. 
					if( p.second == ZoneRelation::OVERLAP_GENERAL || 
						p.second == ZoneRelation::OVERLAP_TERMINII ||
						p.second == ZoneRelation::EQUAL )
					{
						l_has_an_overlap = true;
					}
				}
			} );
			// If it does, duplicate it, which turns it into a free zone, so it will not be seen
			// in future runs of the inner "r" loop.
			if( l_has_an_overlap )
			{				
				l_expr = l_ptz_op->DuplicateToFree();
			}	
        }
	} );	
}


void TreeZoneOverlapFinder::Check( shared_ptr<FreeZoneExpression> &base )
{
	TreeZoneRelation tz_relation( db );

	FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &l_expr)
	{
		if( auto l_ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(l_expr) )
		{			

			FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &r_expr)
			{
				if( auto r_ptz_op = dynamic_pointer_cast<PopulateTreeZoneOperator>(r_expr) )
				{			
					if( l_expr == r_expr ) 
						Break();
					
					auto p = tz_relation.CompareHierarchical( l_ptz_op->GetZone(), r_ptz_op->GetZone() );					
					if( p.second == ZoneRelation::OVERLAP_GENERAL || 
						p.second == ZoneRelation::OVERLAP_TERMINII ||
						p.second == ZoneRelation::EQUAL )
					{
						ASSERT(false)("Tree zone overlap: ")(l_ptz_op->GetZone())(" and ")(r_ptz_op->GetZone());
					}
				}
			} );
        }
	} );	
}

// ------------------------- EmptyZoneElider --------------------------

EmptyZoneElider::EmptyZoneElider()
{
}
	

void EmptyZoneElider::Run( shared_ptr<FreeZoneExpression> &base )
{
	FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &expr)
	{
		if( auto pz_op = dynamic_pointer_cast<PopulateZoneOperator>(expr) )
            if( pz_op->GetZone().IsEmpty() )
            {
				shared_ptr<FreeZoneExpression> child_expr = OnlyElementOf( pz_op->GetChildExpressions() );
				if( auto child_pz_op = dynamic_pointer_cast<PopulateZoneOperator>(child_expr) )
					child_pz_op->AddEmbeddedMarkers( pz_op->GetEmbeddedMarkers() );
				expr = child_expr;
			}
	} );	
}


void EmptyZoneElider::Check( shared_ptr<FreeZoneExpression> &base )
{
	FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &expr)
	{
		if( auto pz_op = dynamic_pointer_cast<PopulateZoneOperator>(expr) )
            ASSERT( !pz_op->GetZone().IsEmpty() )("Found empty zone in populate op: ")(pz_op->GetZone());
	} );	
}

