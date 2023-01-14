#include "tree_update.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "commands.hpp"

using namespace SR;

// ------------------------- Runners --------------------------

FreeZone SR::RunGetFreeZoneNoDB( unique_ptr<Command> cmd, const SCREngine *scr_engine )
{
    stack<FreeZone> free_zone_stack;
    Command::ExecKit exec_kit {nullptr, scr_engine, scr_engine, &free_zone_stack};
	cmd->Execute( exec_kit );   
	ASSERT( free_zone_stack.size() == 1);       
    return free_zone_stack.top();  	
}


void SR::RunVoidForReplace( unique_ptr<Command> cmd, const SCREngine *scr_engine, XTreeDatabase *x_tree_db )
{
	// Ensure we have a CommandSequence
	auto seq = make_unique<CommandSequence>();
	seq->Add( move(cmd) ); 
	
	// flatten...
	CommandSequenceFlattener().Apply(*seq);
	
	// Uniqueness of tree zones
	TreeZoneOverlapFinder finder( x_tree_db, seq.get() );
	
	// calculate SSA indexes
	// err...
	
    stack<FreeZone> free_zone_stack;
    Command::ExecKit exec_kit {x_tree_db, x_tree_db, scr_engine, &free_zone_stack};
	seq->Execute( exec_kit );   
	ASSERT( free_zone_stack.size() == 0);       
}

// ------------------------- TreeZoneOverlapFinder --------------------------

TreeZoneOverlapFinder::TreeZoneOverlapFinder( const XTreeDatabase *db, CommandSequence *seq )
{
	// Put them all into one Overlapping set, pessamistically assuming they
	// all overlap
	
	for( const unique_ptr<Command> &cmd : seq->GetCommands() )
	{
		if( auto tz_cmd = dynamic_cast<const ImmediateTreeZoneCommand *>(cmd.get()) )
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
	
    ForAllCommutativeDistinctPairs( tzps_to_commands, 
                                    [&](const pair<const TreeZone *, const ImmediateTreeZoneCommand *> &l, 
                                        const pair<const TreeZone *, const ImmediateTreeZoneCommand *> &r)
    {
        if( TreeZone::IsOverlap( db, *l.first, *r.first ) )
        {
            // It's a symmentrical relationship so do it both ways around
            overlapping_zones[l.first].insert(r.first);
            overlapping_zones[r.first].insert(l.first);
        }
    } );
    
    //FTRACE(overlapping_zones);
}

// ------------------------- CommandSequenceFlattener --------------------------

void CommandSequenceFlattener::Apply( CommandSequence &seq )
{
	list<unique_ptr<Command>> commands( move(seq.GetCommands()) );
	
	Worker( seq, commands );
}


void CommandSequenceFlattener::Worker( CommandSequence &seq, list<unique_ptr<Command>> &commands )
{
	for( unique_ptr<Command> &cmd : commands )
	{
		ASSERT( cmd );
		if( auto sub_seq = dynamic_pointer_cast<CommandSequence>(cmd) )
		{
			Worker( seq, sub_seq->GetCommands() );
		}
		else
		{
			seq.Add( move(cmd) );
		}
	}
}

