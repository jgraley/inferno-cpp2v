#include "commands.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tree_update.hpp"

using namespace SR;

// ------------------------- Command --------------------------

void Command::ForWalk( function<void(const Command *cmd)> func_in,
	                   function<void(const Command *cmd)> func_out ) const try
{
	WalkImpl(func_in, func_out);
}
catch( BreakException )
{
}		       

// ------------------------- PopulateZoneCommand --------------------------

bool PopulateZoneCommand::IsExpression() const
{
	return true;
}


PopulateZoneCommand::PopulateZoneCommand( vector<unique_ptr<Command>> &&child_expressions_ ) :
	child_expressions(move(child_expressions_))
{
}	


PopulateZoneCommand::PopulateZoneCommand()
{
}	


void PopulateZoneCommand::AddEmbeddedAgentBase( RequiresSubordinateSCREngine *embedded_agent )
{
	embedded_agents.push_back( embedded_agent );
}


int PopulateZoneCommand::GetNumChildExpressions() const
{
	return child_expressions.size();
}


void PopulateZoneCommand::WalkImpl(function<void(const Command *cmd)> func_in,
			                       function<void(const Command *cmd)> func_out) const
{
	if( func_in )
		func_in(this);
	for( const unique_ptr<Command> &child_expression : child_expressions )
		child_expression->WalkImpl(func_in, func_out);
	if( func_out )
		func_out(this);
}


void PopulateZoneCommand::PopulateFreeZone( FreeZone &free_zone, const ExecKit &kit ) const	
{
	//FTRACE(free_zone)("\n");
	vector<FreeZone> child_zones;
	for( const unique_ptr<Command> &child_expression : child_expressions )
	{
		//FTRACE(child_expression)("\n");
		unique_ptr<Zone> zone = child_expression->Evaluate( kit );
		if( auto free_zone = dynamic_pointer_cast<FreeZone>(zone) )
			child_zones.push_back( *free_zone );
		else
			ASSERTFAIL(); // not a free zone	
	}
	
	free_zone.Populate(kit.x_tree_db, child_zones);
	
	for( RequiresSubordinateSCREngine *ea : embedded_agents )
		free_zone.MarkBaseForEmbedded(kit.scr_engine, ea);		
}
	
	
void PopulateZoneCommand::Execute( const ExecKit &kit ) const
{
	ASSERTFAIL();
}


string PopulateZoneCommand::GetTrace() const
{
	return "PopulateZoneCommand TODOOOOOOOOOO "+Trace(child_expressions);
}

// ------------------------- PopulateTreeZoneCommand --------------------------

PopulateTreeZoneCommand::PopulateTreeZoneCommand( unique_ptr<TreeZone> &&zone_, vector<unique_ptr<Command>> &&child_expressions ) :
	PopulateZoneCommand( move(child_expressions) ),
	zone(move(zone_))
{
	ASSERT( zone->GetNumTerminii() == GetNumChildExpressions() );	
}	
		

PopulateTreeZoneCommand::PopulateTreeZoneCommand( unique_ptr<TreeZone> &&zone_ ) :
	zone(move(zone_))
{
	ASSERT( zone->GetNumTerminii() == 0 );	
}


const TreeZone *PopulateTreeZoneCommand::GetZone() const
{
    return zone.get();
}


unique_ptr<Zone> PopulateTreeZoneCommand::Evaluate( const ExecKit &kit ) const
{
	FreeZone free_zone = zone->Duplicate( kit.x_tree_db );
	PopulateFreeZone( free_zone, kit );
	return make_unique<FreeZone>(free_zone);
}


// ------------------------- PopulateFreeZoneCommand --------------------------

PopulateFreeZoneCommand::PopulateFreeZoneCommand( unique_ptr<FreeZone> &&zone_, vector<unique_ptr<Command>> &&child_expressions ) :
	PopulateZoneCommand( move(child_expressions) ),
	zone(move(zone_))
{
	ASSERT( zone->GetNumTerminii() == GetNumChildExpressions() );	
}

		
PopulateFreeZoneCommand::PopulateFreeZoneCommand( unique_ptr<FreeZone> &&zone_ ) :
   	PopulateZoneCommand(),
   	zone(move(zone_))
{
	ASSERT( zone->GetNumTerminii() == 0 );	
}


const FreeZone *PopulateFreeZoneCommand::GetZone() const
{
    return zone.get();
}


unique_ptr<Zone> PopulateFreeZoneCommand::Evaluate( const ExecKit &kit ) const
{
	PopulateFreeZone( *zone, kit );
	return make_unique<FreeZone>(*zone);
}

// ------------------------- UpdateTreeCommand --------------------------

UpdateTreeCommand::UpdateTreeCommand( const TreeZone &target_tree_zone_, unique_ptr<Command> child_expression_ ) :
	target_tree_zone( target_tree_zone_ ),
	child_expression( move(child_expression_) )
{
}


bool UpdateTreeCommand::IsExpression() const
{
	return false;
}

const Command *UpdateTreeCommand::GetExpression() const
{
	return child_expression.get();
}



void UpdateTreeCommand::Execute( const ExecKit &kit ) const
{
    // New zone must be a free zone
	unique_ptr<Zone> zone = child_expression->Evaluate( kit );
	auto source_free_zone = dynamic_pointer_cast<FreeZone>(zone);
	ASSERT( source_free_zone );
    target_tree_zone.Update( kit.x_tree_db, *source_free_zone );
}


string UpdateTreeCommand::GetTrace() const
{
	return "UpdateTreeCommand "+Trace(child_expression)+" over "+Trace(target_tree_zone);
}

// ------------------------- CommandSequence --------------------------

bool CommandSequence::IsExpression() const
{
	ASSERT(!seq.empty());
	return seq.back()->IsExpression(); // like the comma operator
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
