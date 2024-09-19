#include "tree_update.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "commands.hpp"

#include <iostream>

using namespace SR;

// ------------------------- Runners --------------------------

FreeZone SR::RunForBuilder( const FreeZoneExpression *expr )
{
    UP::ExecKit exec_kit {nullptr, nullptr};
	unique_ptr<Zone> zone = expr->Evaluate( exec_kit );   
	if( auto free_zone = dynamic_pointer_cast<FreeZone>(zone) )
		return *free_zone;
	else
		ASSERTFAIL();
}


void SR::RunForReplace( const Command *cmd, const SCREngine *scr_engine, XTreeDatabase *x_tree_db )
{
	//FTRACE(cmd);
	// Uniqueness of tree zones
	const FreeZoneExpression *expr = dynamic_cast<const UpdateTreeCommand &>(*cmd).GetExpression();
	TRACE("\n-----------------------------------------------------------------------------------\n");
	TreeZoneOverlapFinder overlaps( x_tree_db, expr );
	ASSERT(overlaps.overlapping_zones.empty())(overlaps); // Temproary: usually true but obviously not always
	
	// err...
	
	// Execute it
    UP::ExecKit exec_kit {x_tree_db, scr_engine}; 
	cmd->Execute( exec_kit );   
}

// ------------------------- TreeZoneOverlapFinder --------------------------

TreeZoneOverlapFinder::TreeZoneOverlapFinder( const XTreeDatabase *db, const FreeZoneExpression *base )
{
	FreeZoneExpression::ForDepthFirstWalk( base, [&](const FreeZoneExpression *expr)
	{
		if( auto ptz_cmd = dynamic_cast<const PopulateTreeZoneOperator *>(expr) )
        {
            // Note that key is actually TreeZone *, so equal TreeZones get different 
            // rows which is why we InsertSolo()
            const TreeZone *zone = ptz_cmd->GetZone();
            
            // Zone should be known to the DB
            zone->DBCheck(db);
            
            // Record zone and related command
            InsertSolo( tzps_to_commands, make_pair( zone, ptz_cmd ) );
            
            // Start off with an empty overlapping set
            InsertSolo( overlapping_zones, make_pair( zone, set<const TreeZone *>() ) );
        }
	}, nullptr);
	
	// Find the actual overlaps and add to the sets
    ForAllUnorderedPairs( tzps_to_commands, 
                                    [&](const pair<const TreeZone *, const PopulateTreeZoneOperator *> &l, 
                                        const pair<const TreeZone *, const PopulateTreeZoneOperator *> &r)
    {
        if( TreeZone::IsOverlap( db, *l.first, *r.first ) )
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
