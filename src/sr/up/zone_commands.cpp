#include "zone_commands.hpp"

#include "db/x_tree_database.hpp"
#include "common/read_args.hpp"
#include "tree/validate.hpp"
#include "common/lambda_loops.hpp"
#include "tree_update.hpp"
#include "zone_expressions.hpp"

using namespace SR;

// ------------------------- ReplaceCommand --------------------------

ReplaceCommand::ReplaceCommand( const TreeZone &target_tree_zone_, 
                                      shared_ptr<ZoneExpression> child_expression_ ) :
	target_tree_zone( target_tree_zone_ ),
	child_expression( move(child_expression_) )
{
}


shared_ptr<ZoneExpression> &ReplaceCommand::GetExpression()
{
	ASSERT( this );
	return child_expression;
}


const shared_ptr<ZoneExpression> &ReplaceCommand::GetExpression() const
{
	ASSERT( this );
	return child_expression;
}


const TreeZone &ReplaceCommand::GetTargetTreeZone() const
{
	ASSERT( this );
	return target_tree_zone;
}


void ReplaceCommand::Execute(const UpEvalExecKit &kit) const
{
    // New zone must be a free zone
	unique_ptr<FreeZone> source_free_zone = child_expression->Evaluate(kit);
    kit.x_tree_db->MainTreeReplace( target_tree_zone, *source_free_zone );
}


string ReplaceCommand::GetTrace() const
{
	return "ReplaceCommand( \nsource: "+Trace(child_expression)+", \ntarget: "+Trace(target_tree_zone)+" )";
}

// ------------------------- CommandSequence --------------------------

void CommandSequence::Execute(const UpEvalExecKit &kit) const
{
	//FTRACE(" executing");
	for( const shared_ptr<Command> &cmd : seq )
		cmd->Execute(kit);
}
	
    
void CommandSequence::Add( shared_ptr<Command> new_cmd )
{
    seq.push_back(move(new_cmd));
}


bool CommandSequence::IsEmpty() const
{
	return seq.empty();
}


list<shared_ptr<Command>> &CommandSequence::GetCommands()
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
    for( const shared_ptr<Command> &pc : seq )
        elts.push_back( Trace(*pc) );
    return Join( elts, "\n", "CommandSequence[\n", " ]\n" );
}
