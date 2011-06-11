/*
 * containers.hpp
 *
 *  Created on: 30 Aug 2009
 *      Author: jgraley
 */

#ifndef CONTAINERS_HPP
#define CONTAINERS_HPP

#include "common.hpp"

namespace OOStd {

//
// Template for a base class for STL containers with forward iterators.
// Supports direct calls and iterators.
//
// The base container will be derived from SUB_BASE. VALUE_INTERFACE should
// be a base (or compatible type) for the elements of all sub-containers.
//

template< class SUB_BASE, typename VALUE_INTERFACE >
class ContainerInterface : public virtual Traceable, public virtual SUB_BASE
{
public:
	// Abstract base class for the implementation-specific iterators in containers.
	struct iterator_interface : public Traceable
	{
		// TODO const iterator and const versions of begin(), end()
		virtual shared_ptr<iterator_interface> Clone() const = 0; // Make another copy of the present iterator
		virtual iterator_interface &operator++() = 0;
		const virtual VALUE_INTERFACE &operator*() const = 0;
		const virtual VALUE_INTERFACE *operator->() const = 0;
		virtual bool operator==( const iterator_interface &ib ) const = 0;
		virtual void Overwrite( const VALUE_INTERFACE *v ) const = 0;
		virtual const bool IsOrdered() const = 0;
		virtual const int GetCount() const { ASSERTFAIL("Only on CountingIterator"); }
	};

public:
	// Wrapper for iterator_interface, uses boost::shared_ptr<> and Clone() to manage the real iterator
	// and forwards all the operations using co-variance where possible. These can be passed around
	// by value, and have copy-on-write semantics, so big iterators will actually get optimised
	// (in your face, Stepanov!)
	class iterator : public Traceable
	{
	public:
		typedef forward_iterator_tag iterator_category;
		typedef VALUE_INTERFACE value_type;
		typedef int difference_type;
		typedef const value_type *pointer;
		typedef const value_type &reference;

		iterator() :
			pib( shared_ptr<iterator_interface>() ) {}

		iterator( const iterator_interface &ib ) :
			pib( ib.Clone() ) {} // Deep copy because from unmanaged source

		iterator( const iterator &i ) :
			pib( i.pib ) {} // Shallow copy

		iterator &operator=( const iterator &i )
		{
			pib = i.pib; // Shallow copy
			return *this;
		}

		iterator &operator++()
		{
			ASSERT(pib)("Attempt to increment uninitialised iterator");
			EnsureUnique();
			pib->operator++();
			return *this;
		}

		const value_type &operator*() const 
		{
			ASSERT(pib)("Attempt to dereference uninitialised iterator");
			return pib->operator*();
		}

		const value_type *operator->() const
		{
			ASSERT(pib)("Attempt to dereference uninitialised iterator");
			return pib->operator->();
		}

		bool operator==( const iterator &i ) const
		{
			ASSERT(pib)("Attempt to compare uninitialised iterator");
			return pib->operator==( *(i.pib) );
		}

		bool operator!=( const iterator &i ) const
		{
			ASSERT(pib)("Attempt to compare uninitialised iterator");
			return !operator==( i );
		}

		void Overwrite( const value_type *v ) const
		{
			ASSERT(pib)("Attempt to Overwrite through uninitialised iterator");
		    pib->Overwrite( v );
		}
				
		const bool IsOrdered() const
		{
			return pib->IsOrdered();
		}

		const int GetCount() const
		{
			return pib->GetCount();
		}

		iterator_interface *GetUnderlyingIterator()
		{
			if( pib )
				return pib.get();
			else
				return NULL;
		}
	private:
		void EnsureUnique()
		{
			// Call this before modifying the underlying iterator - Performs a deep copy
			// if required to make sure there are no other refs.
			if( pib && !pib.unique() )
				pib = pib->Clone();
			ASSERT( !pib || pib.unique() );
		}

		shared_ptr<iterator_interface> pib;
	};
	typedef iterator const_iterator; // TODO const iterators properly

