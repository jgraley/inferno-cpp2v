#include "tree_update.hpp"

#include "x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"

using namespace SR;

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


void DeclareFreeZoneCommand::Execute( const ExecKit &kit ) const
{
	kit.free_zone_stack->push(*zone);
}


string DeclareFreeZoneCommand::GetTrace() const
{
	return "DeclareFreeZoneCommand      "+Trace(*zone)+" -> PUSH";
}

// ------------------------- DuplicateTreeZoneCommand --------------------------

void DuplicateTreeZoneCommand::Execute( const ExecKit &kit ) const
{
	if( kit.x_tree_db )
		zone.DBCheck(kit.x_tree_db);
	
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
    
    vector<shared_ptr<Updater>> free_zone_terminii;
    for( XLink terminus_upd : zone.GetTerminii() )
    {
		ASSERT( duplicator_terminus_map[terminus_upd].updater );
        free_zone_terminii.push_back( duplicator_terminus_map[terminus_upd].updater );
	}

    // Create a new zone for the result.
    auto result_zone = FreeZone( new_base_x, free_zone_terminii );
    kit.free_zone_stack->push( result_zone );      
}


string DuplicateTreeZoneCommand::GetTrace() const
{
	return "DuplicateTreeZoneCommand    "+Trace(zone)+" -> PUSH";
}

// ------------------------- PopulateFreeZoneCommand --------------------------

PopulateFreeZoneCommand::PopulateFreeZoneCommand()
{
}


void PopulateFreeZoneCommand::Execute( const ExecKit &kit ) const
{
	FreeZone dest_zone = kit.free_zone_stack->top();
	kit.free_zone_stack->pop();
	
    vector<shared_ptr<Updater>> terminii = dest_zone.GetTerminii();
    ASSERT( kit.free_zone_stack->size() >= terminii.size() ); // There must be enough items on the stack
    
    if( dest_zone.IsEmpty() )
    {
        // We're empty, so we should have one terminus
        ASSERT( terminii.size() == 1 );
        // Exactly one zone to attach should be on the stack, and that's also
        // going to be our output, since populating an empty zone just means
        // substituting. So there's nothing to do.
        return;
    }

    // Get a correctly-ordered list of subtrees to overwrite
    list<FreeZone> source_zones;
    for( auto terminus_upd : terminii )
    {
        source_zones.push_front( kit.free_zone_stack->top() );
        kit.free_zone_stack->pop();
    }
                        
    // Iterate over terminii and operand zones together, populating the terminii
    // from the operands. 
    for( auto p : Zip( terminii, source_zones ) )
    {
        shared_ptr<Updater> terminus_upd = p.first;
        FreeZone &source_zone = p.second; 
        ASSERT( source_zone.GetTerminii().empty() )(dest_zone)(" ")(Zip( terminii, source_zones )); // TODO accumulate the terminii in the result zone.
        ASSERT( !source_zone.IsEmpty() );
        // Populate terminus. Apply() will expand SubContainers
        ASSERT( source_zone.GetBase() );
        terminus_upd->Apply( source_zone.GetBase() );
    }
    
    //Validate()( zone->GetBase() );
    
    // Create a new zone for the result, so we don't leave our member zone's terminii in.
    auto result_zone = FreeZone::CreateSubtree( dest_zone.GetBase() );    
    kit.free_zone_stack->push( result_zone );      
}


string PopulateFreeZoneCommand::GetTrace() const
{
	return "PopulateFreeZoneCommand     POP, POP* -> PUSH";
}

// ------------------------- JoinFreeZoneCommand --------------------------

JoinFreeZoneCommand::JoinFreeZoneCommand(int ti) :
    terminus_index(ti)
{
}


void JoinFreeZoneCommand::Execute( const ExecKit &kit ) const
{
	FreeZone source_zone = kit.free_zone_stack->top();
	kit.free_zone_stack->pop();
	FreeZone dest_zone = kit.free_zone_stack->top();
	
    vector<shared_ptr<Updater>> terminii = dest_zone.GetTerminii();
    
    if( dest_zone.IsEmpty() )
    {
        // We're empty, so we should have one terminus
        ASSERT( terminus_index==0 );
        kit.free_zone_stack->pop(); // pop the dest zone
		kit.free_zone_stack->push( source_zone ); // push source zone over it
		// TODO not really SSA: we're not just modifying the zone, 
		// but replacing it with another one. Could elide from the 
		// command sequence.
		return;   
    }

    shared_ptr<Updater> terminus_upd = dest_zone.GetTerminus(terminus_index);
    
    ASSERT( source_zone.GetTerminii().empty() )(dest_zone)(" ")(source_zone);
    ASSERT( !source_zone.IsEmpty() );
    // Populate terminus. Apply() will expand SubContainers
    ASSERT( source_zone.GetBase() );
    terminus_upd->Apply( source_zone.GetBase() );
    
    //Validate()( zone->GetBase() );
    
    // Create a new zone for the result, so we don't leave our member zone's terminii in.
    auto result_zone = FreeZone::CreateSubtree( dest_zone.GetBase() );    
    kit.free_zone_stack->push( result_zone );      
}


