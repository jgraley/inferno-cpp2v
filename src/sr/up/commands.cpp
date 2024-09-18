#include "commands.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tree_update.hpp"
#include "fz_expressions.hpp"

using namespace SR;

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
