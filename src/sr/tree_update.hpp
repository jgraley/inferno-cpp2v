#ifndef TREE_UPDATE_HPP
#define TREE_UPDATE_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "node/tree_ptr.hpp"
#include "link.hpp"

namespace SR 
{
class XTreeDatabase;
	
class Command : public Traceable
{
public:
    struct ExecKit
    {
		// Note: unline EvalKit etc, these pointers are non-const
		// because we intend to actually change thing here.
        XLink root_xlink;
        PatternLink root_plink;
        XTreeDatabase *x_tree_db; 
    };

	virtual void Execute( const ExecKit &kit ) const = 0;
};


class DeleteCommand : public Command
{
public:
    DeleteCommand( XLink target_xlink );
	void Execute( const ExecKit &kit ) const final;	

private:
	XLink target_xlink;
};


class InsertCommand : public Command
{
public:
    InsertCommand( XLink target_xlink, TreePtr<Node> new_x );
	void Execute( const ExecKit &kit ) const final;	

private:
	XLink target_xlink;
	TreePtr<Node> new_x;
};


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
