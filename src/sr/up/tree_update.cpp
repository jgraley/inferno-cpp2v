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
	//TreeZoneOverlapFinder finder( x_tree_db, cmd.get() );
	
	// err...
	
	// Execute it
    Command::ExecKit exec_kit {x_tree_db, scr_engine}; 
	cmd->Execute( exec_kit );   
}

// ------------------------- TreeZoneOverlapFinder --------------------------
/*
TreeZoneOverlapFinder::TreeZoneOverlapFinder( const XTreeDatabase *db, CommandSequence *seq )
{
	// Put them all into one Overlapping set, pessamistically assuming they
	// all overlap
	
	for( const unique_ptr<Command> &cmd : seq->GetCommands() )
	{
		if( auto tz_cmd = dynamic_cast<const DeclareTreeZoneCommand *>(cmd.get()) )
        {
            // Note that key is actually TreeZone *, so equal TreeZones get different 
            // rows which is why we InsertSolo()
            const TreeZone *zone = tz_cmd->GetTreeZone();
            
            // Zone should be known to the DB
            zone->DBCheck(db);
            
            // Record zone and related command
            InsertSolo( tzps_to_commands, make_pair( zone, tz_cmd ) );
            
            // Start off with an empty overlapping set
            InsertSolo( overlapping_zones, make_pair( zone, set<const TreeZone *>() ) );
        }
	}
	
    ForAllUnorderedPairs( tzps_to_commands, 
                                    [&](const pair<const TreeZone *, const DeclareTreeZoneCommand *> &l, 
                                        const pair<const TreeZone *, const DeclareTreeZoneCommand *> &r)
    {
        if( TreeZone::IsOverlap( db, *l.first, *r.first ) )
        {
            // It's a symmentrical relationship so do it both ways around
            overlapping_zones[l.first].insert(r.first);
            overlapping_zones[r.first].insert(l.first);
        }
    } );
    
    //FTRACE(overlapping_zones);
}*/
