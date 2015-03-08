#include "node/node.hpp"
#include "common/common.hpp"
#include "common/trace.hpp"

#ifndef SIMPLE_COMPARE_HPP
#define SIMPLE_COMPARE_HPP

/** A subtree comparison algorithm that only operates on 
   program nodes (not S&R patterns) and therefore can 
   avoid O(n!) algorithms. */
class SimpleCompare : public Traceable
{
    public:
    /// Pass in two pointers to nodes, which can point to subtrees. Result is true if they match. O(1) locally.
    bool operator()( TreePtr<Node> x, TreePtr<Node> y );
    /// Pass in two sequences of pointers to nodes, which can point to subtrees. Result is true if they match. O(n) locally.
    bool operator()( SequenceInterface &x, SequenceInterface &y );
    /// Pass in two collection of pointers to nodes, which can point to subtrees. Result is true if they match. O(n^2) locally.
    bool operator()( CollectionInterface &x, CollectionInterface &y );
};

#endif
