#ifndef SORT_DECLS_HPP
#define SORT_DECLS_HPP

#include "common/common.hpp"
#include "tree/cpptree.hpp"
#include "helpers/walk.hpp"

// Does declaration of a depend on (ie use) declaration of b?
// Set ignore_ptr_ref_record to make function ignore the case where a's type contains a pointer or 
// reference to b
bool IsDependOn( TreePtr<CPPTree::Declaration> a, TreePtr<CPPTree::Declaration> b, bool ignore_indirection_to_record=false );

// Sort some declarations into an order that will compile successfully.
// Optionally ignore pointers and references to records: they can be cyclic and so cannot be sorted
// on. Set to "true" if incomplete declarations for all records have already been emitted. 
Sequence<CPPTree::Declaration> SortDecls( ContainerInterface &c, bool ignore_indirection_to_record );

// Mix them up randomly!!
Sequence<CPPTree::Declaration> JumbleDecls( Sequence<CPPTree::Declaration> c );
Sequence<CPPTree::Declaration> ReverseDecls( Sequence<CPPTree::Declaration> c );

#endif