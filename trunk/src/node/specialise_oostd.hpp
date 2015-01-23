#ifndef SPECIALISE_OOSTD_HPP
#define SPECIALISE_OOSTD_HPP

#include "common/common.hpp"
#include "common/shared_ptr.hpp"
#include "common/containers.hpp"
#include "itemise.hpp"
#include <deque>
#include <set>
#include <iterator>
#include <algorithm>

//#define USE_ORDERING_FOR_COLLECTION_IMPL

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
	virtual OOStd::SharedPtr<Itemiser::Element, Node, Node> MakeValueArchitype() const
    {
        return new VALUE_TYPE; // means VALUE_TYPE must be constructable
    }
    int Cmp( const Itemiser::Element &eo ) const; // implements Itemiser::Element::Cmp() 
#ifdef USE_ORDERING_FOR_COLLECTION_IMPL
    bool operator<( const TreePtr<VALUE_TYPE> &o ) const // needed for std::multiset to reproduce the ordering
    {
        return Cmp(o) < 0;
    }
#endif    
};

// Inferno tree containers
typedef OOStd::ContainerInterface<Itemiser::Element, TreePtrInterface> ContainerInterface;
typedef OOStd::PointIterator<Itemiser::Element, TreePtrInterface> PointIterator;
typedef OOStd::CountingIterator<Itemiser::Element, TreePtrInterface> CountingIterator;
typedef OOStd::SequenceInterface<Itemiser::Element, TreePtrInterface> SequenceInterface;
typedef OOStd::SimpleAssociativeContainerInterface<Itemiser::Element, TreePtrInterface> CollectionInterface;

template<typename VALUE_TYPE, class CONTAINER_IMPL>
struct Container : virtual OOStd::ContainerCommon< Itemiser::Element, TreePtrInterface, CONTAINER_IMPL >
{
    inline Container() {}
    int Cmp( const Itemiser::Element &eo ) const; // implements Itemiser::Element::Cmp()     
};

