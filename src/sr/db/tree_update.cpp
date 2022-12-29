#include "tree_update.hpp"

#include "x_tree_database.hpp"
#include "common/read_args.hpp"

using namespace SR;

// ------------------------- PopulateFreeZoneCommand --------------------------

// Populate a free zone from the stack. Push the resulting subtree to the stack.
PopulateFreeZoneCommand::PopulateFreeZoneCommand( const FreeZone &zone_ ) :
	zone( zone_ )
{
}


void PopulateFreeZoneCommand::Execute( const ExecKit &kit ) const
{
    const list<shared_ptr<Overwriter>> &terminii = zone.GetTerminii();
    ASSERT( kit.free_zone_stack->size() >= terminii.size() ); // There must be enough items on the stack
        
    for( auto terminus_it = terminii.rbegin(); terminus_it != terminii.rend(); ++terminus_it )
    {
        // Do terms backward to compensate for stack reversal     
        Overwriter &terminus_overwritable = **terminus_it;
        TreePtr<Node> new_subtree = kit.free_zone_stack->top().GetBase(); // see #703
        terminus_overwritable.Overwrite( &new_subtree );
        kit.free_zone_stack->pop();
    }
    
    kit.free_zone_stack->push( zone );      
}

// ------------------------- DuplicateAndPopulateTreeZoneCommand --------------------------

// Duplicate a tree zone, making a free zone, and populate it from the stack.
// Push the resulting subtree to the stack.
DuplicateAndPopulateTreeZoneCommand::DuplicateAndPopulateTreeZoneCommand( const TreeZone &zone_ ) :
	zone( zone_ )
{
}


void DuplicateAndPopulateTreeZoneCommand::Execute( const ExecKit &kit ) const
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

// Takes the base of the zone at the top of the stack and remembers it as
// the base to use for the configured embedded engine. No change to stack.
MarkBaseForEmbeddedCommand::MarkBaseForEmbeddedCommand( RequiresSubordinateSCREngine *embedded_agent_ ) :
    embedded_agent( embedded_agent_ )
{
}
    
void MarkBaseForEmbeddedCommand::Execute( const ExecKit &kit ) const
{
    ASSERT( kit.free_zone_stack->top().GetBase() );
    kit.scr_engine->MarkBaseForEmbedded( embedded_agent, kit.free_zone_stack->top().GetBase() );   
}
    
// ------------------------- CreateAndPopulateSubContainerCommand --------------------------

// Create a free subcontainer given a tree subcontainer, and populate it
// from the stack. Push the resulting subtree to the stack.
// TODO generalise to a PopulateFreeZoneCommand?
CreateAndPopulateSubContainerCommand::CreateAndPopulateSubContainerCommand( XLink base_ ) :
    base( base_ )
{
}


void CreateAndPopulateSubContainerCommand::Execute( const ExecKit &kit ) const
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


