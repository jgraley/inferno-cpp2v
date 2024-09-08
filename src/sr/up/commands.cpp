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
	TreeZone &zone = dynamic_cast<TreeZone &>(*(*kit.register_file)[source_reg]);
    ASSERT( !zone.IsEmpty() ); // Need to elide empty zones before executing
	
	if( kit.x_tree_db )
		zone.DBCheck(kit.x_tree_db);
	
    // Iterate over terminii and operand zones together, filling the map for
    // DuplicateSubtree() to use.
    Duplicate::TerminiiMap duplicator_terminus_map;
    for( XLink terminus_upd : zone.GetTerminusXLinks() ) 
        duplicator_terminus_map[terminus_upd] = { TreePtr<Node>(), shared_ptr<Updater>() };

    // Duplicate the subtree, populating from the map.
    TreePtr<Node> new_base_x = Duplicate::DuplicateSubtree( kit.x_tree_db, 
                                                            zone.GetBaseXLink(), 
                                                            duplicator_terminus_map );   
    
    vector<shared_ptr<Updater>> free_zone_terminii;
    for( XLink terminus_upd : zone.GetTerminusXLinks() )
    {
		ASSERT( duplicator_terminus_map[terminus_upd].updater );
        free_zone_terminii.push_back( duplicator_terminus_map[terminus_upd].updater );
	}

    // Create a new zone for the result.
    auto result_zone = FreeZone( new_base_x, free_zone_terminii );
    (*kit.register_file)[dest_reg] = make_unique<FreeZone>(result_zone);      
}


string DuplicateZoneCommand::GetTrace() const
{
	return "DuplicateZoneCommand "+OpName(source_reg)+ " -> "+OpName(dest_reg);
}

// ------------------------- JoinZoneCommand --------------------------

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
    ASSERT( !source_zone.IsEmpty() );
	FreeZone &target_zone = dynamic_cast<FreeZone &>(*(*kit.register_file)[target_reg]);
    ASSERT( !target_zone.IsEmpty() ); // Need to elide empty zones before executing	    

    shared_ptr<Updater> terminus_upd = target_zone.GetTerminusUpdater(terminus_index);
    target_zone.DropTerminus(terminus_index);
    
    // Populate terminus. Apply() will expand SubContainers
    ASSERT( source_zone.GetBaseNode() );
    terminus_upd->Apply( source_zone.GetBaseNode() );
    
    //Validate()( zone->GetBaseNode() );     
}


string JoinZoneCommand::GetTrace() const
{
	return "JoinZoneCommand " +
	       OpName(target_reg) +
	       SSPrintf("[%d]", terminus_index) +
	       " joins " +
  	       OpName(source_reg);

}

// ------------------------- ModifyTreeCommand --------------------------

void ModifyTreeCommand::DetermineOperandRegs( SSAAllocator &allocator )
{
	target_reg = allocator.Pop();
	source_reg = allocator.Pop();
}


Command::Operands ModifyTreeCommand::GetOperandRegs() const
{
	return { {source_reg}, {target_reg}, {} };
}


void ModifyTreeCommand::SetSourceReg( SSAAllocator::Reg reg )
{
	source_reg = reg;
}


void ModifyTreeCommand::Execute( const ExecKit &kit ) const
{
    // New zone must be a free zone
    auto target_tree_zone = dynamic_cast<TreeZone &>(*(*kit.register_file)[target_reg]);
    auto source_free_zone = dynamic_cast<FreeZone &>(*(*kit.register_file)[source_reg]);
	ASSERT( target_tree_zone.GetNumTerminii() == source_free_zone.GetNumTerminii() );
	
	ASSERT( target_tree_zone.GetNumTerminii() == 0 ); // TODO under #723
    
    // Update database 
    kit.x_tree_db->Delete( target_tree_zone.GetBaseXLink() );    
    
    // Patch the tree
    target_tree_zone.GetBaseXLink().SetXPtr( source_free_zone.GetBaseNode() );
    
    // Update database 
    kit.x_tree_db->Insert( target_tree_zone.GetBaseXLink() );      
}


string ModifyTreeCommand::GetTrace() const
{
	return "ModifyTreeCommand "+OpName(source_reg)+" over "+OpName(target_reg);
}

// ------------------------- MarkBaseForEmbeddedCommand --------------------------

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
	// TODO could probably work on TreeZones too
	FreeZone &zone = dynamic_cast<FreeZone &>(*(*kit.register_file)[source_reg]);
	
    ASSERT( !zone.IsEmpty() );
    kit.scr_engine->MarkBaseForEmbedded( embedded_agent, zone.GetBaseNode() );   
    // Note: SCREngine will tell us to take a hike if we execute this more than once
}

    
string MarkBaseForEmbeddedCommand::GetTrace() const
{
	return "MarkBaseForEmbeddedCommand "+OpName(source_reg);
}

// ------------------------- CommandSequence --------------------------

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
