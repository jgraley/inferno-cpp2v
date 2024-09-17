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
	
	
namespace UP
{
	struct ExecKit
    {
		// Note: unlike kits in sym, these pointers are non-const
		// because we intend to actually change things here.
        XTreeDatabase *x_tree_db; 

        // For embedded patterns
        const SCREngine *scr_engine;
	};
}
	
// ------------------------- Command --------------------------
    
class Command : public Traceable
{
public:	
	virtual void Execute( const UP::ExecKit &kit ) const = 0;
};

// ------------------------- FreeZoneExpression --------------------------

class FreeZoneExpression : public Traceable
{
public:	
	static void ForDepthFirstWalk( const FreeZoneExpression *base,
	                               function<void(const FreeZoneExpression *cmd)> func_in,
		                           function<void(const FreeZoneExpression *cmd)> func_out );
	virtual void DepthFirstWalkImpl(function<void(const FreeZoneExpression *cmd)> func_in,
			                        function<void(const FreeZoneExpression *cmd)> func_out) const { ASSERTFAIL(); }
	virtual unique_ptr<Zone> Evaluate( const UP::ExecKit &kit ) const { ASSERTFAIL(); }
};

// ------------------------- PopulateZoneOperator --------------------------

// Construct with any zone and optional marker M. On evaluate: populate the
// zone, apply marker and return the resulting FreeZone. 
class PopulateZoneOperator : public FreeZoneExpression
{
protected:
    PopulateZoneOperator( vector<unique_ptr<FreeZoneExpression>> &&child_expressions_ );
    PopulateZoneOperator();

public:
    void AddEmbeddedAgentBase( RequiresSubordinateSCREngine *embedded_agent );
    
    //const Zone *GetZone() const;
    int GetNumChildExpressions() const;
    
	void DepthFirstWalkImpl(function<void(const FreeZoneExpression *cmd)> func_in,
			                function<void(const FreeZoneExpression *cmd)> func_out) const final;

	void PopulateFreeZone( FreeZone &free_zone, const UP::ExecKit &kit ) const;	

	string GetTrace() const final;

private:
	unique_ptr<Zone> zone;
	vector<unique_ptr<FreeZoneExpression>> child_expressions;
	std::list<RequiresSubordinateSCREngine *> embedded_agents;
};


// ------------------------- PopulateTreeZoneOperator --------------------------

// Construct with tree zone and optional marker M. On evaluate: populate the
// zone, apply marker and return the resulting FreeZone. 
class PopulateTreeZoneOperator : public PopulateZoneOperator
{
public:
    PopulateTreeZoneOperator( unique_ptr<TreeZone> &&zone_, vector<unique_ptr<FreeZoneExpression>> &&child_expressions );
    PopulateTreeZoneOperator( unique_ptr<TreeZone> &&zone_ );
    
    const TreeZone *GetZone() const;
	unique_ptr<Zone> Evaluate( const UP::ExecKit &kit ) const final;	
    
private:
	unique_ptr<TreeZone> zone;
};

// ------------------------- PopulateFreeZoneOperator --------------------------

// Construct with free zone and optional marker M. On evaluate: populate the
// zone, apply marker and return the resulting FreeZone. 
class PopulateFreeZoneOperator : public PopulateZoneOperator
{
public:
    PopulateFreeZoneOperator( unique_ptr<FreeZone> &&zone_, vector<unique_ptr<FreeZoneExpression>> &&child_expressions );
    PopulateFreeZoneOperator( unique_ptr<FreeZone> &&zone_ );

    const FreeZone *GetZone() const;
   	unique_ptr<Zone> Evaluate( const UP::ExecKit &kit ) const final;	

private:
	unique_ptr<FreeZone> zone;
};

// ------------------------- UpdateTreeCommand --------------------------

// Replace that part of the tree represented by a target tree zone with
// the contents of a source free zone returned by a child expression.
class UpdateTreeCommand : public Command
{
public:
    UpdateTreeCommand( const TreeZone &target_tree_zone_, unique_ptr<FreeZoneExpression> child_expression_ );
	const FreeZoneExpression *GetExpression() const;
	
	void Execute( const UP::ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	TreeZone target_tree_zone;
	unique_ptr<FreeZoneExpression> child_expression;
};

// ------------------------- CommandSequence --------------------------

class CommandSequence : public Command
{
public:
	void Execute( const UP::ExecKit &kit ) const final;	

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
