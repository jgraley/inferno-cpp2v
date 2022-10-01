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
    // Orderable could be REPEATABLE which is weak, but we still want
    // to account for the number of equivalent elements, so use multiset.
    // Use a reference to SimpleCompare so derived classes can use it.
    typedef multiset<TreePtr<Node>, const SimpleCompare &> Orderered;

    SimpleCompare( Orderable::OrderProperty order_property = Orderable::STRICT );
    SimpleCompare &operator=(const SimpleCompare &other);
        
    /// Pass in two pointers to nodes, which can point to subtrees. Result is true if they match. O(1) locally.
    virtual Orderable::Result Compare( TreePtr<Node> l, TreePtr<Node> r ) const;
    
    /// Pass in two sequences of pointers to nodes, which can point to subtrees. Result is true if they match. O(n) locally.
    Orderable::Result Compare( SequenceInterface &l, SequenceInterface &r ) const;
    
    /// Pass in two collection of pointers to nodes, which can point to subtrees. Result is true if they match. O(nlogn) locally.
    Orderable::Result Compare( CollectionInterface &l, CollectionInterface &r ) const;

    /// Less operator: for use with set, map etc
    bool operator()( TreePtr<Node> l, TreePtr<Node> r ) const;
    
    /// Make a SimpleCompare-ordered set using the current SC, filled with the elements from the supplied container
    Orderered GetOrdering( ContainerInterface &c ) const;

private:
    Orderable::OrderProperty order_property;
};

#endif
