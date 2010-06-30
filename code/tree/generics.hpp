#ifndef TREE_GENERICS_HPP
#define TREE_GENERICS_HPP

#include "common/common.hpp"
#include "common/shared_ptr.hpp"
#include "common/containers.hpp"
#include <deque>
#include <set>
#include <iterator>
#include "itemise.hpp"
#include "type_info.hpp"
#include "clone.hpp"
#include "common/magic.hpp"
#include "match.hpp"


// Inferno tree shared pointers

struct Node;

// TODO SharedPtr -> TreePtr since we only use it for tree nodes
// TODO optimise SharedPtr, it seems to be somewhat slower than shared_ptr!!!
typedef OOStd::SharedPtrInterface<Itemiser::Element, Node> TreePtrInterface;

template<typename ELEMENT>
class TreePtr : public OOStd::SharedPtr<Itemiser::Element, Node, ELEMENT>
{
public:
	inline TreePtr() : OOStd::SharedPtr<Itemiser::Element, Node, ELEMENT>() {}
	inline TreePtr( ELEMENT *o ) : OOStd::SharedPtr<Itemiser::Element, Node, ELEMENT>(o) {}
	inline TreePtr( const TreePtrInterface &g ) : OOStd::SharedPtr<Itemiser::Element, Node, ELEMENT>(g) {}
    inline TreePtr( const OOStd::SharedPtr<Itemiser::Element, Node, ELEMENT> &g ) : OOStd::SharedPtr<Itemiser::Element, Node, ELEMENT>(g) {}
	template< typename OTHER >
	inline TreePtr( const shared_ptr<OTHER> &o ) : OOStd::SharedPtr<Itemiser::Element, Node, ELEMENT>(o) {}
};

template<typename ELEMENT>
inline TreePtr<ELEMENT> DynamicTreePtrCast( const TreePtrInterface &g )
{
	return OOStd::DynamicPointerCast<Itemiser::Element, Node, ELEMENT>(g);
}


// Inferno tree containers
typedef OOStd::ContainerInterface<Itemiser::Element, TreePtrInterface> ContainerInterface;
typedef OOStd::PointIterator<Itemiser::Element, TreePtrInterface> PointIterator;
typedef OOStd::CountingIterator<Itemiser::Element, TreePtrInterface> CountingIterator;
typedef OOStd::SequenceInterface<Itemiser::Element, TreePtrInterface> SequenceInterface;
typedef OOStd::CollectionInterface<Itemiser::Element, TreePtrInterface> CollectionInterface;

template<typename ELEMENT>
struct Sequence : virtual OOStd::Sequence< Itemiser::Element, TreePtrInterface, deque< TreePtr<ELEMENT> > >
{
	typedef deque< TreePtr<ELEMENT> > Impl;

	inline Sequence() {}
	inline Sequence( const SequenceInterface &cns ) :
		OOStd::Sequence< Itemiser::Element, TreePtrInterface, Impl >( cns ) {}
	inline Sequence( const TreePtrInterface &nx ) :
	    OOStd::Sequence< Itemiser::Element, TreePtrInterface, Impl >( nx ) {}
};


template<typename ELEMENT>
struct Collection : virtual OOStd::Collection< Itemiser::Element, TreePtrInterface, set< TreePtr<ELEMENT> > >
{
 	typedef set< TreePtr<ELEMENT> > Impl;

 	inline Collection<ELEMENT>() {}
    inline Collection( const ContainerInterface &cns ) :
    	OOStd::Collection< Itemiser::Element, TreePtrInterface, Impl >( cns ) {}
    inline Collection( const TreePtrInterface &nx ) :
    	OOStd::Collection< Itemiser::Element, TreePtrInterface, Impl >( nx ) {}
};

// Assmebling sequences using operator,

inline Sequence<Node> operator,( const TreePtrInterface &l, const TreePtrInterface &r )
{
    Sequence<Node> seq;
    seq.push_back( l );
    seq.push_back( r );
    return seq;
}

inline Sequence<Node> operator,( const SequenceInterface &l, const TreePtrInterface &r )
{
    Sequence<Node> seq(l);
    seq.push_back( r );
    return seq;
}

// Handy typing saver for creating objects and SharedPtrs to them.
// MakeShared<X> may be constructed in the same way as X, but will then
// masquerade as a SharedPtr<X> where the pointed-to X has been allocated
// using new. Similar to Boost's make_shared<>() except that being an object
// with a constructor, rather than a free function, it may be used in a
// declaration as well as in a function-like way. So both of the following
// are OK:
// existing_shared_ptr = MakeShared<X>(10); // as per Boost: construction of temporary looks like function call
// MakeShared<X> new_shared_ptr(10); // new Inferno form: new_shared_ptr may now be used like a SharedPtr<X>
template<typename ELEMENT>
struct MakeShared : TreePtr<ELEMENT>
{
	MakeShared() : TreePtr<ELEMENT>( new ELEMENT ) {}
	template<typename CP0>
	MakeShared(const CP0 &cp0) : TreePtr<ELEMENT>( new ELEMENT(cp0) ) {}
	// Add more params as needed...
};

// TODO obsolete? try deleting
template<typename ELEMENT>
struct shared_new : TreePtr<ELEMENT>
{
	shared_new() : TreePtr<ELEMENT>( new ELEMENT )
	{
	}
};

//////////////////////////// Node Model ////////////////////////////

#define FINAL_FUNCTION(F) virtual bool IsFinal() { return (F); }

// Mix together the bounce classes for the benefit of the tree
// TODO figure out how to enforce finality in NODE_FUNCTIONS_FINAL
#define NODE_FUNCTIONS ITEMISE_FUNCTION MATCHER_FUNCTION CLONE_FUNCTION FINAL_FUNCTION(false)
#define NODE_FUNCTIONS_FINAL ITEMISE_FUNCTION MATCHER_FUNCTION CLONE_FUNCTION FINAL_FUNCTION(true)
struct NodeBases : Magic,
                   Traceable,
                   Matcher,
                   Itemiser,
                   Cloner
{
};

// Base class for all tree nodes and nodes in search/replace
// patterns etc. Convention is to use "struct" for derived
// node classes so that everything is public (inferno tree nodes
// are more like records in a database, they have only minimal
// functionality). Also, all derived structs should contain the
// NODE_FUNCTIONS macro which expands to a few virtual functions
// required for common ("bounced") functionality. Where multiple
// inheritance diamonds arise, Node should be derived virtually
// (we always want the set-restricting model of inheritance in
// the inferno tree node hierarchy).
struct Node : NodeBases
{
    NODE_FUNCTIONS

    virtual ~Node(){}  // be a virtual hierarchy
    // Node must be inherited virtually, to allow MI diamonds
    // without making Node ambiguous
};

extern void GenericsTest();

#endif