	// These direct calls to the container are designed to support co-variance.
    virtual void insert( const VALUE_INTERFACE &gx ) = 0;
	virtual const iterator_interface &begin() = 0;
    virtual const iterator_interface &end() = 0;
    virtual void erase( typename ContainerInterface<SUB_BASE, VALUE_INTERFACE>::iterator it ) = 0;
    virtual bool empty() const { return size() == 0; }
    virtual int size() const
    {
    	// TODO support const_interator properly and get rid of this const_cast
    	ContainerInterface *nct = const_cast<ContainerInterface *>(this);
    	int n=0;
    	FOREACH( const VALUE_INTERFACE &x, *nct )
    	    n++;
    	return n;
    }
    virtual void clear() = 0;
};


template< class SUB_BASE, typename VALUE_INTERFACE >
struct SequenceInterface : virtual ContainerInterface<SUB_BASE, VALUE_INTERFACE>
{
    virtual VALUE_INTERFACE &operator[]( int i ) = 0;
    virtual void push_back( const VALUE_INTERFACE &gx ) = 0;
};


template< class SUB_BASE, typename VALUE_INTERFACE >
struct SimpleAssociativeContainerInterface : virtual ContainerInterface<SUB_BASE, VALUE_INTERFACE>
{
	virtual int erase( const VALUE_INTERFACE &gx ) = 0;
	virtual bool IsExist( const VALUE_INTERFACE &gx ) = 0;
};


//
// Abstract template for containers that will be use any STL container as
// the actual implementation.
// Params as for ContainerInterface except we now have to fill in CONTAINER_IMPL
// as the stl container class eg std::list<blah>
//
template<class SUB_BASE, typename VALUE_INTERFACE, class CONTAINER_IMPL>
struct ContainerCommon : virtual ContainerInterface<SUB_BASE, VALUE_INTERFACE>, CONTAINER_IMPL
{
	struct iterator : public CONTAINER_IMPL::iterator,
	                  public ContainerInterface<SUB_BASE, VALUE_INTERFACE>::iterator_interface
	{
		virtual iterator &operator++()
		{
			CONTAINER_IMPL::iterator::operator++();
		    return *this;
		}

		virtual const typename CONTAINER_IMPL::value_type &operator*() const
		{
			return CONTAINER_IMPL::iterator::operator*();
		}

		virtual const typename CONTAINER_IMPL::value_type *operator->() const
		{
			return CONTAINER_IMPL::iterator::operator->();
		}

		virtual bool operator==( const typename ContainerInterface<SUB_BASE, VALUE_INTERFACE>::iterator_interface &ib ) const
		{
            // JSG apparently there's no operator== in std::deque::iterator, which is odd since iterators 
            // are supposed to be Equality Comparable. So we just cast the types really carefully and use ==
		    const typename CONTAINER_IMPL::iterator *pi = dynamic_cast<const typename CONTAINER_IMPL::iterator *>(&ib);
		    ASSERT(pi)("Comparing iterators of different type");
			return *(const typename CONTAINER_IMPL::iterator *)this == *pi;
//		    return CONTAINER_IMPL::iterator::operator==( *this, *pi );
		}
	};

	typedef iterator const_iterator;

    virtual void erase( typename ContainerInterface<SUB_BASE, VALUE_INTERFACE>::iterator it )
    {
        iterator *cit = dynamic_cast<iterator *>( it.GetUnderlyingIterator() );
        ASSERT( cit ); // if this fails, you passed erase() the wrong kind of iterator
        CONTAINER_IMPL::erase( *(typename CONTAINER_IMPL::iterator *)cit );
    }
    virtual bool empty() const
    {
        return CONTAINER_IMPL::empty();
    }
    virtual int size() const
    {
        return CONTAINER_IMPL::size();
    }
    virtual void clear()
    {
    	return CONTAINER_IMPL::clear();
    }
	virtual operator string() const
	{
        return Traceable::CPPFilt( typeid( typename CONTAINER_IMPL::value_type ).name() );
	}
};


//
// Template for containers that use ordered STL containers as implementation
// (basically vector, deque etc). Instantiate as per ContainerCommon.
//
template<class SUB_BASE, typename VALUE_INTERFACE, class CONTAINER_IMPL>
struct Sequence : virtual ContainerCommon<SUB_BASE, VALUE_INTERFACE, CONTAINER_IMPL>, virtual SequenceInterface<SUB_BASE, VALUE_INTERFACE>
{
    using typename CONTAINER_IMPL::insert; // due to silly C++ rule where different overloads hide each other
    inline Sequence<SUB_BASE, VALUE_INTERFACE, CONTAINER_IMPL>() {}
	struct iterator : public ContainerCommon<SUB_BASE, VALUE_INTERFACE, CONTAINER_IMPL>::iterator
    {
		inline iterator( typename CONTAINER_IMPL::iterator &i ) : CONTAINER_IMPL::iterator(i) {}
		inline iterator() {}
		virtual typename CONTAINER_IMPL::value_type &operator*() const
		{
			return CONTAINER_IMPL::iterator::operator*();
		}
		virtual typename CONTAINER_IMPL::value_type *operator->() const
		{
			return CONTAINER_IMPL::iterator::operator->();
		}
		virtual shared_ptr<typename ContainerInterface<SUB_BASE, VALUE_INTERFACE>::iterator_interface> Clone() const
		{
			shared_ptr<iterator> ni( new iterator );
			*ni = *this;
			return ni;
		}
    	virtual void Overwrite( const VALUE_INTERFACE *v ) const
		{
		    // JSG Overwrite() just writes through the pointer got from dereferencing the iterator,
		    // because in Sequences (ordererd containers) elements may be modified.
    		typename CONTAINER_IMPL::value_type x( CONTAINER_IMPL::value_type::InferredDynamicCast(*v) );
		    CONTAINER_IMPL::iterator::operator*() = x;
		}
    	virtual const bool IsOrdered() const
    	{
    		return true; // yes, Sequences are ordered
    	}
	};