string JoinFreeZoneCommand::GetTrace() const
{
	return SSPrintf("JoinFreeZoneCommand       PEEK[%d], POP", terminus_index);
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


string DeleteCommand::GetTrace() const
{
	return "DeleteCommand               "+Trace(target_base_xlink);
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


string InsertCommand::GetTrace() const
{
	return "InsertCommand               POP, "+Trace(target_base_xlink);
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

    
string MarkBaseForEmbeddedCommand::GetTrace() const
{
	return "MarkBaseForEmbeddedCommand  PEEK";
}

// ------------------------- CommandSequence --------------------------

void CommandSequence::Execute( const ExecKit &kit ) const
{
	//FTRACE(" executing");
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


const list<unique_ptr<Command>> &CommandSequence::GetSeq() const
{
	return seq;
}


string CommandSequence::GetTrace() const
{
    list<string> elts;
    for( const unique_ptr<Command> &pc : seq )
        elts.push_back( Trace(*pc) );
    return Join( elts, "\n", "CommandSequence[\n", " ]\n" );
}

// ------------------------- Runners --------------------------

FreeZone SR::RunGetFreeZoneNoDB( unique_ptr<Command> cmd, const SCREngine *scr_engine )
{
    stack<FreeZone> free_zone_stack;
    Command::ExecKit exec_kit {nullptr, scr_engine, scr_engine, &free_zone_stack};
	cmd->Execute( exec_kit );   
	ASSERT( free_zone_stack.size() == 1);       
    return free_zone_stack.top();  	
}


void SR::RunVoidForReplace( unique_ptr<Command> cmd, const SCREngine *scr_engine, XTreeDatabase *x_tree_db )
{
	// Ensure we have a CommandSequence
	auto seq = make_unique<CommandSequence>();
	seq->Add( move(cmd) ); // flattens as it goes...
	
	// Uniqueness of tree zones
	TreeZoneOverlapFinder finder( x_tree_db, seq.get() );
	
	// calculate SASU indexes
	// err...
	
    stack<FreeZone> free_zone_stack;
    Command::ExecKit exec_kit {x_tree_db, x_tree_db, scr_engine, &free_zone_stack};
	seq->Execute( exec_kit );   
	ASSERT( free_zone_stack.size() == 0);       
}

// ------------------------- TreeZoneOverlapFinder --------------------------

TreeZoneOverlapFinder::TreeZoneOverlapFinder( const XTreeDatabase *db, CommandSequence *seq )
{
	// Put them all into one Overlapping set, pessamistically assuming they
	// all overlap
	
	for( const unique_ptr<Command> &cmd : seq->GetSeq() )
	{
		if( auto tz_cmd = dynamic_cast<const ImmediateTreeZoneCommand *>(cmd.get()) )
        {
            // Note that key is actually TreeZone *, so equal TreeZones get different 
            // rows which is why we InsertSolo()
            const TreeZone *zone = tz_cmd->GetTreeZone();
            
            // Zone should be known to the DB
            zone->DBCheck(db);
            
            // Record zone and related command
            InsertSolo( tzps_to_commands, make_pair( zone, tz_cmd ) );
            
            // Start off with an empty overlapping set
            InsertSolo( overlapping_zones, make_pair( zone, set<const TreeZone *>() ) );
        }
	}
	
    ForAllCommutativeDistinctPairs( tzps_to_commands, 
                                    [&](const pair<const TreeZone *, const ImmediateTreeZoneCommand *> &l, 
                                        const pair<const TreeZone *, const ImmediateTreeZoneCommand *> &r)
    {
        if( TreeZone::IsOverlap( db, *l.first, *r.first ) )
        {
            // It's a symmentrical relationship so do it both ways around
            overlapping_zones[l.first].insert(r.first);
            overlapping_zones[r.first].insert(l.first);
        }
    } );
    
    //FTRACE(overlapping_zones);
}
