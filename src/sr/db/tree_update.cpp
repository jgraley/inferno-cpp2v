#include "tree_update.hpp"

#include "x_tree_database.hpp"
#include "common/read_args.hpp"

using namespace SR;

// ------------------------- PushFreeZoneCommand --------------------------

PushFreeZoneCommand::PushFreeZoneCommand( const FreeZone &zone_ ) :
	zone( zone_ )
{
}


void PushFreeZoneCommand::Execute( const ExecKit &kit ) const
{
    list<XLink> terms = zone.GetTerminii();
    ASSERT( kit.free_zone_stack->size() >= terms.size() ); // There must be enough items on the stack
    ASSERT( terms.size() == 0 ); // TODO 
	kit.free_zone_stack->push( zone );      
}

// ------------------------- PushTreeZoneCommand --------------------------

PushTreeZoneCommand::PushTreeZoneCommand( const TreeZone &zone_ ) :
	zone( zone_ )
{
}


void PushTreeZoneCommand::Execute( const ExecKit &kit ) const
{
    list<XLink> terms = zone.GetTerminii();
    ASSERT( kit.free_zone_stack->size() >= terms.size() ); // There must be enough items on the stack
    ASSERT( terms.size() <= 1 ); // DuplicateSubtree() can only do 0 or 1 terminus (TODO).
    
    TreePtr<Node> new_base_x;
    if( terms.size() == 1 ) // one terminus
    {
        new_base_x = Duplicate::DuplicateSubtree(kit.green_grass, 
                                                 zone.GetBase(), 
                                                 terms.front(), 
                                                 kit.free_zone_stack->top().GetBase());   
        kit.free_zone_stack->pop();
    }
    else // size 0 ie no terminii
    {
        new_base_x = Duplicate::DuplicateSubtree(kit.green_grass, zone.GetBase());   
    }
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

// ------------------------- MarkBaseForEmbeddedCommand --------------------------

MarkBaseForEmbeddedCommand::MarkBaseForEmbeddedCommand( RequiresSubordinateSCREngine *embedded_agent_ ) :
    embedded_agent( embedded_agent_ )
{
}
    
void MarkBaseForEmbeddedCommand::Execute( const ExecKit &kit ) const
{
    ASSERT( kit.free_zone_stack->top().GetBase() );
    kit.scr_engine->MarkBaseForEmbedded( embedded_agent, kit.free_zone_stack->top().GetBase() );   
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


