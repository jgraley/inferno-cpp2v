#ifndef TREE_UPDATE_HPP
#define TREE_UPDATE_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../zone.hpp"
#include "../link.hpp"
#include "../duplicate.hpp"
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
    };

	enum OperandMode
	{
		IMMEDIATE,
		STACK
	};

	virtual void Execute( const ExecKit &kit ) const = 0;
};

// ------------------------- PopulateFreeZoneCommand --------------------------

// Populate a free zone from the stack. Push the resulting subtree to the stack.
// TODO fill in new zone terminii from sub zone terminii
class PopulateFreeZoneCommand : public Command
{
public:
    explicit PopulateFreeZoneCommand( const FreeZone &zone );
    explicit PopulateFreeZoneCommand();
	void Execute( const ExecKit &kit ) const final;	

private:
    const OperandMode op_mode;
	mutable unique_ptr<FreeZone> imm_zone; // TODO a deep copy?
};

// ------------------------- DuplicateTreeZoneCommand --------------------------

// Duplicate a tree zone, making a free zone, and push it to the stack.
class DuplicateTreeZoneCommand : public Command
{
public:
    DuplicateTreeZoneCommand( const TreeZone &zone );
	void Execute( const ExecKit &kit ) const final;	

private:
	TreeZone zone;
};

// ------------------------- DuplicateAndPopulateTreeZoneCommand --------------------------

// Duplicate a tree zone, making a free zone, and populate it from the stack.
// Push the resulting subtree to the stack.
class DuplicateAndPopulateTreeZoneCommand : public Command
{
public:
    DuplicateAndPopulateTreeZoneCommand( const TreeZone &zone );
	void Execute( const ExecKit &kit ) const final;	

private:
	TreeZone zone;
};

// ------------------------- DeleteCommand --------------------------

class DeleteCommand : public Command
{
public:
    DeleteCommand( XLink target_base_xlink );
	void Execute( const ExecKit &kit ) const final;	

private:
	XLink target_base_xlink;
};

// ------------------------- InsertCommand --------------------------

class InsertCommand : public Command
{
public:
    InsertCommand( XLink target_base_xlink );
	void Execute( const ExecKit &kit ) const final;	

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
	void Execute( const ExecKit &kit ) const final;	

private:
	RequiresSubordinateSCREngine * const embedded_agent;
};

// ------------------------- CommandSequence --------------------------

class CommandSequence : public Command
{
public:
	void Execute( const ExecKit &kit ) const final;	

	void Add( unique_ptr<Command> cmd );
    bool IsEmpty() const;
	
private:
	list<unique_ptr<Command>> seq;	
};
	
}

#endif
