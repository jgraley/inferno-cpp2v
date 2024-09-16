#include "tree_update.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "commands.hpp"

using namespace SR;

// ------------------------- Runners --------------------------

FreeZone SR::RunForBuilder( const Command *cmd )
{
	ASSERT( cmd->IsExpression() );

    Command::ExecKit exec_kit {nullptr, nullptr};
	unique_ptr<Zone> zone = cmd->Evaluate( exec_kit );   
	if( auto free_zone = dynamic_pointer_cast<FreeZone>(zone) )
		return *free_zone;
	else
		ASSERTFAIL();
}


void SR::RunForReplace( const Command *cmd, const SCREngine *scr_engine, XTreeDatabase *x_tree_db )
{
	ASSERT( !cmd->IsExpression() );

	// Uniqueness of tree zones
	const Command *expr = dynamic_cast<const UpdateTreeCommand &>(*cmd).GetExpression();
	TreeZoneOverlapFinder overlaps( x_tree_db, expr );
	FTRACE(overlaps);
	
	// err...
	
	// Execute it
    Command::ExecKit exec_kit {x_tree_db, scr_engine}; 
	cmd->Execute( exec_kit );   
}

// ------------------------- TreeZoneOverlapFinder --------------------------

TreeZoneOverlapFinder::TreeZoneOverlapFinder( const XTreeDatabase *db, const Command *cmd )
{
	// Put them all into one Overlapping set, pessamistically assuming they
	// all COULD overlap	
	cmd->ForWalk( [&](const Command *c)
	{
		if( auto tz_cmd = dynamic_cast<const PopulateTreeZoneCommand *>(c) )
        {
            // Note that key is actually TreeZone *, so equal TreeZones get different 
            // rows which is why we InsertSolo()
            const TreeZone *zone = tz_cmd->GetZone();
            
            // Zone should be known to the DB
            zone->DBCheck(db);
            
            // Record zone and related command
            InsertSolo( tzps_to_commands, make_pair( zone, tz_cmd ) );
            
            // Start off with an empty overlapping set
            InsertSolo( overlapping_zones, make_pair( zone, set<const TreeZone *>() ) );
        }
	}, nullptr);
	
	// Find the actual overlaps and add to the sets
    ForAllUnorderedPairs( tzps_to_commands, 
                                    [&](const pair<const TreeZone *, const PopulateTreeZoneCommand *> &l, 
                                        const pair<const TreeZone *, const PopulateTreeZoneCommand *> &r)
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
