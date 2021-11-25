#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "tree/cpptree.hpp"

//
// Handy helper to get the node that is the "scope" of the supplied node - ie basically the
// parent in the tree. We have to do searches for this, since the tree does not contain 
// back-pointers.
//
class ScopeNotFoundMismatch : public Mismatch {};
class ScopeOnNonSpecificMismatch : public Mismatch {};

TreePtr<CPPTree::Scope> GetScope( TreePtr<CPPTree::Program> program, TreePtr<CPPTree::Identifier> id );

#endif
