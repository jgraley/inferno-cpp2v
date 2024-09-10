#include "commands.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"

using namespace SR;

// ------------------------- Command --------------------------

string Command::OpName( int reg ) const
{
	if( reg==-1 )
		return "STACK";
	else
		return SSPrintf("Z%d", reg);
}


SSAAllocator::Reg Command::GetSourceReg() const
{
	Operands ops = GetOperandRegs();
	return OnlyElementOf( ops.sources );
}


SSAAllocator::Reg Command::GetTargetReg() const
{
	Operands ops = GetOperandRegs();
	return OnlyElementOf( ops.targets );
}


SSAAllocator::Reg Command::GetDestReg() const
{
	Operands ops = GetOperandRegs();
	return OnlyElementOf( ops.dests );
}

// ------------------------- DeclareFreeZoneCommand --------------------------

bool DeclareFreeZoneCommand::IsExpression() const
{
	return true;
}

DeclareFreeZoneCommand::DeclareFreeZoneCommand( FreeZone &&zone_ ) :
	zone(make_unique<FreeZone>(move(zone_)))
{
}	


void DeclareFreeZoneCommand::DetermineOperandRegs( SSAAllocator &allocator )
{
	dest_reg = allocator.Push();
}


Command::Operands DeclareFreeZoneCommand::GetOperandRegs() const
{
	return { {}, {}, {dest_reg} };
}


const FreeZone *DeclareFreeZoneCommand::GetFreeZone() const
{
    return zone.get();
}


void DeclareFreeZoneCommand::Execute( const ExecKit &kit ) const
{
	(*kit.register_file)[dest_reg] = make_unique<FreeZone>(*zone);
}


string DeclareFreeZoneCommand::GetTrace() const
{
	return "DeclareFreeZoneCommand "+Trace(*zone)+" -> "+OpName(dest_reg);
}

// ------------------------- DeclareTreeZoneCommand --------------------------

bool DeclareTreeZoneCommand::IsExpression() const
{
	return true;
}


DeclareTreeZoneCommand::DeclareTreeZoneCommand( const TreeZone &zone_ ) :
	zone( zone_ )
{
}


void DeclareTreeZoneCommand::DetermineOperandRegs( SSAAllocator &allocator )
{
	dest_reg = allocator.Push();
}


Command::Operands DeclareTreeZoneCommand::GetOperandRegs() const
{
	return { {}, {}, {dest_reg} };
}


const TreeZone *DeclareTreeZoneCommand::GetTreeZone() const
{
    return &zone;
}


void DeclareTreeZoneCommand::Execute( const ExecKit &kit ) const
{
	(*kit.register_file)[dest_reg] = make_unique<TreeZone>(zone);
}


string DeclareTreeZoneCommand::GetTrace() const
{
	return "DeclareTreeZoneCommand "+Trace(zone)+" -> "+OpName(dest_reg);
}

// ------------------------- DuplicateZoneCommand --------------------------

bool DuplicateZoneCommand::IsExpression() const
{
	return true;
}


void DuplicateZoneCommand::DetermineOperandRegs( SSAAllocator &allocator )
{
	source_reg = allocator.Pop();
	dest_reg = allocator.Push();
	ASSERT( source_reg != dest_reg ); // SSA
}


Command::Operands DuplicateZoneCommand::GetOperandRegs() const
{
	return { {source_reg}, {}, {dest_reg} };
}


void DuplicateZoneCommand::Execute( const ExecKit &kit ) const
{
	TreeZone &source_zone = dynamic_cast<TreeZone &>(*(*kit.register_file)[source_reg]);
    auto dest_zone = source_zone.Duplicate(kit.x_tree_db);
    (*kit.register_file)[dest_reg] = make_unique<FreeZone>(dest_zone);      
}


string DuplicateZoneCommand::GetTrace() const
{
	return "DuplicateZoneCommand "+OpName(source_reg)+ " -> "+OpName(dest_reg);
}

// ------------------------- JoinZoneCommand --------------------------

bool JoinZoneCommand::IsExpression() const
{
	return true;
}


JoinZoneCommand::JoinZoneCommand(int ti) :
    terminus_index(ti)
{
}


void JoinZoneCommand::DetermineOperandRegs( SSAAllocator &allocator )
{
	source_reg = allocator.Pop();
	target_reg = allocator.Peek();
}


Command::Operands JoinZoneCommand::GetOperandRegs() const
{
	return { {source_reg}, {target_reg}, {} };
}


