#ifndef DECLARATORS_HPP
#define DECLARATORS_HPP

#include "node/specialise_oostd.hpp"
#include "tree/type_data.hpp"
#include "tree/cpptree.hpp"
#include <string>

namespace Declarators {

//TODO invent NODE_FUNCTIONS_DELETE and use here for safety 
// as well as maybe the scaffolding used in tree update.
// It should fail out with a message that includes the name of the macro.
// Note: it looks like you have to use make_shared<> not MakeTreePtr<>
// with such classes, but why do we need MakeTreePtr<> anyway?
// The benefits of TreePtr are
// - polymorphic double-indirection (TeeePtrInterface)
// - Itemisability

struct CVQuals
{
	TreePtr<Node> constancy;
	// TODO volatility
};


struct Result
{
	enum Outcome
	{
		ABSTRACT,
		CONCRETE,
		WILDCARD
	};
	
	Outcome outcome;
	TreePtr<Node> type_view;
	CVQuals cv_quals_view;
	TreePtr<Node> leaf;
};


// Declarator node
struct Declarator : Node
{
   	virtual Result DeclaratorReduce( TreePtr<Node> type_view, CVQuals cv_quals_view ) const = 0;
   	static Result DoReduce( TreePtr<Node> child, TreePtr<Node> type, CVQuals cv_quals_view );
};

// You could just about imagine a declarator with more than one child 
// declarator or identifier, but C doesn't have any. This class is for 
// just the one.
struct UniDeclarator : Declarator
{
	UniDeclarator(TreePtr<Node> child_);
	
	TreePtr<Node> child;	// Can be Identifier or another Declarator
};

// You could just about imagine a declarator with more than one child 
// declarator or identifier, but C doesn't have any. This class is for 
// just the one.
struct Concrete : UniDeclarator
{
	using UniDeclarator::UniDeclarator;
	
   	Result DeclaratorReduce( TreePtr<Node> type_view, CVQuals cv_quals_view ) const override;
};

// Declarator nodes are named for the type nodes they will resolve into
// not any expression node that just happens to look the same or similar.
struct Indirection : UniDeclarator
{
	using UniDeclarator::UniDeclarator;
};


struct Pointer : Indirection
{
	Pointer(CVQuals cv_quals_decl_, TreePtr<Node> child_);

   	Result DeclaratorReduce( TreePtr<Node> type_view, CVQuals cv_quals_view ) const override;
   	CVQuals cv_quals_decl;
};


struct Reference : Indirection   // NOT AddressOf!!!!
{
	using Indirection::Indirection;

   	Result DeclaratorReduce( TreePtr<Node> type_view, CVQuals cv_quals_view ) const override;
};


struct Array : UniDeclarator // NOT a lookup - gives size of array not index
{
	Array(TreePtr<Node> child_, TreePtr<Node> size_);

   	Result DeclaratorReduce( TreePtr<Node> type_view, CVQuals cv_quals_view ) const override;

    TreePtr<CPPTree::Initialiser> size; ///< evaluates to the size or Uninitialised if not given eg []
};


struct Function : UniDeclarator // NOT a Call - has params, not args
{
	Function(TreePtr<Node> child_, list<TreePtr<Node>> params_, CVQuals cv_quals_decl_);

   	Result DeclaratorReduce( TreePtr<Node> type_view, CVQuals cv_quals_view ) const override;

	list<TreePtr<Node>> params; 
   	CVQuals cv_quals_decl;
};    

};

#endif
