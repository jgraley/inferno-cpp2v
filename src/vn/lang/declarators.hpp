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

struct Result
{
	bool abstract;
	TreePtr<Node> innermost;
};


// Declarator node
struct Declarator : Node
{
   	virtual Result DeclaratorReduce( TreePtr<Node> &type ) const = 0;
   	static Result DoReduce( TreePtr<Node> inner, TreePtr<Node> &type );
};

// You could just about imagine a declarator with more than one inner 
// declarator or identifier, but C doesn't have any. This class is for 
// just the one.
struct UniDeclarator : Declarator
{
	UniDeclarator(TreePtr<Node> inner_);
	
	TreePtr<Node> inner;	// Can be Identifier or another Declarator
};

// You could just about imagine a declarator with more than one inner 
// declarator or identifier, but C doesn't have any. This class is for 
// just the one.
struct Concrete : UniDeclarator
{
	using UniDeclarator::UniDeclarator;
	
   	Result DeclaratorReduce( TreePtr<Node> &type ) const override;
};

// Declarator nodes are named for the type nodes they will resolve into
// not any expression node that just happens to look the same or similar.
struct Indirection : UniDeclarator
{
	using UniDeclarator::UniDeclarator;
};


struct Pointer : Indirection
{
	using Indirection::Indirection;	

   	Result DeclaratorReduce( TreePtr<Node> &type ) const override;
};


struct Reference : Indirection   // NOT AddressOf!!!!
{
	using Indirection::Indirection;

   	Result DeclaratorReduce( TreePtr<Node> &type ) const override;
};


struct Array : UniDeclarator // NOT a lookup - gives size of array not index
{
	Array(TreePtr<Node> inner_, TreePtr<Node> size_);

   	Result DeclaratorReduce( TreePtr<Node> &type ) const override;

    TreePtr<CPPTree::Initialiser> size; ///< evaluates to the size or Uninitialised if not given eg []
};


struct Function : UniDeclarator // NOT a Call - has params, not args
{
	Function(TreePtr<Node> inner_, list<TreePtr<Node>> params_);

   	Result DeclaratorReduce( TreePtr<Node> &type ) const override;

	list<TreePtr<Node>> params; 
};    

};

#endif
