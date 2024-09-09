#include "tree_update.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "commands.hpp"

using namespace SR;

// ------------------------- Runners --------------------------

FreeZone SR::RunForBuilder( unique_ptr<Command> cmd )
{
	// Ensure we have a CommandSequence
	auto seq = make_unique<CommandSequence>();
	seq->Add( move(cmd) ); 
	
	// Flatten...
	CommandSequenceFlattener().Apply(*seq);
	
	// Calculate SSA indexes
	SSAAllocator ssa_allocator;
	seq->DetermineOperandRegs( ssa_allocator );
    SSAAllocator::Reg out_reg = ssa_allocator.Pop();

	// Elide empty zones
	EmptyZoneRemover().Apply(*seq);

    Command::RegisterFile register_file;
    Command::ExecKit exec_kit {nullptr, nullptr, &register_file};
	seq->Execute( exec_kit );   
	
	// We absolutely require a free zone
    return dynamic_cast<FreeZone &>(*register_file[out_reg]);
}


void SR::RunForReplace( unique_ptr<Command> cmd, const SCREngine *scr_engine, XTreeDatabase *x_tree_db )
{
	// Ensure we have a CommandSequence
	auto seq = make_unique<CommandSequence>();
	seq->Add( move(cmd) ); 
	
	// Flatten...
	CommandSequenceFlattener().Apply(*seq);
	
	// Calculate SSA indexes
	SSAAllocator ssa_allocator;
	seq->DetermineOperandRegs( ssa_allocator );
		
	// Elide empty zones
	EmptyZoneRemover().Apply(*seq);

	// Uniqueness of tree zones
	TreeZoneOverlapFinder finder( x_tree_db, seq.get() );
	
	// err...
	
    Command::RegisterFile register_file;    
    Command::ExecKit exec_kit {x_tree_db, scr_engine, &register_file};
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

// ------------------------- EmptyZoneRemover --------------------------

EmptyZoneRemover::OperandMaps EmptyZoneRemover::DetermineOpUsers( CommandSequence &seq )
{
	OperandMaps maps;
	for( unique_ptr<Command> &cmd : seq.GetCommands() )
	{
		Command::Operands ops = cmd->GetOperandRegs();
		for( SSAAllocator::Reg op : ops.sources )
			maps.as_source[op].insert(&cmd);
		for( SSAAllocator::Reg op : ops.targets )
			maps.as_target[op].insert(&cmd);
		for( SSAAllocator::Reg op : ops.dests )
			InsertSolo(maps.as_dest, make_pair(op, &cmd)); // SSA
	}
	return maps;
}


void EmptyZoneRemover::Apply( CommandSequence &seq )
{
	const OperandMaps maps = DetermineOpUsers(seq);
	set<SSAAllocator::Reg> empty_fz_regs;
	set<SSAAllocator::Reg> empty_tz_regs;
	set<unique_ptr<Command> *> commands_to_drop;
	
	for( unique_ptr<Command> &cmd : seq.GetCommands() )
	{
		if( auto tz_cmd = dynamic_cast<const DeclareTreeZoneCommand *>(cmd.get()) )
		{
			if( tz_cmd->GetTreeZone()->IsEmpty() ) 
			{
			    empty_tz_regs.insert( tz_cmd->GetDestReg() );
			    commands_to_drop.insert(&cmd);
			}
		} 
		else if( auto fz_cmd = dynamic_cast<const DeclareFreeZoneCommand *>(cmd.get()) )
		{
			if( fz_cmd->GetFreeZone()->IsEmpty() )
			{ 
			    empty_fz_regs.insert( fz_cmd->GetDestReg() );
			    commands_to_drop.insert(&cmd);
			}
		}
	}
	
	for( SSAAllocator::Reg reg : empty_tz_regs )
	{
		for( unique_ptr<Command> *cmd : maps.as_source.at(reg) )
		{
			if( auto d_cmd = dynamic_cast<const DuplicateZoneCommand *>(cmd->get()) )
			{
				empty_fz_regs.insert( d_cmd->GetDestReg() );
				commands_to_drop.insert(cmd);			
			}
			else
			{
				FTRACE("empty tree zone reg Z%d used as source in ", reg)(*cmd);
				ASSERTFAIL(); // empty treezone went into something unexpected
			}			
		}
	}
	
	for( SSAAllocator::Reg reg : empty_fz_regs )
	{		
		// This is the join that we will elide, because the target zone is
		// empty and the join ends up replacing it.
		// Can only cope with one usage as target because an EMPTY free zone 
		// should only have one terminus and each terminus is only in one join.
		unique_ptr<Command> *cmd_target = OnlyElementOf( maps.as_target.at(reg) );
		auto j_cmd_target = dynamic_cast<JoinZoneCommand *>(cmd_target->get());
		ASSERT( j_cmd_target );
		commands_to_drop.insert(cmd_target);			
		
		// Usages of the empty zone to be updated to bypass it and go to 
		// whichever zone was going to be joined to it.
		SSAAllocator::Reg new_source_reg = j_cmd_target->GetSourceReg();
		for( unique_ptr<Command> *cmd_source : maps.as_source.at(reg) )
		{
			if( auto j_cmd_source = dynamic_cast<JoinZoneCommand *>(cmd_source->get()) )
			{
				j_cmd_source->SetSourceReg( new_source_reg );
			}
			else if( auto b_cmd_source = dynamic_cast<MarkBaseForEmbeddedCommand *>(cmd_source->get()) )
			{
				b_cmd_source->SetSourceReg( new_source_reg );				
			}
			else if( auto m_cmd_source = dynamic_cast<UpdateTreeCommand *>(cmd_source->get()) )
			{
				m_cmd_source->SetSourceReg( new_source_reg );				
			}
			else
			{
				FTRACE(seq);
				ASSERT(false)("Reg Z%d used in ", reg)(cmd_source); // don't know about this command
			}
		}		
	}
	
	for( unique_ptr<Command> * cmd : commands_to_drop )
		seq.GetCommands().remove( *cmd );
}