void JoinZoneCommand::SetSourceReg( SSAAllocator::Reg reg )
{
	source_reg = reg;
}


void JoinZoneCommand::Execute( const ExecKit &kit ) const
{
	// Only free zones can be joined
	FreeZone source_zone = dynamic_cast<FreeZone &>(*(*kit.register_file)[source_reg]);
	FreeZone &target_zone = dynamic_cast<FreeZone &>(*(*kit.register_file)[target_reg]);
	target_zone.Join(source_zone, terminus_index);    
}


string JoinZoneCommand::GetTrace() const
{
	return "JoinZoneCommand " +
	       OpName(target_reg) +
	       SSPrintf("[%d]", terminus_index) +
	       " joins " +
  	       OpName(source_reg);

}

// ------------------------- UpdateTreeCommand --------------------------

bool UpdateTreeCommand::IsExpression() const
{
	return false;
}


void UpdateTreeCommand::DetermineOperandRegs( SSAAllocator &allocator )
{
	target_reg = allocator.Pop();
	source_reg = allocator.Pop();
}


Command::Operands UpdateTreeCommand::GetOperandRegs() const
{
	return { {source_reg}, {target_reg}, {} };
}


void UpdateTreeCommand::SetSourceReg( SSAAllocator::Reg reg )
{
	source_reg = reg;
}


void UpdateTreeCommand::Execute( const ExecKit &kit ) const
{
    // New zone must be a free zone
    auto target_tree_zone = dynamic_cast<TreeZone &>(*(*kit.register_file)[target_reg]);
    auto source_free_zone = dynamic_cast<FreeZone &>(*(*kit.register_file)[source_reg]);
    target_tree_zone.Update( kit.x_tree_db, source_free_zone );
}


string UpdateTreeCommand::GetTrace() const
{
	return "UpdateTreeCommand "+OpName(source_reg)+" over "+OpName(target_reg);
}

// ------------------------- MarkBaseForEmbeddedCommand --------------------------

bool MarkBaseForEmbeddedCommand::IsExpression() const
{
	return true;
}

MarkBaseForEmbeddedCommand::MarkBaseForEmbeddedCommand( RequiresSubordinateSCREngine *embedded_agent_ ) :
    embedded_agent( embedded_agent_ )
{
}
    
    
void MarkBaseForEmbeddedCommand::DetermineOperandRegs( SSAAllocator &allocator )
{
	source_reg = allocator.Peek();
}


Command::Operands MarkBaseForEmbeddedCommand::GetOperandRegs() const
{
	return { {source_reg}, {}, {} };
}


void MarkBaseForEmbeddedCommand::SetSourceReg( SSAAllocator::Reg reg )
{
	source_reg = reg;
}


void MarkBaseForEmbeddedCommand::Execute( const ExecKit &kit ) const
{
	Zone &zone = *(*kit.register_file)[source_reg];
	zone.MarkBaseForEmbedded(kit.scr_engine, embedded_agent);
}

    
string MarkBaseForEmbeddedCommand::GetTrace() const
{
	return "MarkBaseForEmbeddedCommand "+OpName(source_reg);
}

// ------------------------- CommandSequence --------------------------

bool CommandSequence::IsExpression() const
{
	ASSERT(!seq.empty());
	return seq.back()->IsExpression(); // like the comma operator
}

void CommandSequence::DetermineOperandRegs( SSAAllocator &allocator )
{
	for( const unique_ptr<Command> &cmd : seq )
		cmd->DetermineOperandRegs( allocator );
}


Command::Operands CommandSequence::GetOperandRegs() const
{
	return { {}, {}, {} };
}


void CommandSequence::Execute( const ExecKit &kit ) const
{
	//FTRACE(" executing");
	for( const unique_ptr<Command> &cmd : seq )
		cmd->Execute(kit);
}
	
    
void CommandSequence::Add( unique_ptr<Command> new_cmd )
{
    seq.push_back(move(new_cmd));
}


void CommandSequence::AddAtStart( unique_ptr<Command> new_cmd )
{
    seq.push_front(move(new_cmd));
}


bool CommandSequence::IsEmpty() const
{
	return seq.empty();
}


list<unique_ptr<Command>> &CommandSequence::GetCommands()
{
	return seq;
}


void CommandSequence::Clear()
{
	seq.clear();
}


string CommandSequence::GetTrace() const
{
    list<string> elts;
    for( const unique_ptr<Command> &pc : seq )
        elts.push_back( Trace(*pc) );
    return Join( elts, "\n", "CommandSequence[\n", " ]\n" );
}
