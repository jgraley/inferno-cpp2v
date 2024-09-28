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
	
	// TODO enact free zone markers (I think)

	EmptyZoneElider().Run(expr);
	EmptyZoneElider().Check(expr);
	
	//FreeZoneMerger().Run(expr); // TODO should work once FZ markers enacted
	//FreeZoneMerger().Check(expr);

	TreeZoneOverlapHandler( x_tree_db ).Run(expr);
	TreeZoneOverlapHandler( x_tree_db ).Check(expr);
	
	// TODO deal with out-of-sequence (DF) tree zones
	// Hand-code a walker which focusses on tree zones. Merge should ensure that
	// we can get to child tree zones after ZERO or ONE layer, use an "if" for
	// these cases and collect child TZs into a list. Or remove dependency on
	// FZ merge by finding that list using a walk.
	// On the way in:
	// - maintain an "acceptable DF range" - anywhere at root, but under tree zones
	//   it's determined by terminii. The end of the last acceptable range is TBD.
	// - simplest algo: when a child TZ is found out of acceptable range, just duplicate
	//   every TZ under it (use ForDFWalk) 
	// - When we have multiple TZs we also have to check that they are in order
	//   relative to each other. Use bases. This is a linear ordering problem and
	//   it's good to begin by finding correctly ordered subsequences.
	
	// TODO enact tree zone markers (I think)
			
	// TODO reductive inversion using Quark algo
	
	// TODO merge tree zones and check initial update command is now trivial
	
	// Execute it
    UP::ExecKit exec_kit {x_tree_db, scr_engine}; 
	initial_cmd->Execute( exec_kit );   
}

// ------------------------- TreeZoneOverlapHandler --------------------------

TreeZoneOverlapHandler::TreeZoneOverlapHandler( const XTreeDatabase *db_ ) :
	db( db_ )
{
}


void TreeZoneOverlapHandler::Run( shared_ptr<FreeZoneExpression> &base )
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
			// If it overlaps with something, duplicate it, which turns it into a free zone, so it will 
			// not be seen in future runs of the inner "r" loop.
			if( l_has_an_overlap )
			{				
				l_expr = l_ptz_op->DuplicateToFree();
			}	
        }
	} );	
}


void TreeZoneOverlapHandler::Check( shared_ptr<FreeZoneExpression> &base )
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

// ------------------------- FreeZoneMerger --------------------------

FreeZoneMerger::FreeZoneMerger()
{
}
	

void FreeZoneMerger::Run( shared_ptr<FreeZoneExpression> &base )
{
	FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &expr)
	{
		if( auto pz_op = dynamic_pointer_cast<PopulateFreeZoneOperator>(expr) )
        {
			FreeZone &free_zone = pz_op->GetZone();
			ASSERT( !free_zone.IsEmpty() );

			FreeZone::TerminusIterator it_t = free_zone.GetTerminiiBegin();
			pz_op->ForChildren([&](shared_ptr<FreeZoneExpression> &child_expr)
			{
				ASSERT( it_t != free_zone.GetTerminiiEnd() ); // length mismatch		
				if( auto child_pz_op = dynamic_pointer_cast<PopulateFreeZoneOperator>(child_expr) )
				{	
					FreeZone &child_free_zone = child_pz_op->GetZone();
					// Presently only able to represent markers at base of zone, but if a child
					// expr has one, after populating we'd need to put it our zone at some other
					// location.
					ASSERT( child_pz_op->GetEmbeddedMarkers().empty() );
					
					it_t = free_zone.PopulateTerminus( it_t, make_unique<FreeZone>(child_free_zone) );		
				}	
				else
				{
					it_t++;
				}						
			} );
			ASSERT( it_t == free_zone.GetTerminiiEnd() ); // length mismatch	
		}
	} );			
}


void FreeZoneMerger::Check( shared_ptr<FreeZoneExpression> &base )
{
	FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &expr)
	{
		if( auto pz_op = dynamic_pointer_cast<PopulateFreeZoneOperator>(expr) )
        {
			pz_op->ForChildren([&](shared_ptr<FreeZoneExpression> &child_expr)
			{
				if( auto child_pz_op = dynamic_pointer_cast<PopulateFreeZoneOperator>(child_expr) )
					ASSERT(false)("Free zone ")(*expr)(" touching another free zone ")(*child_expr);
			} );
		}
	} );		
}


