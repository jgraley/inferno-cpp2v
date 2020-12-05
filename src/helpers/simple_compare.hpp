#include "node/specialise_oostd.hpp"
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
    // Ordering could be REPEATABLE which is weak, but we still want
    // to account for the number of equivalent elements, so use multiset
    typedef multiset<TreePtr<Node>, SimpleCompare> Ordered;

    SimpleCompare( Matcher::Ordering ordering = Matcher::UNIQUE );
    
    /// Pass in two pointers to nodes, which can point to subtrees. Result is true if they match. O(1) locally.
    CompareResult Compare( TreePtr<Node> x, TreePtr<Node> y );
    
    /// Pass in two sequences of pointers to nodes, which can point to subtrees. Result is true if they match. O(n) locally.
    CompareResult Compare( SequenceInterface &x, SequenceInterface &y );
    
    /// Pass in two collection of pointers to nodes, which can point to subtrees. Result is true if they match. O(nlogn) locally.
    CompareResult Compare( CollectionInterface &x, CollectionInterface &y );

    /// Less operator: for use with set, map etc
    bool operator()( TreePtr<Node> x, TreePtr<Node> y );
    
    /// Make a SimpleCompare-ordered set using the current SC, filled with the elements from the supplied container
    Ordered GetOrdered( ContainerInterface &c );

private:
    const Matcher::Ordering ordering;
};

#endif