template<typename VALUE_TYPE>
struct Sequence : virtual Container< VALUE_TYPE, deque< TreePtr<VALUE_TYPE> > >,
                  virtual OOStd::Sequence< Itemiser::Element, TreePtrInterface, deque< TreePtr<VALUE_TYPE> > >
{
    // TODO do these belong here, or in containers.hpp?
    using OOStd::Sequence< Itemiser::Element, TreePtrInterface, deque< TreePtr<VALUE_TYPE> > >::insert;
    using OOStd::Sequence< Itemiser::Element, TreePtrInterface, deque< TreePtr<VALUE_TYPE> > >::begin;
    using OOStd::Sequence< Itemiser::Element, TreePtrInterface, deque< TreePtr<VALUE_TYPE> > >::end;
    
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
struct Collection : virtual Container< VALUE_TYPE, multiset< TreePtr<VALUE_TYPE> > >,
                    virtual OOStd::SimpleAssociativeContainer< Itemiser::Element, TreePtrInterface, multiset< TreePtr<VALUE_TYPE> > >
{
    // TODO do these belong here, or in containers.hpp?
    using OOStd::SimpleAssociativeContainer< Itemiser::Element, TreePtrInterface, multiset< TreePtr<VALUE_TYPE> > >::insert;
    using OOStd::SimpleAssociativeContainer< Itemiser::Element, TreePtrInterface, multiset< TreePtr<VALUE_TYPE> > >::begin;
    using OOStd::SimpleAssociativeContainer< Itemiser::Element, TreePtrInterface, multiset< TreePtr<VALUE_TYPE> > >::end;
 	typedef multiset< TreePtr<VALUE_TYPE> > Impl;

 	inline Collection<VALUE_TYPE>() {}
	template<typename L, typename R>
	inline Collection( const pair<L, R> &p ) :
		OOStd::SimpleAssociativeContainer< Itemiser::Element, TreePtrInterface, Impl >( p ) {}
	template< typename OTHER >
	inline Collection( const TreePtr<OTHER> &v ) :
		OOStd::SimpleAssociativeContainer< Itemiser::Element, TreePtrInterface, Impl >( v ) {}
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
    // ... add more as required
    
	virtual const iterator &begin() { return my_begin; }
    virtual const iterator &end()   { return my_end; }
    virtual void erase( ContainerInterface::iterator ) { ASSERTFAIL("Cannot modify ContainerFromIterator<>"); }
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


template<typename VALUE_TYPE> int TreePtr<VALUE_TYPE>::Cmp( const Itemiser::Element &eo ) const
{
    INDENT;
    const TreePtr<VALUE_TYPE> *po = dynamic_cast< const TreePtr<VALUE_TYPE>* >(&eo);
    ASSERT( po )("Cmp() called with mismatched types");
    const TreePtr<VALUE_TYPE> &o = *po; // get a ref for convenience
 
    // Null-ness of the pointers takes first priority
    bool this_not_null = !!*this;
    bool o_not_null = !!o;
    TRACE(*this)(" < ")(o)("\n");
    if( !(this_not_null && o_not_null) )
    {
        int nc = (int)this_not_null - (int)o_not_null; 
        TRACE("is %d (nullness)\n", nc);
        return nc;
    }
    
    {
        INDENT;
        // Identity of the node itself takes second priority
        int lc = (*this)->GetLocalCmp(*o);
        TRACE(**this)(" < ")(*o)("\n");
        if( lc != 0 )
        {
            TRACE("is %d (node type)\n", lc);
            return lc;
        } 
        
        // Itemise them both - sizes should match!
        vector< Itemiser::Element * > t_memb = (*this)->Itemise();
        vector< Itemiser::Element * > o_memb = o->Itemise();
        ASSERT( t_memb.size() == o_memb.size() ); // since we should have excluded differing node types above
        
        for( int i=0; i<t_memb.size(); i++ )
        {
            ASSERT( t_memb[i] )( "itemise returned null element");
            ASSERT( o_memb[i] )( "itemise returned null element");

            int pc = t_memb[i]->Cmp( *o_memb[i] );
            if( pc != 0 )
            {
                TRACE("is %d (member %d)\n", pc, i);
                return pc;
            } 
        }
    }

    // survived to the end? then we have a match.
    TRACE("is 0\n");
    return 0;
}


template<typename VALUE_TYPE, class CONTAINER_IMPL> int Container<VALUE_TYPE, CONTAINER_IMPL>::Cmp( const Itemiser::Element &eo ) const
{
    INDENT;
    const Container<VALUE_TYPE, CONTAINER_IMPL> *po = dynamic_cast< const Container<VALUE_TYPE, CONTAINER_IMPL> *>(&eo);
    ASSERT( po )("Cmp() called with mismatched types");
    
    TRACE("Container comparison\n");
    
    // Sorry!
    Container<VALUE_TYPE, CONTAINER_IMPL> &o = const_cast< Container<VALUE_TYPE, CONTAINER_IMPL> &>(*po); // get a ref for convenience
    Container<VALUE_TYPE, CONTAINER_IMPL> &t = const_cast< Container<VALUE_TYPE, CONTAINER_IMPL> &>(*this); // get a ref for convenience
    
    typename Container<VALUE_TYPE, CONTAINER_IMPL>::iterator tit, oit;
    
    // Check each element in turn. And-rule in condition means we stop at the end of the smallest.
    int i;
    for( tit = t.begin(), oit = o.begin(), i=0; tit != t.end() && oit != o.end(); ++tit, ++oit, ++i )
    {
        int pc = tit->Cmp( *oit );
        if( pc != 0 )
        {
            TRACE("is %d (element %d)\n", pc, i);
            return pc;
        } 
    }

    int lc=0;
    if( tit == t.end() )           
        lc = -1; // this was shorter, so less
    else if( oit == o.end() )
        lc = 1; // other was shorter, so greater
    else
        lc = 0; // length equal, all elements checked, must be the same.

    if( lc != 0 )
        TRACE("is %d (length)\n", lc);
    else
        TRACE("is 0\n");

    return lc;       
}

#endif

