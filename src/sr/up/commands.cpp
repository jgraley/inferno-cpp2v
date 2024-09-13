#include "commands.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tree_update.hpp"

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

// ------------------------- PopulateZoneCommand --------------------------

bool PopulateZoneCommand::IsExpression() const
{
	return true;
}


PopulateZoneCommand::PopulateZoneCommand( unique_ptr<Zone> &&zone_, vector<unique_ptr<Command>> &&child_expressions_ ) :
	zone(move(zone_)),
	child_expressions(move(child_expressions_))
{
	ASSERT( zone->GetNumTerminii() == child_expressions.size() );
}	


PopulateZoneCommand::PopulateZoneCommand( unique_ptr<Zone> &&zone_ ) :
	zone(move(zone_))
{
	ASSERT( zone->GetNumTerminii() == 0 );
}	


void PopulateZoneCommand::AddEmbeddedAgentBase( RequiresSubordinateSCREngine *embedded_agent )
{
	embedded_agents.push_back( embedded_agent );
}


void PopulateZoneCommand::DetermineOperandRegs( SSAAllocator &allocator ) const
{
	dest_reg = allocator.Push();
}


Command::Operands PopulateZoneCommand::GetOperandRegs() const
{
	return { {}, {}, {dest_reg} };
}


const Zone *PopulateZoneCommand::GetZone() const
{
    return zone.get();
}


void PopulateZoneCommand::Execute( const ExecKit &kit ) const
{
	//FTRACE(zone)("\n");
	FreeZone free_zone;
	if( auto fzp = dynamic_cast<FreeZone *>(zone.get()) )
	{
		free_zone = *fzp;
	}
	else if( auto tzp = dynamic_cast<TreeZone *>(zone.get()) )
	{
		free_zone = tzp->Duplicate( kit.x_tree_db );
	}
	else
	{
		ASSERTFAIL();
	}		
	
	//FTRACE(free_zone)("\n");
	vector<FreeZone> child_zones;
	for( const unique_ptr<Command> &child_expression : child_expressions )
	{
		//FTRACE(child_expression)("\n");
		child_zones.push_back( Evaluate( child_expression.get(), kit ) );	
	}
	
	free_zone.Populate(kit.x_tree_db, child_zones);
	
	for( RequiresSubordinateSCREngine *ea : embedded_agents )
		free_zone.MarkBaseForEmbedded(kit.scr_engine, ea);
	
	(*kit.register_file)[dest_reg] = make_unique<FreeZone>(free_zone);
}


string PopulateZoneCommand::GetTrace() const
{
	return "PopulateZoneCommand "+Trace(*zone)+" -> "+OpName(dest_reg);
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


void DeclareTreeZoneCommand::DetermineOperandRegs( SSAAllocator &allocator ) const
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

// ------------------------- UpdateTreeCommand --------------------------

bool UpdateTreeCommand::IsExpression() const
{
	return false;
}


void UpdateTreeCommand::DetermineOperandRegs( SSAAllocator &allocator ) const
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

// ------------------------- CommandSequence --------------------------

bool CommandSequence::IsExpression() const
{
	ASSERT(!seq.empty());
	return seq.back()->IsExpression(); // like the comma operator
}

void CommandSequence::DetermineOperandRegs( SSAAllocator &allocator ) const
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
