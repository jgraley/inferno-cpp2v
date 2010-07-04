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

// TODO optimise SharedPtr, it seems to be somewhat slower than shared_ptr!!!
typedef OOStd::SharedPtrInterface<Itemiser::Element, Node> TreePtrInterface;

template<typename VALUE_TYPE>
class TreePtr : public OOStd::SharedPtr<Itemiser::Element, Node, VALUE_TYPE>
{
public:
	inline TreePtr() : OOStd::SharedPtr<Itemiser::Element, Node, VALUE_TYPE>() {}
	inline TreePtr( VALUE_TYPE *o ) : OOStd::SharedPtr<Itemiser::Element, Node, VALUE_TYPE>(o) {}
	inline TreePtr( const TreePtrInterface &g ) : OOStd::SharedPtr<Itemiser::Element, Node, VALUE_TYPE>(g) {}
    inline operator TreePtr<Node>() const { return OOStd::SharedPtr<Itemiser::Element, Node, VALUE_TYPE>::operator OOStd::SharedPtr<Itemiser::Element, Node, Node>(); }
	inline TreePtr( const OOStd::SharedPtr<Itemiser::Element, Node, VALUE_TYPE> &g ) : OOStd::SharedPtr<Itemiser::Element, Node, VALUE_TYPE>(g) {}
	template< typename OTHER >
	inline TreePtr( const shared_ptr<OTHER> &o ) : OOStd::SharedPtr<Itemiser::Element, Node, VALUE_TYPE>(o) {}
	template< typename OTHER >
	inline TreePtr( const TreePtr<OTHER> &o ) : OOStd::SharedPtr<Itemiser::Element, Node, VALUE_TYPE>(o) {}
	static inline TreePtr<VALUE_TYPE> DynamicCast( const TreePtrInterface &g )
	{
		return OOStd::SharedPtr<Itemiser::Element, Node, VALUE_TYPE>::DynamicCast(g);
	}
};



// Inferno tree containers
typedef OOStd::ContainerInterface<Itemiser::Element, TreePtrInterface> ContainerInterface;
typedef OOStd::PointIterator<Itemiser::Element, TreePtrInterface> PointIterator;
typedef OOStd::CountingIterator<Itemiser::Element, TreePtrInterface> CountingIterator;
typedef OOStd::SequenceInterface<Itemiser::Element, TreePtrInterface> SequenceInterface;
typedef OOStd::SimpleAssociativeContainerInterface<Itemiser::Element, TreePtrInterface> CollectionInterface;

template<typename VALUE_TYPE>
struct Sequence : virtual OOStd::Sequence< Itemiser::Element, TreePtrInterface, deque< TreePtr<VALUE_TYPE> > >
{
	typedef deque< TreePtr<VALUE_TYPE> > Impl;

	inline Sequence() {}
	template<typename L, typename R>
	inline Sequence( const pair<L, R> &p ) :
		OOStd::Sequence< Itemiser::Element, TreePtrInterface, Impl >( p ) {}
	template< typename OTHER >
	inline Sequence( const TreePtr<OTHER> &v ) :
		OOStd::Sequence< Itemiser::Element, TreePtrInterface, Impl >( v ) {}
};


template<typename VALUE_TYPE>
struct Collection : virtual OOStd::SimpleAssociativeContainer< Itemiser::Element, TreePtrInterface, set< TreePtr<VALUE_TYPE> > >
{
 	typedef set< TreePtr<VALUE_TYPE> > Impl;

 	inline Collection<VALUE_TYPE>() {}
	template<typename L, typename R>
	inline Collection( const pair<L, R> &p ) :
		OOStd::SimpleAssociativeContainer< Itemiser::Element, TreePtrInterface, Impl >( p ) {}
	template< typename OTHER >
	inline Collection( const TreePtr<OTHER> &v ) :
		OOStd::SimpleAssociativeContainer< Itemiser::Element, TreePtrInterface, Impl >( v ) {}
};

// Handy typing saver for creating objects and SharedPtrs to them.
// MakeTreePtr<X> may be constructed in the same way as X, but will then
// masquerade as a SharedPtr<X> where the pointed-to X has been allocated
// using new. Similar to Boost's make_shared<>() except that being an object
// with a constructor, rather than a free function, it may be used in a
// declaration as well as in a function-like way. So both of the following
// are OK:
// existing_shared_ptr = MakeTreePtr<X>(10); // as per Boost: construction of temporary looks like function call
// MakeTreePtr<X> new_shared_ptr(10); // new Inferno form: new_shared_ptr may now be used like a SharedPtr<X>
template<typename VALUE_TYPE>
struct MakeTreePtr : TreePtr<VALUE_TYPE>
{
	MakeTreePtr() : TreePtr<VALUE_TYPE>( new VALUE_TYPE ) {}
	template<typename CP0>
	MakeTreePtr(const CP0 &cp0) : TreePtr<VALUE_TYPE>( new VALUE_TYPE(cp0) ) {}
	// Add more params as needed...
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
