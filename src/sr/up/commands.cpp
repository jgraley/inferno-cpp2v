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


void DeclareFreeZoneCommand::SetOperandRegs( SSAAllocator &allocator )
{
	dest_reg = allocator.Push();
}


void DeclareFreeZoneCommand::Execute( const ExecKit &kit ) const
{
	(*kit.register_file)[dest_reg] = make_unique<FreeZone>(*zone);
}


string DeclareFreeZoneCommand::GetTrace() const
{
	return "DeclareFreeZoneCommand      "+Trace(*zone)+" -> "+OpName(dest_reg);
}

// ------------------------- DeclareTreeZoneCommand --------------------------

void DeclareTreeZoneCommand::SetOperandRegs( SSAAllocator &allocator )
{
	dest_reg = allocator.Push();
}


void DeclareTreeZoneCommand::Execute( const ExecKit &kit ) const
{
	(*kit.register_file)[dest_reg] = make_unique<TreeZone>(zone);
}


string DeclareTreeZoneCommand::GetTrace() const
{
	return "DeclareTreeZoneCommand      "+Trace(zone)+" -> "+OpName(dest_reg);
}

// ------------------------- DuplicateZoneCommand --------------------------

void DuplicateZoneCommand::SetOperandRegs( SSAAllocator &allocator )
{
	dest_reg = allocator.Peek();
}


void DuplicateZoneCommand::Execute( const ExecKit &kit ) const
{
	TreeZone &zone = dynamic_cast<TreeZone &>(*(*kit.register_file)[dest_reg]);
	
	if( kit.x_tree_db )
		zone.DBCheck(kit.x_tree_db);
	
    if( zone.IsEmpty() )
    {
		// Duplicate::DuplicateSubtree() can't work with the
		// terminus-at-base you get with an empty zone, so handle that
		// case explicitly.
        (*kit.register_file)[dest_reg] = make_unique<FreeZone>(FreeZone::CreateEmpty()); 
        return;
    }

    // Iterate over terminii and operand zones together, filling the map for
    // DuplicateSubtree() to use.
    Duplicate::TerminiiMap duplicator_terminus_map;
    for( XLink terminus_upd : zone.GetTerminusXLinks() ) 
        duplicator_terminus_map[terminus_upd] = { TreePtr<Node>(), shared_ptr<Updater>() };

    // Duplicate the subtree, populating from the map.
    TreePtr<Node> new_base_x = Duplicate::DuplicateSubtree( kit.green_grass, 
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
	return "DuplicateZoneCommand        "+OpName(dest_reg);
}

// ------------------------- JoinZoneCommand --------------------------

JoinZoneCommand::JoinZoneCommand(int ti) :
    terminus_index(ti)
{
}


void JoinZoneCommand::SetOperandRegs( SSAAllocator &allocator )
{
	source_reg = allocator.Pop();
	dest_reg = allocator.Peek();
}


void JoinZoneCommand::Execute( const ExecKit &kit ) const
{
	// Only free zones can be joined
	FreeZone source_zone = dynamic_cast<FreeZone &>(*(*kit.register_file)[source_reg]);
	FreeZone &dest_zone = dynamic_cast<FreeZone &>(*(*kit.register_file)[dest_reg]);
	    
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

    shared_ptr<Updater> terminus_upd = dest_zone.GetTerminusUpdater(terminus_index);
    dest_zone.DropTerminus(terminus_index);
    
    // Populate terminus. Apply() will expand SubContainers
    ASSERT( source_zone.GetBaseNode() );
    terminus_upd->Apply( source_zone.GetBaseNode() );
    
    //Validate()( zone->GetBaseNode() );     
}


string JoinZoneCommand::GetTrace() const
{
	return "JoinZoneCommand         " +
	       OpName(dest_reg) +
	       SSPrintf("[%d], ", terminus_index) +
	       OpName(source_reg);
}

// ------------------------- ModifyTreeCommand --------------------------

void ModifyTreeCommand::SetOperandRegs( SSAAllocator &allocator )
{
	target_reg = allocator.Pop();
	source_reg = allocator.Pop();
}


void ModifyTreeCommand::Execute( const ExecKit &kit ) const
{
    // New zone must be a free zone
    auto target_zone = dynamic_cast<TreeZone &>(*(*kit.register_file)[target_reg]);
    auto source_free_zone = dynamic_cast<FreeZone &>(*(*kit.register_file)[source_reg]);

	ASSERT( target_zone.GetNumTerminii() == 0 ); // TODO under #723
    XLink target_base_xlink = target_zone.GetBaseXLink();
    
    // Update database 
    kit.x_tree_db->Delete( target_base_xlink );    

    // Patch the tree
    target_base_xlink.ClearXPtr();

    ASSERT( !target_base_xlink.GetChildX() );
    
    // Patch the tree
    target_base_xlink.SetXPtr( source_free_zone.GetBaseNode() );
    
    // Update database 
    kit.x_tree_db->Insert( target_base_xlink );      
}


string ModifyTreeCommand::GetTrace() const
{
	return "ModifyTreeCommand               "+OpName(source_reg)+", "+OpName(target_reg);
}

// ------------------------- MarkBaseForEmbeddedCommand --------------------------

MarkBaseForEmbeddedCommand::MarkBaseForEmbeddedCommand( RequiresSubordinateSCREngine *embedded_agent_ ) :
    embedded_agent( embedded_agent_ )
{
}
    
    
void MarkBaseForEmbeddedCommand::SetOperandRegs( SSAAllocator &allocator )
{
	dest_reg = allocator.Peek();
}


void MarkBaseForEmbeddedCommand::Execute( const ExecKit &kit ) const
{
	// TODO could probably work on TreeZones too
	FreeZone &zone = dynamic_cast<FreeZone &>(*(*kit.register_file)[dest_reg]);
	
    ASSERT( !zone.IsEmpty() );
    kit.scr_engine->MarkBaseForEmbedded( embedded_agent, zone.GetBaseNode() );   
    // Note: SCREngine will tell us to take a hike if we execute this more than once
}

    
string MarkBaseForEmbeddedCommand::GetTrace() const
{
	return "MarkBaseForEmbeddedCommand  "+OpName(dest_reg);
}

// ------------------------- CommandSequence --------------------------

void CommandSequence::SetOperandRegs( SSAAllocator &allocator )
{
	for( const unique_ptr<Command> &cmd : seq )
		cmd->SetOperandRegs( allocator );
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
