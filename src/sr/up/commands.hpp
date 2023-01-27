#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "zone.hpp"
#include "../link.hpp"
#include "duplicate.hpp"
#include "../scr_engine.hpp"
#include "ssa_allocator.hpp"

namespace SR 
{
class XTreeDatabase;
	
// ------------------------- Command --------------------------
    
class Command : public Traceable
{
public:
	typedef map<SSAAllocator::Reg, unique_ptr<Zone>> RegisterFile;

    struct ExecKit
    {
		// Note: unline EvalKit etc, these pointers are non-const
		// because we intend to actually change things here.
        XTreeDatabase *x_tree_db; 
        
        // Remove after #702 and just use x_tree_db directly
        const Duplicate::DirtyGrassUpdateInterface *green_grass;

        // For embedded patterns
        const SCREngine *scr_engine;

        // "Register bank" of free zones for workspace
        RegisterFile *register_file;        
    };

	virtual void SetOperandRegs( SSAAllocator &allocator ) = 0;

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

// Create a free zone
class DeclareFreeZoneCommand : public Command
{
public:
    DeclareFreeZoneCommand( FreeZone &&zone );
	void SetOperandRegs( SSAAllocator &allocator ) final;
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	unique_ptr<FreeZone> zone;
	int dest_reg = -1;
};

// ------------------------- DeclareTreeZoneCommand --------------------------

// Create a tree zone
class DeclareTreeZoneCommand : public ImmediateTreeZoneCommand
{
public:
    using ImmediateTreeZoneCommand::ImmediateTreeZoneCommand;
	void SetOperandRegs( SSAAllocator &allocator ) final;
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	int dest_reg = -1;
};


// ------------------------- DuplicateZoneCommand --------------------------

// Duplicate a tree zone, making a free zone
class DuplicateZoneCommand : public Command
{
public:
	void SetOperandRegs( SSAAllocator &allocator ) final;
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	int dest_reg = -1;
};

// ------------------------- JoinZoneCommand --------------------------

// Populate one terminus of a free zone. Source zone should be on 
// top of stack and will be popped, and then dest free zone which will 
// be peeked.
class JoinZoneCommand : public Command
{
public:
    explicit JoinZoneCommand(int ti);
	void SetOperandRegs( SSAAllocator &allocator ) final;
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	const int terminus_index;
	int source_reg = -1;
	int dest_reg = -1;
};

// ------------------------- ModifyZoneCommand --------------------------

class ModifyZoneCommand : public Command
{
public:
    ModifyZoneCommand( TreeZone target_zone );
	void SetOperandRegs( SSAAllocator &allocator ) final;
	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	TreeZone target_zone;
	int source_reg = -1;
};

// ------------------------- MarkBaseForEmbeddedCommand --------------------------

// Takes the base of the zone at the top of the stack and remembers it as
// the base to use for the configured embedded engine. No change to stack.
class MarkBaseForEmbeddedCommand : public Command
{
public:
    MarkBaseForEmbeddedCommand( RequiresSubordinateSCREngine *embedded_agent );
	void SetOperandRegs( SSAAllocator &allocator ) final;
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
	void SetOperandRegs( SSAAllocator &allocator ) final;
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
