#include "commands.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tree_update.hpp"

using namespace SR;

// ------------------------- Command --------------------------

// ------------------------- PopulateZoneCommand --------------------------

bool PopulateZoneCommand::IsExpression() const
{
	return true;
}


PopulateZoneCommand::PopulateZoneCommand( unique_ptr<Zone> &&zone_, vector<unique_ptr<Command>> &&child_expressions_ ) :
	zone(move(zone_)),
	child_expressions(move(child_expressions_))
{
	ASSERT( zone->GetNumTerminii() == child_expressions.size() );
}	


PopulateZoneCommand::PopulateZoneCommand( unique_ptr<Zone> &&zone_ ) :
	zone(move(zone_))
{
	ASSERT( zone->GetNumTerminii() == 0 );
}	


void PopulateZoneCommand::AddEmbeddedAgentBase( RequiresSubordinateSCREngine *embedded_agent )
{
	embedded_agents.push_back( embedded_agent );
}


const Zone *PopulateZoneCommand::GetZone() const
{
    return zone.get();
}


unique_ptr<Zone> PopulateZoneCommand::Evaluate( const ExecKit &kit ) const	
{
	//FTRACE(zone)("\n");
	FreeZone free_zone;
	if( auto fzp = dynamic_cast<FreeZone *>(zone.get()) )
	{
		free_zone = *fzp;
	}
	else if( auto tzp = dynamic_cast<TreeZone *>(zone.get()) )
	{
		free_zone = tzp->Duplicate( kit.x_tree_db );
	}
	else
	{
		ASSERTFAIL();
	}		
	
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
		
	return make_unique<FreeZone>(free_zone);
}
	
	
void PopulateZoneCommand::Execute( const ExecKit &kit ) const
{
	ASSERTFAIL();
}


string PopulateZoneCommand::GetTrace() const
{
	return "PopulateZoneCommand TODOOOOOOOOOO "+Trace(*zone)+" -> "+Trace(child_expressions);
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
