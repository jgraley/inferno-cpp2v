#include "tree_update.hpp"

#include "x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"

using namespace SR;

// ------------------------- DeclareFreeZoneCommand --------------------------

DeclareFreeZoneCommand::DeclareFreeZoneCommand( const FreeZone &zone_ ) :
	zone(zone_)
{
}	


void DeclareFreeZoneCommand::Execute( const ExecKit &kit ) const
{
	kit.free_zone_stack->push(zone);
}

// ------------------------- DuplicateTreeZoneCommand --------------------------

DuplicateTreeZoneCommand::DuplicateTreeZoneCommand( const TreeZone &zone_ ) :
	zone( zone_ )
{
}


void DuplicateTreeZoneCommand::Execute( const ExecKit &kit ) const
{
    if( zone.IsEmpty() )
    {
		// Duplicate::DuplicateSubtree() can't work with the
		// terminus-at-base you get with an empty zone, so handle that
		// case explicitly.
        kit.free_zone_stack->push( FreeZone::CreateEmpty() ); 
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
    
    list<shared_ptr<Updater>> free_zone_terminii;
    for( XLink terminus_upd : zone.GetTerminii() )
    {
		ASSERT( duplicator_terminus_map[terminus_upd].updater );
        free_zone_terminii.push_back( duplicator_terminus_map[terminus_upd].updater );
	}

    // Create a new zone for the result.
    auto result_zone = FreeZone( new_base_x, free_zone_terminii );
    kit.free_zone_stack->push( result_zone );      
}

// ------------------------- PopulateFreeZoneCommand --------------------------

PopulateFreeZoneCommand::PopulateFreeZoneCommand()
{
}


void PopulateFreeZoneCommand::Execute( const ExecKit &kit ) const
{
	FreeZone zone = kit.free_zone_stack->top();
	kit.free_zone_stack->pop();
	
    const list<shared_ptr<Updater>> &terminii = zone.GetTerminii();
    ASSERT( kit.free_zone_stack->size() >= terminii.size() ); // There must be enough items on the stack
    
    if( zone.IsEmpty() )
    {
        // We're empty, so we should have one terminus
        ASSERT( terminii.size() == 1 );
        // Exactly one zone to attach should be on the stack, and that's also
        // going to be our output, since populating an empty zone just means
        // substituting. So there's nothing to do.
        return;
    }

    // Get a correctly-ordered list of subtrees to overwrite
    list<FreeZone> operand_zones;
    for( auto terminus_upd : terminii )
    {
        operand_zones.push_front( kit.free_zone_stack->top() );
        kit.free_zone_stack->pop();
    }
                        
    // Iterate over terminii and operand zones together, populating the terminii
    // from the operands. 
    for( auto p : Zip( terminii, operand_zones ) )
    {
        shared_ptr<Updater> terminus_upd = p.first;
        FreeZone &operand_zone = p.second; 
        ASSERT( operand_zone.GetTerminii().empty() )(zone)(" ")(Zip( terminii, operand_zones )); // TODO accumulate the terminii in the result zone.
        ASSERT( !operand_zone.IsEmpty() );
        // Populate terminus. Apply() will expand SubContainers
        ASSERT( operand_zone.GetBase() );
        terminus_upd->Apply( operand_zone.GetBase() );
    }
    
    //Validate()( zone->GetBase() );
    
    // Create a new zone for the result, so we don't leave our member zone's terminii in.
    auto result_zone = FreeZone::CreateSubtree( zone.GetBase() );    
    kit.free_zone_stack->push( result_zone );      
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
    FreeZone zone = kit.free_zone_stack->top();
    ASSERT( !zone.IsEmpty() );
    //Validate()( zone.GetBase() );
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
	FreeZone zone = kit.free_zone_stack->top();
	
    ASSERT( !zone.IsEmpty() );
    kit.scr_engine->MarkBaseForEmbedded( embedded_agent, zone.GetBase() );   
    // Note: SCREngine will tell us to take a hike if we execute this more than once
}
    
// ------------------------- CommandSequence --------------------------

void CommandSequence::Execute( const ExecKit &kit ) const
{
	for( const unique_ptr<Command> &cmd : seq )
		cmd->Execute(kit);
}
	
    
void CommandSequence::Add( unique_ptr<Command> new_cmd )
{
    if( auto new_seq = dynamic_pointer_cast<CommandSequence>(new_cmd) )
    {
        seq.insert( seq.end(),
                    make_move_iterator(new_seq->seq.begin()),
                    make_move_iterator(new_seq->seq.end()) );    
    }
    else
    {
	    seq.push_back(move(new_cmd));
    }
}


bool CommandSequence::IsEmpty() const
{
	return seq.empty();
}


