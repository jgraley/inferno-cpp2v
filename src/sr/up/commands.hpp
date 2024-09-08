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

        // For embedded patterns
        const SCREngine *scr_engine;

        // "Register bank" of free zones for workspace
        RegisterFile *register_file;        
    };

	struct Operands
	{
		set<SSAAllocator::Reg> sources;
		set<SSAAllocator::Reg> targets;
		set<SSAAllocator::Reg> dests;
	};

	virtual void DetermineOperandRegs( SSAAllocator &allocator ) = 0;
	virtual Operands GetOperandRegs() const = 0;
	SSAAllocator::Reg GetSourceReg() const;	
	SSAAllocator::Reg GetTargetReg() const;	
	SSAAllocator::Reg GetDestReg() const;	

	virtual void Execute( const ExecKit &kit ) const = 0;
	
protected:
	string OpName( SSAAllocator::Reg reg ) const;
};

// ------------------------- DeclareFreeZoneCommand --------------------------

// Create a new free zone
class DeclareFreeZoneCommand : public Command
{
public:
    DeclareFreeZoneCommand( FreeZone &&zone );
	void DetermineOperandRegs( SSAAllocator &allocator ) final;
	Operands GetOperandRegs() const final;
    const FreeZone *GetFreeZone() const;

	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	unique_ptr<FreeZone> zone;
	SSAAllocator::Reg dest_reg = -1;
};

// ------------------------- DeclareTreeZoneCommand --------------------------

// Create a new tree zone
class DeclareTreeZoneCommand : public Command
{
public:
    DeclareTreeZoneCommand( const TreeZone &zone );
	void DetermineOperandRegs( SSAAllocator &allocator ) final;
	Operands GetOperandRegs() const final;
    const TreeZone *GetTreeZone() const;

	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	TreeZone zone;
	SSAAllocator::Reg dest_reg = -1;
};


// ------------------------- DuplicateZoneCommand --------------------------

// Duplicate a dest tree zone, making a new free zone
class DuplicateZoneCommand : public Command
{
public:
	void DetermineOperandRegs( SSAAllocator &allocator ) final;
	Operands GetOperandRegs() const final;

	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	SSAAllocator::Reg source_reg = -1;
	SSAAllocator::Reg dest_reg = -1;
};

// ------------------------- JoinZoneCommand --------------------------

// Populate one terminus of a dest free zone with a source free zone
class JoinZoneCommand : public Command
{
public:
    explicit JoinZoneCommand(int ti);
	void DetermineOperandRegs( SSAAllocator &allocator ) final;
	Operands GetOperandRegs() const final;
	void SetSourceReg( SSAAllocator::Reg reg );

	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	const int terminus_index;
	SSAAllocator::Reg source_reg = -1;
	SSAAllocator::Reg target_reg = -1;
};

// ------------------------- ModifyTreeCommand --------------------------

// Replace that part of the tree represented by a target tree zone with
// the contents of a source free zone.
class ModifyTreeCommand : public Command
{
public:
	void DetermineOperandRegs( SSAAllocator &allocator ) final;
	Operands GetOperandRegs() const final;
	void SetSourceReg( SSAAllocator::Reg reg );

	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	SSAAllocator::Reg source_reg = -1;
	SSAAllocator::Reg target_reg = -1;
};

// ------------------------- MarkBaseForEmbeddedCommand --------------------------

// Takes the base of the zone at the top of the stack and remembers it as
// the base to use for the configured embedded engine. No change to stack.
class MarkBaseForEmbeddedCommand : public Command
{
public:
    MarkBaseForEmbeddedCommand( RequiresSubordinateSCREngine *embedded_agent );
	void DetermineOperandRegs( SSAAllocator &allocator ) final;
	Operands GetOperandRegs() const final;
	void SetSourceReg( SSAAllocator::Reg reg );

	void Execute( const ExecKit &kit ) const final;	

	string GetTrace() const final;

private:
	RequiresSubordinateSCREngine * const embedded_agent;
	SSAAllocator::Reg source_reg = -1;
};

// ------------------------- CommandSequence --------------------------

class CommandSequence : public Command
{
public:
	void DetermineOperandRegs( SSAAllocator &allocator ) final;
	Operands GetOperandRegs() const final;

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
