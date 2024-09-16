#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "free_zone.hpp"
#include "tree_zone.hpp"
#include "../link.hpp"
#include "duplicate.hpp"
#include "../scr_engine.hpp"

namespace SR 
{
class XTreeDatabase;
	
// ------------------------- Command --------------------------
    
class Command : public Traceable
{
public:
    struct ExecKit
    {
		// Note: unlike kits in sym, these pointers are non-const
		// because we intend to actually change things here.
        XTreeDatabase *x_tree_db; 

        // For embedded patterns
        const SCREngine *scr_engine;
	};

	virtual bool IsExpression() const = 0;

    // These erally want to be in an Expression class
	void ForWalk( function<void(const Command *cmd)> func_in,
		          function<void(const Command *cmd)> func_out ) const;
	virtual void WalkImpl(function<void(const Command *cmd)> func_in,
			              function<void(const Command *cmd)> func_out) const { ASSERTFAIL(); }
	virtual unique_ptr<Zone> Evaluate( const ExecKit &kit ) const { ASSERTFAIL(); }
	
	virtual void Execute( const ExecKit &kit ) const = 0;
};

// ------------------------- PopulateZoneCommand --------------------------

// Construct with any zone and optional marker M. On evaluate: populate the
// zone, apply marker and return the resulting FreeZone. 
class PopulateZoneCommand : public Command
{
protected:
    PopulateZoneCommand( vector<unique_ptr<Command>> &&child_expressions_ );
    PopulateZoneCommand();

public:
    void AddEmbeddedAgentBase( RequiresSubordinateSCREngine *embedded_agent );
    
	bool IsExpression() const final;
    //const Zone *GetZone() const;
    int GetNumChildExpressions() const;
    
	void WalkImpl(function<void(const Command *cmd)> func_in,
			      function<void(const Command *cmd)> func_out) const final;

	void PopulateFreeZone( FreeZone &free_zone, const ExecKit &kit ) const;	
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	unique_ptr<Zone> zone;
	vector<unique_ptr<Command>> child_expressions;
	std::list<RequiresSubordinateSCREngine *> embedded_agents;
};


// ------------------------- PopulateTreeZoneCommand --------------------------

// Construct with tree zone and optional marker M. On evaluate: populate the
// zone, apply marker and return the resulting FreeZone. 
class PopulateTreeZoneCommand : public PopulateZoneCommand
{
public:
    PopulateTreeZoneCommand( unique_ptr<TreeZone> &&zone_, vector<unique_ptr<Command>> &&child_expressions );
    PopulateTreeZoneCommand( unique_ptr<TreeZone> &&zone_ );
    
    const TreeZone *GetZone() const;
	unique_ptr<Zone> Evaluate( const ExecKit &kit ) const final;	
    
private:
	unique_ptr<TreeZone> zone;
};

// ------------------------- PopulateFreeZoneCommand --------------------------

// Construct with free zone and optional marker M. On evaluate: populate the
// zone, apply marker and return the resulting FreeZone. 
class PopulateFreeZoneCommand : public PopulateZoneCommand
{
public:
    PopulateFreeZoneCommand( unique_ptr<FreeZone> &&zone_, vector<unique_ptr<Command>> &&child_expressions );
    PopulateFreeZoneCommand( unique_ptr<FreeZone> &&zone_ );

    const FreeZone *GetZone() const;
   	unique_ptr<Zone> Evaluate( const ExecKit &kit ) const final;	

private:
	unique_ptr<FreeZone> zone;
};

// ------------------------- UpdateTreeCommand --------------------------

// Replace that part of the tree represented by a target tree zone with
// the contents of a source free zone returned by a child expression.
class UpdateTreeCommand : public Command
{
public:
    UpdateTreeCommand( const TreeZone &target_tree_zone_, unique_ptr<Command> child_expression_ );
	bool IsExpression() const final;
	const Command *GetExpression() const;
	
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	TreeZone target_tree_zone;
	unique_ptr<Command> child_expression;
};

// ------------------------- CommandSequence --------------------------

class CommandSequence : public Command
{
public:
	bool IsExpression() const final;

	void Execute( const ExecKit &kit ) const final;	

	void Add( unique_ptr<Command> new_cmd );
	void AddAtStart( unique_ptr<Command> new_cmd );
    bool IsEmpty() const;
    list<unique_ptr<Command>> &GetCommands();
    void Clear();    
	
	string GetTrace() const final;

private:
	list<unique_ptr<Command>> seq;	
};

}

#endif
