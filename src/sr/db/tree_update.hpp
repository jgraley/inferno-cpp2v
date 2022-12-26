#ifndef TREE_UPDATE_HPP
#define TREE_UPDATE_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../zone.hpp"
#include "../link.hpp"

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
        
        // Forth-like stack of generated nodes.
        stack<FreeZone> *free_zone_stack;
    };

	virtual void Execute( const ExecKit &kit ) const = 0;
};

// ------------------------- PushFreeZoneCommand --------------------------

class PushFreeZoneCommand : public Command
{
public:
    PushFreeZoneCommand( const FreeZone &new_zone );
	void Execute( const ExecKit &kit ) const final;	

private:
	FreeZone new_zone;
};

// ------------------------- DeleteCommand --------------------------

class DeleteCommand : public Command
{
public:
    DeleteCommand( const TreeZone &target );
	void Execute( const ExecKit &kit ) const final;	

private:
	TreeZone target;
};

// ------------------------- InsertCommand --------------------------

class InsertCommand : public Command
{
public:
    InsertCommand( const TreeZone &target );
	void Execute( const ExecKit &kit ) const final;	

private:
	TreeZone target;
};

// ------------------------- CommandSequence --------------------------

class CommandSequence : public Command
{
public:
	void Add( shared_ptr<Command> cmd );
	void Execute( const ExecKit &kit ) const final;	
	
private:
	list<shared_ptr<Command>> seq;	
};
	
}

#endif
