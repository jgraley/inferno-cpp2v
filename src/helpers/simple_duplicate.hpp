// A deep copy for trees or subtrees. FreeTerminus support removed because not precise 
// enough to find the right link (and unused)

#ifndef SIMPLE_DUPLICATE_HPP
#define SIMPLE_DUPLICATE_HPP

#include "node/specialise_oostd.hpp"
#include "common/trace.hpp"

class SimpleDuplicate
{
public:
    // Duplicate a single node (just a thin interface onto the Node Interface's Duplicate method)
    static TreePtr<Node> DuplicateNode( TreePtr<Node> source );
    static TreePtr<Node> DuplicateSubtree( TreePtr<Node> source );    
};

#endif
