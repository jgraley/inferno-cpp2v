#ifndef TREE_UPDATE_HPP
#define TREE_UPDATE_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../zone.hpp"
#include "../link.hpp"

namespace SR 
{
class XTreeDatabase;
	
class UpdateCommand : public Traceable
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


class PushCommand : public UpdateCommand
{
public:
    PushCommand( const FreeZone &new_zone );
	void Execute( const ExecKit &kit ) const final;	

private:
	FreeZone new_zone;
};


class DeleteCommand : public UpdateCommand
{
public:
    DeleteCommand( const TreeZone &target );
	void Execute( const ExecKit &kit ) const final;	

private:
	TreeZone target;
};


class InsertCommand : public UpdateCommand
{
public:
    InsertCommand( const TreeZone &target );
	void Execute( const ExecKit &kit ) const final;	

private:
	TreeZone target;
};


class CommandSequence : public UpdateCommand
{
public:
	void Add( shared_ptr<UpdateCommand> cmd );
	void Execute( const ExecKit &kit ) const final;	
	
private:
	list<shared_ptr<UpdateCommand>> seq;	
};
	
}

#endif
