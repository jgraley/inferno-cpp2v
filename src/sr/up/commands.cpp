#include "commands.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tree_update.hpp"

using namespace SR;

// ------------------------- FreeZoneExpression --------------------------

void FreeZoneExpression::ForDepthFirstWalk( const FreeZoneExpression *base,
	                                        function<void(const FreeZoneExpression *cmd)> func_in,
	                                        function<void(const FreeZoneExpression *cmd)> func_out ) try
{
	base->DepthFirstWalkImpl(func_in, func_out);
}
catch( BreakException )
{
}		       

// ------------------------- PopulateZoneOperator --------------------------

PopulateZoneOperator::PopulateZoneOperator( vector<unique_ptr<FreeZoneExpression>> &&child_expressions_ ) :
	child_expressions(move(child_expressions_))
{
}	


PopulateZoneOperator::PopulateZoneOperator()
{
}	


void PopulateZoneOperator::AddEmbeddedAgentBase( RequiresSubordinateSCREngine *embedded_agent )
{
	embedded_agents.push_back( embedded_agent );
}


int PopulateZoneOperator::GetNumChildExpressions() const
{
	return child_expressions.size();
}


void PopulateZoneOperator::DepthFirstWalkImpl(function<void(const FreeZoneExpression *cmd)> func_in,
			                                 function<void(const FreeZoneExpression *cmd)> func_out) const
{
	if( func_in )
		func_in(this);
	for( const unique_ptr<FreeZoneExpression> &child_expression : child_expressions )
		child_expression->DepthFirstWalkImpl(func_in, func_out);
	if( func_out )
		func_out(this);
}


void PopulateZoneOperator::PopulateFreeZone( FreeZone &free_zone, const UP::ExecKit &kit ) const	
{
	//FTRACE(free_zone)("\n");
	vector<FreeZone> child_zones;
	for( const unique_ptr<FreeZoneExpression> &child_expression : child_expressions )
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
	

string PopulateZoneOperator::GetTrace() const
{
	return "PopulateZoneOperator TODOOOOOOOOOO "+Trace(child_expressions);
}

// ------------------------- PopulateTreeZoneOperator --------------------------

PopulateTreeZoneOperator::PopulateTreeZoneOperator( unique_ptr<TreeZone> &&zone_, 
                                                  vector<unique_ptr<FreeZoneExpression>> &&child_expressions ) :
	PopulateZoneOperator( move(child_expressions) ),
	zone(move(zone_))
{
	ASSERT( zone->GetNumTerminii() == GetNumChildExpressions() );	
}	
		

PopulateTreeZoneOperator::PopulateTreeZoneOperator( unique_ptr<TreeZone> &&zone_ ) :
	zone(move(zone_))
{
	ASSERT( zone->GetNumTerminii() == 0 );	
}


const TreeZone *PopulateTreeZoneOperator::GetZone() const
{
    return zone.get();
}


unique_ptr<Zone> PopulateTreeZoneOperator::Evaluate( const UP::ExecKit &kit ) const
{
	FreeZone free_zone = zone->Duplicate( kit.x_tree_db );
	PopulateFreeZone( free_zone, kit );
	return make_unique<FreeZone>(free_zone);
}


// ------------------------- PopulateFreeZoneOperator --------------------------

PopulateFreeZoneOperator::PopulateFreeZoneOperator( unique_ptr<FreeZone> &&zone_, 
                                                  vector<unique_ptr<FreeZoneExpression>> &&child_expressions ) :
	PopulateZoneOperator( move(child_expressions) ),
	zone(move(zone_))
{
	ASSERT( zone->GetNumTerminii() == GetNumChildExpressions() );	
}

		
PopulateFreeZoneOperator::PopulateFreeZoneOperator( unique_ptr<FreeZone> &&zone_ ) :
   	PopulateZoneOperator(),
   	zone(move(zone_))
{
	ASSERT( zone->GetNumTerminii() == 0 );	
}


const FreeZone *PopulateFreeZoneOperator::GetZone() const
{
    return zone.get();
}


unique_ptr<Zone> PopulateFreeZoneOperator::Evaluate( const UP::ExecKit &kit ) const
{
	PopulateFreeZone( *zone, kit );
	return make_unique<FreeZone>(*zone);
}

// ------------------------- UpdateTreeCommand --------------------------

UpdateTreeCommand::UpdateTreeCommand( const TreeZone &target_tree_zone_, 
                                      unique_ptr<FreeZoneExpression> child_expression_ ) :
	target_tree_zone( target_tree_zone_ ),
	child_expression( move(child_expression_) )
{
}


const FreeZoneExpression *UpdateTreeCommand::GetExpression() const
{
	return child_expression.get();
}



void UpdateTreeCommand::Execute( const UP::ExecKit &kit ) const
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

void CommandSequence::Execute( const UP::ExecKit &kit ) const
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
