#ifndef SORT_DECLS_HPP
#define SORT_DECLS_HPP

#include "common/common.hpp"
#include "tree/cpptree.hpp"
#include "helpers/walk.hpp"
#include "uniquify_identifiers.hpp"

// Does declaration of a depend on (ie use) declaration of b?
// set ignore_ptr_ref_record to make function ignore the case where a's type contains a pointer or 
// reference to b
bool IsDependOn( TreePtr<CPPTree::Declaration> a, TreePtr<CPPTree::Declaration> b, bool ignore_indirection_to_record=false );

// Sort some declarations into an order that will compile successfully.
// Optionally ignore pointers and references to records: they can be cyclic and so cannot be sorted
// on. set to "true" if incomplete declarations for all records have already been emitted. 
Sequence<CPPTree::Declaration> SortDecls( ContainerInterface &c, bool ignore_indirection_to_record, const UniquifyNames::IdentifierNameMap &unique_ids );

// Sort using SimpleCompare
Sequence<CPPTree::Declaration> PreSortDecls( Sequence<CPPTree::Declaration> c, const UniquifyNames::IdentifierNameMap &unique_ids );

// Mix them up randomly!!
Sequence<CPPTree::Declaration> JumbleDecls( Sequence<CPPTree::Declaration> c );

// Reverse them!!
Sequence<CPPTree::Declaration> ReverseDecls( Sequence<CPPTree::Declaration> c );

#endif
