// A deep copy for trees or subtrees. It supports an optional "terminus" feature which allows the deep copy to revert
// to a shallow copy at some position in the subtree. When used, the node within the input tree at which we should
// stop deep-copying is needed, as well as a pointer to the subtree to shallow-copy into that place (so it does not 
// have to be the same one). Omit both pointers if not needed.

#ifndef DUPLICATE_HPP
#define DUPLICATE_HPP

#include "node/specialise_oostd.hpp"
#include "common/trace.hpp"

class Duplicate
{
public:
    // Duplicate a single node (just a thin interface onto the Node Interface's Duplicate method)
    static TreePtr<Node> DuplicateNode( TreePtr<Node> source );
    static TreePtr<Node> DuplicateSubtree( TreePtr<Node> source,
                                           TreePtr<Node> source_terminus = TreePtr<Node>(),
                                           TreePtr<Node> dest_terminus = TreePtr<Node>() );    
};

#endif
