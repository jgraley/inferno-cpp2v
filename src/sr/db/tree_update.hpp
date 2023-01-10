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

	virtual void Execute( const ExecKit &kit ) const = 0;
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
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	unique_ptr<FreeZone> zone;
};

// ------------------------- DuplicateTreeZoneCommand --------------------------

// Duplicate a tree zone, making a free zone, and push it to the stack.
class DuplicateTreeZoneCommand : public ImmediateTreeZoneCommand
{
public:
    using ImmediateTreeZoneCommand::ImmediateTreeZoneCommand;
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;
};

// ------------------------- PopulateFreeZoneCommand --------------------------

// Populate a free zone and push the resulting subtree 
// to the stack. Free zone to populate should be on top of stack, 
// and then the overlay free zones, in push order.
// TODO fill in new zone terminii from sub zone terminii?
class PopulateFreeZoneCommand : public Command
{
public:
    explicit PopulateFreeZoneCommand();
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;
};

// ------------------------- DeleteCommand --------------------------

class DeleteCommand : public Command
{
public:
    DeleteCommand( XLink target_base_xlink );
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
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	RequiresSubordinateSCREngine * const embedded_agent;
};

// ------------------------- CommandSequence --------------------------

class CommandSequence : public Command
{
public:
	void Execute( const ExecKit &kit ) const final;	

	void Add( unique_ptr<Command> new_cmd );
    bool IsEmpty() const;
    const list<unique_ptr<Command>> &GetSeq() const;
	
	string GetTrace() const final;

private:
	list<unique_ptr<Command>> seq;	
};

// ------------------------- Runners --------------------------

FreeZone RunGetFreeZoneNoDB( unique_ptr<Command> cmd, const SCREngine *scr_engine );
void RunVoidForReplace( unique_ptr<Command> cmd, const SCREngine *scr_engine, XTreeDatabase *x_tree_db );
	
// ------------------------- TreeZoneOverlapFinder --------------------------

class TreeZoneOverlapFinder
{
public:
	TreeZoneOverlapFinder( const XTreeDatabase *db, CommandSequence *seq );
	
private:	
    map<const TreeZone *, const ImmediateTreeZoneCommand *> tzps_to_commands;
    map<const TreeZone *, set<const TreeZone *>> overlapping_zones;
};

}

#endif