    virtual typename CONTAINER_IMPL::value_type &operator[]( int i )
    {
    	return CONTAINER_IMPL::operator[](i);
    }
	virtual void insert( const VALUE_INTERFACE &gx )
	{
		push_back( gx );
	}
	virtual void push_back( const VALUE_INTERFACE &gx )
	{
		typename CONTAINER_IMPL::value_type sx( CONTAINER_IMPL::value_type::InferredDynamicCast(gx) );
		CONTAINER_IMPL::push_back( sx );
	}
	template<typename OTHER>
	inline void push_back( const OTHER &gx )
	{
		typename CONTAINER_IMPL::value_type sx(gx);
		CONTAINER_IMPL::push_back( sx );
	}

	// Covariant style only works with refs and pointers, so force begin/end to return refs safely
	// This complies with STL's thread safety model. To quote SGI,
	// "The SGI implementation of STL is thread-safe only in the sense that simultaneous accesses
	// to distinct containers are safe, and simultaneous read accesses to to shared containers are
	// safe. If multiple threads access a single container, and at least one thread may potentially
	// write, then the user is responsible for ensuring mutual exclusion between the threads during
	// the container accesses."
	// So that's OK then.
    iterator my_begin, my_end;

    virtual const iterator &begin()
    {
    	my_begin.CONTAINER_IMPL::iterator::operator=( CONTAINER_IMPL::begin() );
    	return my_begin;
    }
    virtual const iterator &end()
    {
    	my_end.CONTAINER_IMPL::iterator::operator=( CONTAINER_IMPL::end() );
    	return my_end;
    }
	Sequence( const SequenceInterface<SUB_BASE, VALUE_INTERFACE> &cns )
	{
		// TODO support const_interator properly and get rid of this const_cast
		SequenceInterface<SUB_BASE, VALUE_INTERFACE> *ns = const_cast< SequenceInterface<SUB_BASE, VALUE_INTERFACE> * >( &cns );
		for( typename SequenceInterface<SUB_BASE, VALUE_INTERFACE>::iterator i=ns->begin();
		     i != ns->end();
		     ++i )
		{
            CONTAINER_IMPL::push_back( (typename CONTAINER_IMPL::value_type)*i );
		}
	}
	Sequence( const VALUE_INTERFACE &nx )
	{
		typename CONTAINER_IMPL::value_type sx( CONTAINER_IMPL::value_type::InferredDynamicCast(nx) );
		CONTAINER_IMPL::push_back( sx );
	}
	template<typename L, typename R>
	inline Sequence( const pair<L, R> &p )
	{
		*this = Sequence<SUB_BASE, VALUE_INTERFACE, CONTAINER_IMPL>( p.first );
		Sequence<SUB_BASE, VALUE_INTERFACE, CONTAINER_IMPL> t( p.second );

		for( typename Sequence<SUB_BASE, VALUE_INTERFACE, CONTAINER_IMPL>::iterator i=t.begin();
		     i != t.end();
		     ++i )
		{
            CONTAINER_IMPL::push_back( *i );
		}
	}
	inline Sequence( const typename CONTAINER_IMPL::value_type &v )
	{
		push_back( v );
	}
};


//
// Template for containers that use unordered STL containers as implementation
// (basically associative containers). Instantiate as per ContainerCommon.
//
template<class SUB_BASE, typename VALUE_INTERFACE, class CONTAINER_IMPL>
struct SimpleAssociativeContainer : virtual ContainerCommon<SUB_BASE, VALUE_INTERFACE, CONTAINER_IMPL>, virtual SimpleAssociativeContainerInterface<SUB_BASE, VALUE_INTERFACE>
{
    inline SimpleAssociativeContainer<SUB_BASE, VALUE_INTERFACE, CONTAINER_IMPL>() {}
	struct iterator : public ContainerCommon<SUB_BASE, VALUE_INTERFACE, CONTAINER_IMPL>::iterator
    {
		inline iterator( typename CONTAINER_IMPL::iterator &i ) : CONTAINER_IMPL::iterator(i) {}
		inline iterator() {}
		virtual shared_ptr<typename ContainerInterface<SUB_BASE, VALUE_INTERFACE>::iterator_interface> Clone() const
		{
			shared_ptr<iterator> ni( new iterator );
			*ni = *this;
			return ni;
		}
    	virtual void Overwrite( const VALUE_INTERFACE *v ) const
		{
		    // SimpleAssociativeContainers (unordered containers) do not allow elements to be modified
		    // because the internal data structure depends on element values. So we 
		    // erase the old element and insert the new one; thus, Overwrite() should not be assumed O(1)
    		typename CONTAINER_IMPL::value_type s( CONTAINER_IMPL::value_type::InferredDynamicCast(*v) );
    		((CONTAINER_IMPL *)owner)->erase( *this );
		    *(typename CONTAINER_IMPL::iterator *)this = ((CONTAINER_IMPL *)owner)->insert( s ); // become an iterator for the newly inserted element
 		}
    	virtual const bool IsOrdered() const
    	{
    		return false; // no, SimpleAssociativeContainers are not ordered
    	}
        SimpleAssociativeContainer<SUB_BASE, VALUE_INTERFACE, CONTAINER_IMPL> *owner;
	};

