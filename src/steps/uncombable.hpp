/*
 * uncombable.hpp
 *
 *  Created on: 14 Mar 2012
 *      Author: jgraley
 */

#ifndef UNCOMBABLE_HPP
#define UNCOMBABLE_HPP

#include "tree/cpptree.hpp"
#include "common/common.hpp"

namespace Steps {
/// Make a sub-pattern for search that restricts for uncombability
/** Insert in search pattern at the base of a construct, which can be a Statement
    or an Expression, and this step will restrict the search so that it only 
    matches if the construct contains something uncombable, somewhere underneath. */
TreePtr<Node> MakeCheckUncombable( TreePtr<CPPTree::Statement> construct );
};

#endif
