#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "zone.hpp"
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
		// Note: unline EvalKit etc, these pointers are non-const
		// because we intend to actually change things here.
        XTreeDatabase *x_tree_db; 
        
        // Remove after #702 and just use x_tree_db directly
        const Duplicate::DirtyGrassUpdateInterface *green_grass;

        // For embedded patterns
        const SCREngine *scr_engine;

        // Forth-like stack of generated nodes.
        // Rule: we push in natural order, and correct the reversal when reading
        stack<FreeZone> *free_zone_stack;        
        map<int, FreeZone> *free_zone_regs;        
    };

	virtual void SetOperands( int &pseudo_stack_top ) = 0;

	virtual void Execute( const ExecKit &kit ) const = 0;
	
protected:
	string OpName( int reg ) const;
};

// ------------------------- ImmediateTreeZoneCommand --------------------------

class ImmediateTreeZoneCommand : public Command
{
public:
    ImmediateTreeZoneCommand( const TreeZone &zone );

    const TreeZone *GetTreeZone() const;
    
protected:
	TreeZone zone;
};

// ------------------------- DeclareFreeZoneCommand --------------------------

// Put a free zone onto the stack.
class DeclareFreeZoneCommand : public Command
{
public:
    DeclareFreeZoneCommand( FreeZone &&zone );
	void SetOperands( int &pseudo_stack_top ) final;
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	unique_ptr<FreeZone> zone;
	int dest_reg = -1;
};

// ------------------------- DuplicateTreeZoneCommand --------------------------

// Duplicate a tree zone, making a free zone, and push it to the stack.
class DuplicateTreeZoneCommand : public ImmediateTreeZoneCommand
{
public:
    using ImmediateTreeZoneCommand::ImmediateTreeZoneCommand;
	void SetOperands( int &pseudo_stack_top ) final;
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	int dest_reg = -1;
};

// ------------------------- JoinFreeZoneCommand --------------------------

// Populate one terminus of a free zone. Source zone should be on 
// top of stack and will be popped, and then dest free zone which will 
// be peeked.
class JoinFreeZoneCommand : public Command
{
public:
    explicit JoinFreeZoneCommand(int ti);
	void SetOperands( int &pseudo_stack_top ) final;
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	const int terminus_index;
	int source_reg = -1;
	int dest_reg = -1;
};

// ------------------------- DeleteCommand --------------------------

class DeleteCommand : public Command
{
public:
    DeleteCommand( XLink target_base_xlink );
	void SetOperands( int &pseudo_stack_top ) final;
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	XLink target_base_xlink;
};

// ------------------------- InsertCommand --------------------------

class InsertCommand : public Command
{
public:
    InsertCommand( XLink target_base_xlink );
	void SetOperands( int &pseudo_stack_top ) final;
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	XLink target_base_xlink;
};

// ------------------------- MarkBaseForEmbeddedCommand --------------------------

// Takes the base of the zone at the top of the stack and remembers it as
// the base to use for the configured embedded engine. No change to stack.
class MarkBaseForEmbeddedCommand : public Command
{
public:
    MarkBaseForEmbeddedCommand( RequiresSubordinateSCREngine *embedded_agent );
	void SetOperands( int &pseudo_stack_top ) final;
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	RequiresSubordinateSCREngine * const embedded_agent;
	int dest_reg = -1;
};

// ------------------------- CommandSequence --------------------------

class CommandSequence : public Command
{
public:
	void SetOperands( int &pseudo_stack_top ) final;
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