	virtual void insert( const VALUE_INTERFACE &gx )
	{
		typename CONTAINER_IMPL::value_type sx( CONTAINER_IMPL::value_type::InferredDynamicCast(gx) );
		CONTAINER_IMPL::insert( sx );
	}
	template<typename OTHER>
	inline void insert( const OTHER &gx )
	{
		typename CONTAINER_IMPL::value_type sx(gx);
		CONTAINER_IMPL::insert( sx );
	}
	virtual int erase( const VALUE_INTERFACE &gx )
	{
		typename CONTAINER_IMPL::value_type sx( CONTAINER_IMPL::value_type::InferredDynamicCast(gx) );
		return CONTAINER_IMPL::erase( sx );
	}
	virtual bool IsExist( const VALUE_INTERFACE &gx )
	{
		typename CONTAINER_IMPL::value_type sx( CONTAINER_IMPL::value_type::InferredDynamicCast(gx) );
		typename CONTAINER_IMPL::iterator it = CONTAINER_IMPL::find( sx );
		return it != CONTAINER_IMPL::end();
	}

	iterator my_begin, my_end;
    virtual const iterator &begin()
    {
    	my_begin.CONTAINER_IMPL::iterator::operator=( CONTAINER_IMPL::begin() );
    	my_begin.owner = this;
    	return my_begin;
    }
    virtual const iterator &end()
    {
    	my_end.CONTAINER_IMPL::iterator::operator=( CONTAINER_IMPL::end() );
    	my_end.owner = this;
    	return my_end;
    }
    SimpleAssociativeContainer( const ContainerInterface<SUB_BASE, VALUE_INTERFACE> &cns )
	{
		// TODO support const_interator properly and get rid of this const_cast
    	ContainerInterface<SUB_BASE, VALUE_INTERFACE> *ns = const_cast< ContainerInterface<SUB_BASE, VALUE_INTERFACE> * >( &cns );
		for( typename ContainerInterface<SUB_BASE, VALUE_INTERFACE>::iterator i=ns->begin();
		     i != ns->end();
		     ++i )
		{
            CONTAINER_IMPL::insert( *i );
		}
	}
    SimpleAssociativeContainer( const VALUE_INTERFACE &nx )
	{
		typename CONTAINER_IMPL::value_type sx( CONTAINER_IMPL::value_type::InferredDynamicCast(nx) );
        CONTAINER_IMPL::insert( sx );
	}
	template<typename L, typename R>
	inline SimpleAssociativeContainer( const pair<L, R> &p )
	{
		*this = SimpleAssociativeContainer<SUB_BASE, VALUE_INTERFACE, CONTAINER_IMPL>( p.first );
		SimpleAssociativeContainer<SUB_BASE, VALUE_INTERFACE, CONTAINER_IMPL> t( p.second );

		for( typename SimpleAssociativeContainer<SUB_BASE, VALUE_INTERFACE, CONTAINER_IMPL>::iterator i=t.begin();
		     i != t.end();
		     ++i )
		{
            CONTAINER_IMPL::insert( *i );
		}
	}
	inline SimpleAssociativeContainer( const typename CONTAINER_IMPL::value_type &v )
	{
		insert( v );
	}
};

//
// Iterator that points to a single object, no container required.
// We do not support looping/incrementing or FOREACH (which requires a
// container) but we do permit compare, deref and Overwrite(). This lets
// ContainerInterface::iterator be used generically even when objects are
// not in containers.
//
template<class SUB_BASE, typename VALUE_INTERFACE>
struct PointIterator : public ContainerInterface<SUB_BASE, VALUE_INTERFACE>::iterator_interface
{
    VALUE_INTERFACE * element;

