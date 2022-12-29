#include "tree_update.hpp"

#include "x_tree_database.hpp"
#include "common/read_args.hpp"

using namespace SR;

// ------------------------- PopulateFreeZoneCommand --------------------------

PopulateFreeZoneCommand::PopulateFreeZoneCommand( const FreeZone &zone_ ) :
	zone( zone_ )
{
}


void PopulateFreeZoneCommand::Execute( const ExecKit &kit ) const
{
    const list<shared_ptr<Updater>> &terminii = zone.GetTerminii();
    ASSERT( kit.free_zone_stack->size() >= terminii.size() ); // There must be enough items on the stack
    
    // Get a forward list of subtrees to overwrite
    list<FreeZone> subtrees;
    for( auto terminus_upd : terminii )
    {
        subtrees.push_front( kit.free_zone_stack->top() );
        kit.free_zone_stack->pop();
    }
            
    for( auto terminus_upd : terminii )
    {
        TreePtr<Node> elt = subtrees.front().GetBase(); // see #703
        subtrees.pop_front();

        // Direct support for sub containers
        if( ContainerInterface *sub_con = dynamic_cast<ContainerInterface *>(elt.get()) )
        {
            for( const TreePtrInterface &sub_elt : *sub_con )
                terminus_upd->Insert( (TreePtr<Node>)sub_elt );                                     
        }
        else
        {
            terminus_upd->Insert( elt );
        }
    }
    
    kit.free_zone_stack->push( zone );      
}

// ------------------------- DuplicateTreeZoneCommand --------------------------

DuplicateTreeZoneCommand::DuplicateTreeZoneCommand( const TreeZone &zone_ ) :
	zone( zone_ )
{
}


void DuplicateTreeZoneCommand::Execute( const ExecKit &kit ) const
{
    // todo
}

// ------------------------- DuplicateAndPopulateTreeZoneCommand --------------------------

DuplicateAndPopulateTreeZoneCommand::DuplicateAndPopulateTreeZoneCommand( const TreeZone &zone_ ) :
	zone( zone_ )
{
}


void DuplicateAndPopulateTreeZoneCommand::Execute( const ExecKit &kit ) const
{
    list<XLink> terminii = zone.GetTerminii();
    ASSERT( kit.free_zone_stack->size() >= terminii.size() ); // There must be enough items on the stack
    
    map<XLink, TreePtr<Node>> duplicator_terminus_map;
    
    // Do terms backward to compensate for stack reversal
    for( auto terminus_it = terminii.rbegin(); terminus_it != terminii.rend(); ++terminus_it )
    {        
        duplicator_terminus_map[*terminus_it] = kit.free_zone_stack->top().GetBase();
        kit.free_zone_stack->pop();
    }

    TreePtr<Node> new_base_x = Duplicate::DuplicateSubtree( kit.green_grass, 
                                                            zone.GetBase(), 
                                                            duplicator_terminus_map );   
    
    for( auto p : duplicator_terminus_map )
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


