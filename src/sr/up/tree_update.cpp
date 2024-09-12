#include "tree_update.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "commands.hpp"

using namespace SR;

// ------------------------- Runners --------------------------

FreeZone SR::Evaluate( unique_ptr<Command> cmd, const Command::EvalKit &eval_kit )
{
	ASSERT( cmd->IsExpression() );

	// Ensure we have a CommandSequence
	auto seq = make_unique<CommandSequence>();
	seq->Add( move(cmd) ); 
	
	// Flatten...
	CommandSequenceFlattener().Apply(*seq);
	
	ASSERT( seq->IsExpression() );

	// Calculate SSA indexes
	SSAAllocator ssa_allocator;
	seq->DetermineOperandRegs( ssa_allocator );
    SSAAllocator::Reg out_reg = ssa_allocator.Pop();

    Command::RegisterFile register_file;
    Command::ExecKit exec_kit;
    exec_kit.register_file = &register_file;
    (Command::EvalKit &)exec_kit = eval_kit; 
	seq->Execute( exec_kit );   
	
	// We absolutely require a free zone
    return dynamic_cast<FreeZone &>(*register_file[out_reg]);
}


FreeZone SR::RunForBuilder( unique_ptr<Command> cmd )
{
	ASSERT( cmd->IsExpression() );

    Command::EvalKit eval_kit {nullptr, nullptr};
    
    return Evaluate(move(cmd), eval_kit);
}


void SR::RunForReplace( unique_ptr<Command> cmd, const SCREngine *scr_engine, XTreeDatabase *x_tree_db )
{
	ASSERT( !cmd->IsExpression() );

	// Ensure we have a CommandSequence
	auto seq = make_unique<CommandSequence>();
	seq->Add( move(cmd) ); 
	
	// Flatten...
	CommandSequenceFlattener().Apply(*seq);
	
	ASSERT( !seq->IsExpression() );
	
	// Calculate SSA indexes
	SSAAllocator ssa_allocator;
	seq->DetermineOperandRegs( ssa_allocator );

	// Uniqueness of tree zones
	//TreeZoneOverlapFinder finder( x_tree_db, seq.get() );
	
	// err...
	
    Command::RegisterFile register_file;    
    Command::ExecKit exec_kit;
    exec_kit.register_file = &register_file;
    (Command::EvalKit &)exec_kit = Command::EvalKit{x_tree_db, scr_engine}; 
	seq->Execute( exec_kit );   
}

// ------------------------- TreeZoneOverlapFinder --------------------------

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