    PointIterator() :
        element(NULL) // means end-of-range
    {
    }

    PointIterator( const PointIterator &other ) :
        element(other.element)
    {
    }

    PointIterator( VALUE_INTERFACE *i ) :
        element(i)
    {      
        ASSERT(i); // We don't allow NULL as input because it means end-of-range
    }

	virtual shared_ptr<typename ContainerInterface<SUB_BASE, VALUE_INTERFACE>::iterator_interface> Clone() const
	{
		shared_ptr<PointIterator> ni( new PointIterator(*this) );
		return ni;
	}

	virtual PointIterator &operator++()
	{
		element = NULL; // ie if we increment, we get to the end of the range
		return *this;
	}

	virtual VALUE_INTERFACE &operator*() const
	{
	    ASSERT(element)("Tried to dereference NULL PointIterator");
		return *element;
	}

	virtual VALUE_INTERFACE *operator->() const
	{
	    ASSERT(element)("Tried to dereference NULL PointIterator");
		return element;
	}

	virtual bool operator==( const typename ContainerInterface<SUB_BASE, VALUE_INTERFACE>::iterator_interface &ib ) const
	{
		const PointIterator *pi = dynamic_cast<const PointIterator *>(&ib);
		ASSERT(pi)("Comparing point iterator with something else ")(ib);
		return pi->element == element;
	}
	
	virtual void Overwrite( const VALUE_INTERFACE *v ) const
	{
	    *element = *v;
	}

	virtual const bool IsOrdered() const
	{
		return true; // shouldn't matter what we return here
	}
};

template<class SUB_BASE, typename VALUE_INTERFACE>
struct CountingIterator : public ContainerInterface<SUB_BASE, VALUE_INTERFACE>::iterator_interface
{
    int element;

    CountingIterator() :
        element(0)
    {
    }

    CountingIterator( int i ) :
        element(i)
    {
    }

	virtual shared_ptr<typename ContainerInterface<SUB_BASE, VALUE_INTERFACE>::iterator_interface> Clone() const
	{
		shared_ptr<CountingIterator> ni( new CountingIterator(*this) );
		return ni;
	}

	virtual CountingIterator &operator++()
	{
		element++;
		return *this;
	}

	virtual VALUE_INTERFACE &operator*() const
	{
	    ASSERTFAIL("Cannot dereference CountingIterator, use GetCount instead");
	}

	const virtual VALUE_INTERFACE *operator->() const
	{
		ASSERTFAIL("Cannot dereference CountingIterator, use GetCount instead");
	}

	virtual bool operator==( const typename ContainerInterface<SUB_BASE, VALUE_INTERFACE>::iterator_interface &ib ) const
	{
		const CountingIterator *pi = dynamic_cast<const CountingIterator *>(&ib);
		ASSERT(pi)("Comparing counting iterator with something else ")( ib );
		return pi->element == element;
	}

	virtual void Overwrite( const VALUE_INTERFACE *v ) const
	{
	    ASSERTFAIL("Cannot Overwrite through CountingIterator");
	}

	virtual const bool IsOrdered() const
	{
		return true; // counting iterator counts 1, 2, 3, like that, so seems to be ordered
	}

	const int GetCount() const
	{
		return element;
	}
};

// Allow operator, to be used to create pairs. Also handy for maps.
template<typename L, typename R>
inline pair<L,R> operator,( const L &l, const R &r )
{
    return pair<L,R>(l,r);
}

}; // namespace

#endif /* GENERICS_HPP */
