#ifndef SCOPE_HPP
#define SCOPE_HPP

#include "tree/tree.hpp"

//
// Handy helper to get the node that is the "scope" of the supplied node - ie basically the
// parent in the tree. We have to do searches for this, since the tree does not contain 
// back-pointers.
//
TreePtr<Scope> GetScope( TreePtr<Program> program, TreePtr<Identifier> id );

#endif
