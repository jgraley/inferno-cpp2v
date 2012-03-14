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
TreePtr<Node> MakeCheckUncombable( TreePtr<CPPTree::Statement> construct );
};

#endif
