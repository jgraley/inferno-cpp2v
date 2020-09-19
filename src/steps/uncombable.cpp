/*
 * uncombable.cpp
 *
 *  Created on: 14 Mar 2012
 *      Author: jgraley
 */
 
#include "steps/uncombable.hpp"
#include "tree/cpptree.hpp"
#include "common/common.hpp"
#include "sr/agents/all.hpp"

using namespace CPPTree;
using namespace Steps;
using namespace SR;
 
TreePtr<Node> Steps::MakeCheckUncombable( TreePtr<CPPTree::Statement> construct )
{
    MakePatternPtr< MatchAll<Node> > all;
    MakePatternPtr< Stuff<Node> > stuff;
    MakePatternPtr<Uncombable> uncombable;
    
    all->patterns = (construct, stuff);
    stuff->terminus = uncombable;
    //TODO maybe restrict recursion to Statement, in case of inline functions, classes etc (though
    // we may never support these, or else lower them out in advance)
    
    return all;
}
