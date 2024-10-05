#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "free_zone.hpp"
#include "tree_zone.hpp"
#include "../link.hpp"
#include "duplicate.hpp"
#include "../scr_engine.hpp"
#include "fz_expressions.hpp"

namespace SR 
{
class XTreeDatabase;	
class ZoneExpression;
	
// ------------------------- Command --------------------------
    
class Command : public Traceable
{
public:	
	virtual void Execute() const = 0;
};

// ------------------------- UpdateTreeCommand --------------------------

// Replace that part of the tree represented by a target tree zone with
// the contents of a source free zone returned by a child expression.
class UpdateTreeCommand : public Command
{
public:
    UpdateTreeCommand( const TreeZone &target_tree_zone_, 
                       shared_ptr<ZoneExpression> child_expression_ );
	shared_ptr<ZoneExpression> &GetExpression();
	const shared_ptr<ZoneExpression> &GetExpression() const;
	
	void Execute() const final;	

	string GetTrace() const final;

private:
	TreeZone target_tree_zone;
	shared_ptr<ZoneExpression> child_expression;
};

// ------------------------- CommandSequence --------------------------

class CommandSequence : public Command
{
public:
	void Execute() const final;	

	void Add( shared_ptr<Command> new_cmd );
	void AddAtStart( shared_ptr<Command> new_cmd );
    bool IsEmpty() const;
    list<shared_ptr<Command>> &GetCommands();
    void Clear();    
	
	string GetTrace() const final;

private:
	list<shared_ptr<Command>> seq;	
};

}

#endif
