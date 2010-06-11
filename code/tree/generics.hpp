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

typedef OOStd::SharedPtrInterface<Itemiser::Element, Node> SharedPtrInterface;


template<typename ELEMENT>
class SharedPtr : public OOStd::SharedPtr<Itemiser::Element, Node, ELEMENT>
{
public:
	SharedPtr() : OOStd::SharedPtr<Itemiser::Element, Node, ELEMENT>() {}
	SharedPtr( ELEMENT *o ) : OOStd::SharedPtr<Itemiser::Element, Node, ELEMENT>(o) {}
	SharedPtr( const SharedPtrInterface &g ) : OOStd::SharedPtr<Itemiser::Element, Node, ELEMENT>(g) {}
	template< typename OTHER >
	SharedPtr( const shared_ptr<OTHER> &o ) : OOStd::SharedPtr<Itemiser::Element, Node, ELEMENT>(o) {}

	virtual operator string() const
	{
        return Traceable::CPPFilt( typeid( ELEMENT ).name() );
	}
};


// Inferno tree containers
// TODO deprecate use of sshared_ptr everywhere in Inferno, then remove the functions
// here like push_back( const shared_ptr<Node> ), then maybe make SequenceInterface and
// CollectionInterface be typedefs
typedef OOStd::ContainerInterface<Itemiser::Element, SharedPtrInterface> ContainerInterface;

typedef OOStd::PointIterator<Itemiser::Element, SharedPtrInterface> PointIterator;
typedef OOStd::CountingIterator<Itemiser::Element, SharedPtrInterface> CountingIterator;

struct SequenceInterface : virtual OOStd::SequenceInterface< Itemiser::Element, SharedPtrInterface >
{
	virtual void push_back( const shared_ptr<Node> &gx ) = 0;
};

struct CollectionInterface : virtual OOStd::CollectionInterface< Itemiser::Element, SharedPtrInterface >
{
	virtual void insert( const shared_ptr<Node> &gx ) = 0;
	virtual int erase( const shared_ptr<Node> &gx ) = 0;
};

template<typename ELEMENT>
struct Sequence : virtual SequenceInterface, virtual OOStd::Sequence< Itemiser::Element, SharedPtrInterface, deque< SharedPtr<ELEMENT> > >
{
	typedef deque< SharedPtr<ELEMENT> > Impl;

	inline Sequence() {}
	inline Sequence( const SequenceInterface &cns ) :
		OOStd::Sequence< Itemiser::Element, SharedPtrInterface, Impl >( cns ) {}
	inline Sequence( const SharedPtrInterface &nx ) :
	    OOStd::Sequence< Itemiser::Element, SharedPtrInterface, Impl >( nx ) {}

	virtual void push_back( const shared_ptr<Node> &gx )
	{
		Impl::push_back( gx );
	}

	virtual operator string() const
	{
        return CPPFilt( typeid( ELEMENT ).name() );
	}
};


template<typename ELEMENT>
struct Collection : CollectionInterface, OOStd::Collection< Itemiser::Element, SharedPtrInterface, set< SharedPtr<ELEMENT> > >
{
 	typedef set< SharedPtr<ELEMENT> > Impl;

 	inline Collection<ELEMENT>() {}
    inline Collection( const ContainerInterface &cns ) :
    	OOStd::Collection< Itemiser::Element, SharedPtrInterface, Impl >( cns ) {}
    inline Collection( const SharedPtrInterface &nx ) :
    	OOStd::Collection< Itemiser::Element, SharedPtrInterface, Impl >( nx ) {}

    virtual void insert( const shared_ptr<Node> &gx )
	{
		Impl::insert( gx );
	}
	virtual int erase( const shared_ptr<Node> &gx )
	{
		return Impl::erase( gx );
	}
	virtual operator string() const
	{
        return CPPFilt( typeid( ELEMENT ).name() );
	}
};

// Assmebling sequences using operator,

//template<class LELEMENT, class RELEMENT>
inline Sequence<Node> operator,( const SharedPtrInterface &l, const SharedPtrInterface &r )
{
    Sequence<Node> seq;
    seq.push_back( (const SharedPtrInterface &)l );
    seq.push_back( (const SharedPtrInterface &)r );
    return seq;
}

//template<class RELEMENT>
inline Sequence<Node> operator,( const SequenceInterface &l, const SharedPtrInterface &r )
{
    Sequence<Node> seq = l;
    seq.push_back( (const SharedPtrInterface &)r );
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
struct MakeShared : SharedPtr<ELEMENT>
{
	MakeShared() : SharedPtr<ELEMENT>( new ELEMENT ) {}
	template<typename CP0>
	MakeShared(const CP0 &cp0) : SharedPtr<ELEMENT>( new ELEMENT(cp0) ) {}
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
