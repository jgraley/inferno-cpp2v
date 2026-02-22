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

// All the declarators will masquerade as identifiers, since they are 
// found in what would ordinarily be an identifier's place.
typedef CPPTree::Identifier Base;

// You could jsut about imagine a declarator with more than one inner 
// declarator or identifier, but C doesn't have any. This class is for 
// just the one.
struct UniDeclarator : Base
{
	UniDeclarator(TreePtr<Base> inner_);
	TreePtr<Base> inner;	// Can be Identifier or another Declarator
};

// Declarator nodes are named for the type nodes they will resolve into
// not any expression node that just happens to look the same or similar.
struct Indirection : UniDeclarator
{
	using UniDeclarator::UniDeclarator;
};


struct Reference : Indirection   // NOT AddressOf!!!!xs
{
	using Indirection::Indirection;
};


struct Pointer : Indirection
{
	using Indirection::Indirection;
};


struct Array : UniDeclarator // NOT a lookup - gives size of array not index
{
	Array(TreePtr<Base> inner_, TreePtr<CPPTree::Initialiser> size_);

    TreePtr<CPPTree::Initialiser> size; ///< evaluates to the size or Uninitialised if not given eg []
};


struct Function : UniDeclarator // NOT a Call - has params, not args
{
	Function(TreePtr<Base> inner_, Collection<CPPTree::Declaration> params_);

	Collection<CPPTree::Declaration> params; // TODO be Parameter #803
};    

};

#endif
