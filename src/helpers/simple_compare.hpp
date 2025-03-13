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
    typedef multiset<reference_wrapper<Node>, const SimpleCompare &> NodeOrdering;

    SimpleCompare( Orderable::OrderProperty order_property = Orderable::TOTAL );
    SimpleCompare &operator=(const SimpleCompare &other);

    /// Less operator: for use with set, map etc
    bool operator()( const Node &l, const Node &r ) const;    
        
    /// Pass in two nodes, which can have subtrees. Result is 0 if they match. O(1) locally.
    virtual Orderable::Diff Compare3Way( const Node &l, const Node &r ) const;

    /// Pass in two sequences of pointers to nodes, which can point to subtrees. Result is true if they match. O(n) locally.
    Orderable::Diff Compare3Way( SequenceInterface &l, SequenceInterface &r ) const;
    
    /// Pass in two collection of pointers to nodes, which can point to subtrees. Result is true if they match. O(nlogn) locally.
    Orderable::Diff Compare3Way( CollectionInterface &l, CollectionInterface &r ) const;

    /// Make a SimpleCompare-ordered set using the current SC, filled with the elements from the supplied container
    NodeOrdering GetNodeOrdering( ContainerInterface &c ) const;

    // ---------------- Legacy interface used by renderer only --------------------
    typedef multiset<TreePtr<Node>, const SimpleCompare &> TreePtrOrdering;

    /// Less operator: for use with set, map etc (legacy)
    bool operator()( TreePtr<Node> l, TreePtr<Node> r ) const;    
        
    /// Pass in two pointers to nodes, which can point to subtrees. Result is 0 if they match. O(1) locally. (legacy)
    virtual Orderable::Diff Compare3Way( TreePtr<Node> l, TreePtr<Node> r ) const;
    
    /// Make a SimpleCompare-ordered set using the current SC, filled with the elements from the supplied container (legacy)
    TreePtrOrdering GetTreePtrOrdering( ContainerInterface &c ) const;

private:
    Orderable::OrderProperty order_property;
};

#endif
