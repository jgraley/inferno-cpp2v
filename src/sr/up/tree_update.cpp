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


void SR::RunForReplace( const Command *cmd, const SCREngine *scr_engine, XTreeDatabase *x_tree_db )
{
	//FTRACE(cmd);
	// Uniqueness of tree zones
	shared_ptr<FreeZoneExpression> expr = dynamic_cast<const UpdateTreeCommand &>(*cmd).GetExpression();
	
	EmptyZoneElider().Run(expr);
	EmptyZoneElider().Check(expr);
	
	TreeZoneOverlapFinder overlaps( x_tree_db, expr );
	//ASSERT(overlaps.overlapping_zones.empty())(overlaps); // Temproary: usually true but obviously not always
	
	// err...
	
	// Execute it
    UP::ExecKit exec_kit {x_tree_db, scr_engine}; 
	cmd->Execute( exec_kit );   
}

// ------------------------- TreeZoneOverlapFinder --------------------------

TreeZoneOverlapFinder::TreeZoneOverlapFinder( const XTreeDatabase *db, shared_ptr<FreeZoneExpression> base )
{
	TreeZoneRelation tz_relation( db );
	
	FreeZoneExpression::ForDepthFirstWalk( base, [&](shared_ptr<FreeZoneExpression> &expr)
	{
		if( auto ptz_cmd = dynamic_pointer_cast<PopulateTreeZoneOperator>(expr) )
        {
            // Note that key is actually TreeZone *, so equal TreeZones get different 
            // rows which is why we InsertSolo()
            const TreeZone *zone = ptz_cmd->GetZone();
            
            // Zone should be known to the DB
            zone->DBCheck(db);
            
            // Record zone and related command
            InsertSolo( tzps_to_commands, make_pair( zone, ptz_cmd.get() ) );
            
            // Start off with an empty overlapping set
            InsertSolo( overlapping_zones, make_pair( zone, set<const TreeZone *>() ) );
        }
	}, nullptr);
	
	// Find the actual overlaps and add to the sets
    ForAllUnorderedPairs( tzps_to_commands, 
                                    [&](const pair<const TreeZone *, const PopulateTreeZoneOperator *> &l, 
                                        const pair<const TreeZone *, const PopulateTreeZoneOperator *> &r)
    {
		auto p = tz_relation.CompareHierarchical( *l.first, *r.first );
        if( p.second == ZoneRelation::OVERLAP_GENERAL || p.second == ZoneRelation::OVERLAP_TERMINII )
        {
            // It's a symmetrical relationship so do it both ways around
            overlapping_zones[l.first].insert(r.first);
            overlapping_zones[r.first].insert(l.first);
        }
    } );
          
    // Discard empty sets
	for (auto it = overlapping_zones.cbegin(); it != overlapping_zones.cend();)
	{
	    if (it->second.empty())	 
		    it = overlapping_zones.erase(it);
	    else
			it++;	  
	}			  
}


string TreeZoneOverlapFinder::GetTrace() const
{
	return Trace(overlapping_zones);
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
            if( pz_op->GetZone()->IsEmpty() )
            {
				shared_ptr<FreeZoneExpression> child_expr = OnlyElementOf( pz_op->GetChildExpressions() );
				if( auto child_pz_op = dynamic_pointer_cast<PopulateZoneOperator>(child_expr) )
					child_pz_op->AddEmbeddedMarkers( pz_op->GetEmbeddedMarkers() );
				expr = child_expr;
			}
	});	
}


void EmptyZoneElider::Check( shared_ptr<FreeZoneExpression> &base )
{
	FreeZoneExpression::ForDepthFirstWalk( base, nullptr, [&](shared_ptr<FreeZoneExpression> &expr)
	{
		if( auto pz_op = dynamic_pointer_cast<PopulateZoneOperator>(expr) )
            ASSERT( !pz_op->GetZone()->IsEmpty() )("Found empty zone in populate op: ")(*pz_op->GetZone());
	});	
}

