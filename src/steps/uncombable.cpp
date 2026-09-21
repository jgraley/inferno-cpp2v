/*
 * uncombable.cpp
 *
 *  Created on: 14 Mar 2012
 *      Author: jgraley
 */
 
#include "steps/uncombable.hpp"
#include "tree/cpptree.hpp"
#include "common/common.hpp"
#include "pattern_helpers.hpp"

using namespace CPPTree;
using namespace Steps;
using namespace VN;
 
TreePtr<Node> Steps::MakeCheckUncombable( TreePtr<CPPTree::StmtDecl> construct )
{
    auto all = MakePatternNode<ConjunctionAgent, Node>();
    auto stuff = MakePatternNode<StuffAgent, Node>();
    auto uncombable = MakePatternNode<Uncombable>();
    
    all->conjuncts = (construct, stuff);
    stuff->terminus = uncombable;
    //TODO maybe restrict recursion to StmtDecl, in case of inline functions, classes etc (though
    // we may never support these, or else lower them out in advance)
    
    return all;
}
