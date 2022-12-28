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
        
    while( !terms.empty() )
    {
        // Do terms backward to compensate for stack reversal        
        terms.back().SetXPtr( kit.free_zone_stack->top().GetBase() );
        terms.pop_back();
        kit.free_zone_stack->pop();
    }
    
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
    
    map<XLink, TreePtr<Node>> terminii;
    while( !terms.empty() )
    {
        // Do terms backward to compensate for stack reversal
        terminii[terms.back()] = kit.free_zone_stack->top().GetBase();
        terms.pop_back();
        kit.free_zone_stack->pop();
    }

    TreePtr<Node> new_base_x = Duplicate::DuplicateSubtree( kit.green_grass, 
                                                            zone.GetBase(), 
                                                            terminii );   
    
    for( auto p : terminii )
        ASSERT( !p.second ); // these are switched to NULL on reaching each terminus

    FreeZone new_free_zone( new_base_x );
    kit.free_zone_stack->push( new_free_zone );      
}

// ------------------------- DeleteCommand --------------------------

DeleteCommand::DeleteCommand( XLink target_base_xlink_ ) :
	target_base_xlink( target_base_xlink_ )
{
}


void DeleteCommand::Execute( const ExecKit &kit ) const
{
    // Update database 
    kit.x_tree_db->Delete( target_base_xlink );    

    // Patch the tree
    target_base_xlink.ClearXPtr();
}	

// ------------------------- InsertCommand --------------------------

InsertCommand::InsertCommand( XLink target_base_xlink_ ) :
	target_base_xlink( target_base_xlink_ )
{
}


void InsertCommand::Execute( const ExecKit &kit ) const
{
    ASSERT( !target_base_xlink.GetChildX() );
    
    // Patch the tree
    target_base_xlink.SetXPtr( kit.free_zone_stack->top().GetBase() );
    
    // Update database 
    kit.x_tree_db->Insert( target_base_xlink );      

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
    
// ------------------------- PushSubContainerCommand --------------------------

PushSubContainerCommand::PushSubContainerCommand( XLink base_ ) :
    base( base_ )
{
}


void PushSubContainerCommand::Execute( const ExecKit &kit ) const
{
    ASSERT( base );
    TreePtr<Node> base_node = base.GetChildX();
    
    // Key needs to implement ContainerInterface
    ContainerInterface *base_container = dynamic_cast<ContainerInterface *>(base_node.get());
    ASSERT( base_container )("Star node ")(*this)(" keyed to ")(*base_node)(" which should implement ContainerInterface");  
    
    // Make a subcontainer of the corresponding type
    TreePtr<SubContainer> dest;
    if( dynamic_cast<SequenceInterface *>(base_container) )
        dest = MakeTreeNode<SubSequence>();
    else if( dynamic_cast<CollectionInterface *>(base_container) )
        dest = MakeTreeNode<SubCollection>();
    else
        ASSERT(0)("Please add new kind of container");
    
    // Copy elements into dest subcontainer, duplicating all the subtrees
    TRACE("Walking container length %d\n", base_container->size() );
    ContainerInterface *dest_container = dynamic_cast<ContainerInterface *>(dest.get());
    for( const TreePtrInterface &key_elt : *base_container )
    {
        TRACE("Building ")(key_elt)("\n");
        TreePtr<Node> dest_elt = kit.free_zone_stack->top().GetBase();
        kit.free_zone_stack->pop();
        dest_container->insert_front( dest_elt ); // undo stack reversal
    }
    
    FreeZone dest_free_zone( dest );
    kit.free_zone_stack->push( dest_free_zone );      
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


