#ifndef DECLARATORS_HPP
#define DECLARATORS_HPP

#include "node/specialise_oostd.hpp"
#include "tree/type_data.hpp"
#include "tree/cpptree.hpp"
#include <string>

namespace Declarators {

// All the declarators will masquerade as identifiers, since they are 
// found in what would ordinarily be an identifier's place.
typedef CPPTree::Identifier Base;

// You could jsut about imagine a declarator with more than one inner 
// declarator or identifier, but C doesn't have any. This class is for 
// just the one.
struct UniDeclarator : Base
{
	NODE_FUNCTIONS
	TreePtr<Base> inner;	// Can be Identifier or another Declarator
};

// Declarator nodes are named for the type nodes they will resolve into
// not any expression node that just happens to look the same or similar.
struct Indirection : UniDeclarator
{
	NODE_FUNCTIONS
};


struct Reference : Indirection   // NOT AddressOf!!!!xs
{
	NODE_FUNCTIONS
};


struct Pointer : Indirection
{
	NODE_FUNCTIONS 
};


struct Array : UniDeclarator // NOT a lookup - gives size of array not index
{
	NODE_FUNCTIONS

    TreePtr<CPPTree::Initialiser> size; ///< evaluates to the size or Uninitialised if not given eg []
};


struct Function : UniDeclarator // NOT a Call - has params, not args
{
	NODE_FUNCTIONS

	Collection<CPPTree::Declaration> params; // TODO be Parameter #803
};    

};

#endif
