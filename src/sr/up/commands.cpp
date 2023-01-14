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
		return SSPrintf("R%d", reg);
}

// ------------------------- ImmediateTreeZoneCommand --------------------------

ImmediateTreeZoneCommand::ImmediateTreeZoneCommand( const TreeZone &zone_ ) :
	zone( zone_ )
{
}


const TreeZone *ImmediateTreeZoneCommand::GetTreeZone() const
{
    return &zone;
}

// ------------------------- DeclareFreeZoneCommand --------------------------

DeclareFreeZoneCommand::DeclareFreeZoneCommand( FreeZone &&zone_ ) :
	zone(make_unique<FreeZone>(move(zone_)))
{
}	


void DeclareFreeZoneCommand::SetOperands( int &pseudo_stack_top )
{
	pseudo_stack_top++;
	dest_reg = pseudo_stack_top;
}


void DeclareFreeZoneCommand::Execute( const ExecKit &kit ) const
{
	(*kit.free_zone_regs)[dest_reg] = *zone;
}


string DeclareFreeZoneCommand::GetTrace() const
{
	return "DeclareFreeZoneCommand      "+Trace(*zone)+" -> "+OpName(dest_reg);
}

// ------------------------- DuplicateTreeZoneCommand --------------------------

void DuplicateTreeZoneCommand::SetOperands( int &pseudo_stack_top )
{
	pseudo_stack_top++;
	dest_reg = pseudo_stack_top;
}


void DuplicateTreeZoneCommand::Execute( const ExecKit &kit ) const
{
	if( kit.x_tree_db )
		zone.DBCheck(kit.x_tree_db);
	
    if( zone.IsEmpty() )
    {
		// Duplicate::DuplicateSubtree() can't work with the
		// terminus-at-base you get with an empty zone, so handle that
		// case explicitly.
        (*kit.free_zone_regs)[dest_reg] = FreeZone::CreateEmpty(); 
        return;
    }

    // Iterate over terminii and operand zones together, filling the map for
    // DuplicateSubtree() to use.
    Duplicate::TerminiiMap duplicator_terminus_map;
    for( XLink terminus_upd : zone.GetTerminii() ) 
        duplicator_terminus_map[terminus_upd] = { TreePtr<Node>(), shared_ptr<Updater>() };

    // Duplicate the subtree, populating from the map.
    TreePtr<Node> new_base_x = Duplicate::DuplicateSubtree( kit.green_grass, 
                                                            zone.GetBase(), 
                                                            duplicator_terminus_map );   
    
    vector<shared_ptr<Updater>> free_zone_terminii;
    for( XLink terminus_upd : zone.GetTerminii() )
    {
		ASSERT( duplicator_terminus_map[terminus_upd].updater );
        free_zone_terminii.push_back( duplicator_terminus_map[terminus_upd].updater );
	}

    // Create a new zone for the result.
    auto result_zone = FreeZone( new_base_x, free_zone_terminii );
    (*kit.free_zone_regs)[dest_reg] = result_zone;      
}


string DuplicateTreeZoneCommand::GetTrace() const
{
	return "DuplicateTreeZoneCommand    "+Trace(zone)+" -> "+OpName(dest_reg);
}

// ------------------------- JoinFreeZoneCommand --------------------------

JoinFreeZoneCommand::JoinFreeZoneCommand(int ti) :
    terminus_index(ti)
{
}


void JoinFreeZoneCommand::SetOperands( int &pseudo_stack_top )
{
	source_reg = pseudo_stack_top;
	pseudo_stack_top--;
	dest_reg = pseudo_stack_top;
}


void JoinFreeZoneCommand::Execute( const ExecKit &kit ) const
{
	FreeZone source_zone = (*kit.free_zone_regs)[source_reg];
	FreeZone &dest_zone = (*kit.free_zone_regs)[dest_reg];
	    
    if( dest_zone.IsEmpty() )
    {
        // We're empty, so we should have one terminus
        ASSERT( terminus_index==0 );
		dest_zone = source_zone; // push source zone over it
		// TODO not really SSA: we're not just modifying the zone, 
		// but replacing it with another one. Could elide from the 
		// command sequence.
		return;   
    }

    ASSERT( !source_zone.IsEmpty() );

    shared_ptr<Updater> terminus_upd = dest_zone.GetTerminus(terminus_index);
    dest_zone.DropTerminus(terminus_index);
    
    // Populate terminus. Apply() will expand SubContainers
    ASSERT( source_zone.GetBase() );
    terminus_upd->Apply( source_zone.GetBase() );
    
    //Validate()( zone->GetBase() );     
}


string JoinFreeZoneCommand::GetTrace() const
{
	return "JoinFreeZoneCommand         " +
	       OpName(dest_reg) +
	       SSPrintf("[%d], ", terminus_index) +
	       OpName(source_reg);
}

// ------------------------- DeleteCommand --------------------------

DeleteCommand::DeleteCommand( XLink target_base_xlink_ ) :
	target_base_xlink( target_base_xlink_ )
{
}


void DeleteCommand::SetOperands( int &pseudo_stack_top )
{
}


void DeleteCommand::Execute( const ExecKit &kit ) const
{
    // Update database 
    kit.x_tree_db->Delete( target_base_xlink );    

    // Patch the tree
    target_base_xlink.ClearXPtr();
}	


string DeleteCommand::GetTrace() const
{
	return "DeleteCommand               "+Trace(target_base_xlink);
}

// ------------------------- InsertCommand --------------------------

InsertCommand::InsertCommand( XLink target_base_xlink_ ) :
	target_base_xlink( target_base_xlink_ )
{
}


void InsertCommand::SetOperands( int &pseudo_stack_top )
{
	source_reg = pseudo_stack_top;
	pseudo_stack_top--;
}


void InsertCommand::Execute( const ExecKit &kit ) const
{
    ASSERT( !target_base_xlink.GetChildX() );
    
    // Patch the tree
    target_base_xlink.SetXPtr( (*kit.free_zone_regs)[source_reg].GetBase() );
    
    // Update database 
    kit.x_tree_db->Insert( target_base_xlink );      
}


string InsertCommand::GetTrace() const
{
	return "InsertCommand               POP, "+Trace(target_base_xlink);
}

// ------------------------- MarkBaseForEmbeddedCommand --------------------------

MarkBaseForEmbeddedCommand::MarkBaseForEmbeddedCommand( RequiresSubordinateSCREngine *embedded_agent_ ) :
    embedded_agent( embedded_agent_ )
{
}
    
    
void MarkBaseForEmbeddedCommand::SetOperands( int &pseudo_stack_top )
{
	dest_reg = pseudo_stack_top;
}


void MarkBaseForEmbeddedCommand::Execute( const ExecKit &kit ) const
{
	FreeZone &zone = (*kit.free_zone_regs)[dest_reg];
	
    ASSERT( !zone.IsEmpty() );
    kit.scr_engine->MarkBaseForEmbedded( embedded_agent, zone.GetBase() );   
    // Note: SCREngine will tell us to take a hike if we execute this more than once
}

    
string MarkBaseForEmbeddedCommand::GetTrace() const
{
	return "MarkBaseForEmbeddedCommand  "+OpName(dest_reg);
}

// ------------------------- CommandSequence --------------------------

void CommandSequence::SetOperands( int &pseudo_stack_top )
{
	for( const unique_ptr<Command> &cmd : seq )
		cmd->SetOperands( pseudo_stack_top );
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
