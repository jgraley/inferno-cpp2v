#include "tree_update.hpp"

#include "x_tree_database.hpp"
#include "common/read_args.hpp"

using namespace SR;

// ------------------------- PushFreeZoneCommand --------------------------

PushFreeZoneCommand::PushFreeZoneCommand( const FreeZone &new_zone_ ) :
	new_zone( new_zone_ )
{
}


void PushFreeZoneCommand::Execute( const ExecKit &kit ) const
{
	kit.free_zone_stack->push( new_zone );      
}

// ------------------------- PushTreeZoneCommand --------------------------

PushTreeZoneCommand::PushTreeZoneCommand( const TreeZone &new_zone_ ) :
	new_zone( new_zone_ )
{
}


void PushTreeZoneCommand::Execute( const ExecKit &kit ) const
{
    TreePtr<Node> new_base_x = Duplicate::DuplicateSubtree(kit.green_grass, new_zone.GetBase());   
    FreeZone new_free_zone( new_base_x );
    kit.free_zone_stack->push( new_free_zone );      
}

// ------------------------- DeleteCommand --------------------------

DeleteCommand::DeleteCommand( const TreeZone &target_ ) :
	target( target_ )
{
}


void DeleteCommand::Execute( const ExecKit &kit ) const
{
    // Update database 
    kit.x_tree_db->Delete( target );    

    // Patch the tree
    target.GetBase().ClearXPtr();
}	

// ------------------------- InsertCommand --------------------------

InsertCommand::InsertCommand( const TreeZone &target_ ) :
	target( target_ )
{
}


void InsertCommand::Execute( const ExecKit &kit ) const
{
	// Get base of target zone
    XLink target_base_xlink = target.GetBase();    
    ASSERT( !target_base_xlink.GetChildX() );
    
    // Patch the tree
    target_base_xlink.SetXPtr( kit.free_zone_stack->top().GetBase() );
    
    // Update database 
    TreeZone new_tree_zone( target_base_xlink, kit.free_zone_stack->top() );
    kit.x_tree_db->Insert( new_tree_zone );      

    kit.free_zone_stack->pop();
}

// ------------------------- CommandSequence --------------------------


void CommandSequence::Execute( const ExecKit &kit ) const
{
	for( const unique_ptr<Command> &cmd : seq )
		cmd->Execute(kit);
}
	
    
void CommandSequence::Add( unique_ptr<Command> cmd )
{
	seq.push_back(move(cmd));
}


bool CommandSequence::IsEmpty() const
{
	return seq.empty();
}


