#ifndef SPECIALISE_OOSTD_HPP
#define SPECIALISE_OOSTD_HPP

#include "common/common.hpp"
#include "shared_ptr.hpp"
#include "containers.hpp"
#include <list>
#include <set>
#include <iterator>

#define USE_LIST_FOR_COLLECTION 1

// Inferno tree shared pointers



#if USE_LIST_FOR_COLLECTION
#define COLLECTION_IMPL list
#define COLLECTION_BASE OOStd::Sequence
#define COLLECTION_INTERFACE_BASE OOStd::SequenceInterface
#else
#define COLLECTION_IMPL multiset
#define COLLECTION_BASE OOStd::SimpleAssociativeContainer
#define COLLECTION_INTERFACE_BASE OOStd::SimpleAssociativeContainerInterface
#endif

#define SEQUENCE_IMPL list

// Inferno tree containers
typedef OOStd::ContainerInterface ContainerInterface;
typedef OOStd::PointIterator PointIterator;
typedef OOStd::CountingIterator CountingIterator;
struct SequenceInterface : virtual OOStd::SequenceInterface
{
};
struct CollectionInterface : virtual COLLECTION_INTERFACE_BASE
{
};

template<typename VALUE_TYPE>
struct Sequence : virtual OOStd::Sequence< VALUE_TYPE >,
                  virtual SequenceInterface
{
	inline Sequence() {}
	template<typename L, typename R>
	inline Sequence( const pair<L, R> &p ) :
		OOStd::Sequence< VALUE_TYPE >( p ) {}
	template< typename OTHER >
	inline Sequence( const TreePtr<OTHER> &v ) :
		OOStd::Sequence< VALUE_TYPE >( v ) {}
    Sequence& operator=( std::initializer_list< TreePtr<VALUE_TYPE> > ilv )
    {
        (void)OOStd::Sequence< VALUE_TYPE >::operator=(ilv);
        return *this;
    }
};


template<typename VALUE_TYPE> 
struct Collection : virtual COLLECTION_BASE< VALUE_TYPE >,
                    virtual CollectionInterface
{
 	inline Collection<VALUE_TYPE>() {}
	template<typename L, typename R>
	inline Collection( const pair<L, R> &p ) :
		COLLECTION_BASE< VALUE_TYPE >( p ) {}
	template< typename OTHER >
	inline Collection( const TreePtr<OTHER> &v ) :
		COLLECTION_BASE< VALUE_TYPE >( v ) {}
};


/** Produce a container around an iterator that has already been defined.
    Iterator's constructor should produce an "end" iterator if constructed
    without parameters, otherwise a "begin" iterator. The begin constructors
    parameters should be supplied to the template, and will become the
    container's constructor parameters. Iterators don't have to be 
    const, but I think the container does, so clear() etc are disallowed. */
template< typename Iterator, typename ConsParam1, 
                             typename ConsParam2=int, 
                             typename ConsParam3=int >
class ContainerFromIterator : public ContainerInterface
{
public:
	typedef Iterator iterator; /// So that the standard Container::iterator requirement is met
	ContainerFromIterator( ConsParam1 p1 ) : my_begin( p1 ), my_end() {} /// Constructor for 1 param
    ContainerFromIterator( ConsParam1 p1, 
                           ConsParam2 p2 ) : my_begin( p1, p2 ), my_end() {} /// Constructor for 2 params
    ContainerFromIterator( ConsParam1 p1, 
                           ConsParam2 p2,
                           ConsParam2 p3 ) : my_begin( p1, p2, p3 ), my_end() {} /// Constructor for 3 params
    ContainerFromIterator( ConsParam1 p1, 
                           ConsParam2 p2,
                           ConsParam2 p3,
                           ConsParam2 p4 ) : my_begin( p1, p2, p3, p4 ), my_end() {} /// Constructor for 4 params
    // ... add more as required
    
	virtual const iterator &begin() { return my_begin; }
    virtual const iterator &end()   { return my_end; }
    virtual void erase( const ContainerInterface::iterator_interface & ) { ASSERTFAIL("Cannot modify ContainerFromIterator<>"); }
    virtual void clear()                               { ASSERTFAIL("Cannot modify ContainerFromIterator<>"); }    
    virtual void insert( const TreePtrInterface & )    { ASSERTFAIL("Cannot modify ContainerFromIterator<>"); }
private:
    iterator my_begin, my_end;
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
	template<typename CP0, typename CP1>
	MakeTreePtr(const CP0 &cp0, const CP1 &cp1) : TreePtr<VALUE_TYPE>( new VALUE_TYPE(cp0, cp1) ) {}
	template<typename CP0, typename CP1, typename CP2>
	MakeTreePtr(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2) : TreePtr<VALUE_TYPE>( new VALUE_TYPE(cp0, cp1, cp2) ) {}
	// Add more params as needed...
};

template<>
struct MakeTreePtr<Node> : TreePtr<Node>
{
	MakeTreePtr() : TreePtr<Node>( new Node ) {}
	template<typename CP0>
	MakeTreePtr(const CP0 &cp0) : TreePtr<Node>( new Node(cp0) ) {}
	template<typename CP0, typename CP1>
	MakeTreePtr(const CP0 &cp0, const CP1 &cp1) : TreePtr<Node>( new Node(cp0, cp1) ) {}
	template<typename CP0, typename CP1, typename CP2>
	MakeTreePtr(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2) : TreePtr<Node>( new Node(cp0, cp1, cp2) ) {}
	// Add more params as needed...
};

#endif

