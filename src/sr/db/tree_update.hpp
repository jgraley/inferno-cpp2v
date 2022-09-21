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
		// because we intend to actually change thing here.
        XLink root_xlink;
        XTreeDatabase *x_tree_db; 
    };

	virtual void Execute( const ExecKit &kit ) const = 0;
};


class DeleteCommand : public UpdateCommand
{
public:
    DeleteCommand( XLink target_xlink );
	void Execute( const ExecKit &kit ) const final;	

private:
	XLink target_xlink;
};


class InsertCommand : public UpdateCommand
{
public:
    InsertCommand( XLink target_xlink, TreePtr<Node> new_x, PatternLink base_plink );
	void Execute( const ExecKit &kit ) const final;	

private:
	XLink target_xlink;
	TreePtr<Node> new_x;
	PatternLink base_plink;
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
