#include "tree_update.hpp"

#include "x_tree_database.hpp"
#include "common/read_args.hpp"

using namespace SR;

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


PushCommand::PushCommand( const FreeZone &new_zone_ ) :
	new_zone( new_zone_ )
{
}


void PushCommand::Execute( const ExecKit &kit ) const
{
	kit.free_zone_stack->push( new_zone );      
}


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


void CommandSequence::Add( shared_ptr<UpdateCommand> cmd )
{
	seq.push_back(cmd);
}


void CommandSequence::Execute( const ExecKit &kit ) const
{
	for( shared_ptr<UpdateCommand> cmd : seq )
		cmd->Execute(kit);
}
	
