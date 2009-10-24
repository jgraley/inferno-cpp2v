/*
 * containers.hpp
 *
 *  Created on: 30 Aug 2009
 *      Author: jgraley
 */

#ifndef CONTAINERS_HPP
#define CONTAINERS_HPP

#include "common.hpp"

//
// Template for a base class for STL containers with forward iterators.
// Supports direct calls and iterators.
//
// The base container will be derived from SUB_BASE. VALUE_TYPE should
// be a base (or compatible type) for the elements of all sub-containers.
//
template< class SUB_BASE, typename VALUE_TYPE >
class STLContainerBase : public virtual SUB_BASE
{
public:
	// Abstract base class for the iterators in sub-containers. This is just to get
	// virtual calls - this is not the generic iterator.
	struct iterator_base
	{
		// TODO const iterator and const versions of begin(), end()
		virtual shared_ptr<iterator_base> Clone() const = 0; // Make another copy of the present iterator
		virtual iterator_base &operator++() = 0;
		const virtual VALUE_TYPE &operator*() const = 0; 
		const virtual VALUE_TYPE *operator->() const = 0; 
		virtual bool operator==( const iterator_base &ib ) const = 0;
		virtual void Overwrite( const VALUE_TYPE *v ) const = 0;
	};

public:
	// Generic iterator, uses boost::shared_ptr<> and Clone() to manage the real iterator
	// and forwards all the operations using co-variance where possible.
	class iterator
	{
	public:
		typedef forward_iterator_tag iterator_category;
		typedef VALUE_TYPE value_type;
		typedef int difference_type;
		typedef value_type *pointer;
		typedef value_type &reference;

		iterator() :
			pib( shared_ptr<iterator_base>() ) {}

		iterator( const iterator_base &ib ) :
			pib( ib.Clone() ) {}

		iterator( const iterator &i ) :
			pib( i.pib ? i.pib->Clone() : shared_ptr<iterator_base>() ) {} // Only clone if the other iterator has been initialised

		iterator &operator++()
		{
			ASSERT(pib)("Attempt to increment uninitialised iterator");
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

		void Overwrite( const VALUE_TYPE *v ) const
		{
			ASSERT(pib)("Attempt to Overwrite through uninitialised iterator");
		    pib->Overwrite( v );
		}
				
		shared_ptr<iterator_base> pib;
	};
	typedef iterator const_iterator; // TODO const iterators properly

	// These direct calls to the container are designed to support co-variance.
	virtual const iterator_base &begin() = 0;
    virtual const iterator_base &end() = 0;
    virtual void erase( typename STLContainerBase<SUB_BASE, VALUE_TYPE>::iterator it ) = 0;
    virtual bool empty() const = 0;
    virtual int size() const = 0;
    virtual void clear() = 0;
};


//
// Abstract template for containers that will be use any STL container as
// the actual implementation.
// Params as for ContainerBase except we now have to fill in CONTAINER_IMPL
// as the stl container class eg std::list<blah>
//
template<class SUB_BASE, typename VALUE_TYPE, class CONTAINER_IMPL>
struct STLContainer : virtual STLContainerBase<SUB_BASE, VALUE_TYPE>, CONTAINER_IMPL
{
	struct iterator : public CONTAINER_IMPL::iterator,
	                  public STLContainerBase<SUB_BASE, VALUE_TYPE>::iterator_base
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

		virtual bool operator==( const typename STLContainerBase<SUB_BASE, VALUE_TYPE>::iterator_base &ib ) const
		{
            // JSG apparently there's no operator== in std::deque::iterator, which is odd since iterators 
            // are supposed to be Equality Comparable. So we just cast the types really carefully and use ==
		    const typename CONTAINER_IMPL::iterator *pi;
			if( pi = dynamic_cast<const typename CONTAINER_IMPL::iterator *>(&ib) )
				return *(const typename CONTAINER_IMPL::iterator *)this == *pi; 
			else
				return false; // comparing iterators of different types; must be from different containers
		}
	};

	typedef iterator const_iterator;

    virtual void erase( typename STLContainerBase<SUB_BASE, VALUE_TYPE>::iterator it )
    {
        iterator *cit = dynamic_cast<iterator *>( it.pib.get() );
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
};

//
// Template for containers that use ordered STL containers as implementation
// (basically vector, deque etc). Instantiate as per STLContainer.
//
template<class SUB_BASE, typename VALUE_TYPE, class CONTAINER_IMPL>
struct STLSequence : virtual STLContainer<SUB_BASE, VALUE_TYPE, CONTAINER_IMPL>
{
	struct iterator : public STLContainer<SUB_BASE, VALUE_TYPE, CONTAINER_IMPL>::iterator
    {
		virtual shared_ptr<typename STLContainerBase<SUB_BASE, VALUE_TYPE>::iterator_base> Clone() const
		{
			shared_ptr<iterator> ni( new iterator );
			*ni = *this;
			return ni;
		}
    	virtual void Overwrite( const VALUE_TYPE *v ) const
		{
		    // JSG Overwrite() just writes through the pointer got from dereferencing the iterator,
		    // because in Sequences (ordererd containers) elements may be modified.
		    CONTAINER_IMPL::iterator::operator*() = *v;
		}
	};

    // Covarient style only works with refs and pointers, so force begin/end to return refs safely
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
};

//
// Template for containers that use unordered STL containers as implementation
// (basically associative containers). Instantiate as per STLContainer.
//
template<class SUB_BASE, typename VALUE_TYPE, class CONTAINER_IMPL>
struct STLCollection : virtual STLContainer<SUB_BASE, VALUE_TYPE, CONTAINER_IMPL>
{
	struct iterator : public STLContainer<SUB_BASE, VALUE_TYPE, CONTAINER_IMPL>::iterator
    {
		virtual shared_ptr<typename STLContainerBase<SUB_BASE, VALUE_TYPE>::iterator_base> Clone() const
		{
			shared_ptr<iterator> ni( new iterator );
			*ni = *this;
			return ni;
		}
    	virtual void Overwrite( const VALUE_TYPE *v ) const
		{
		    // Collections (unordered containers) do not allow elements to be modified
		    // because the internal data structure depends on element values. So we 
		    // erase the old element and insert the new one; thus, Overwrite() should not be assumed O(1)
		    ((CONTAINER_IMPL *)owner)->erase( *this );
		    pair<typename CONTAINER_IMPL::iterator, bool> result = ((CONTAINER_IMPL *)owner)->insert( *v );
		    ASSERT( result.second ); // insert must succeed (see SGI docs)
		    *(typename CONTAINER_IMPL::iterator *)this = result.first; // become an iterator for the newly inserted element
		}
        STLCollection<SUB_BASE, VALUE_TYPE, CONTAINER_IMPL> *owner;
	};

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
};

//
// Iterator that points to a single object, no container required.
// We do not support looping/incrementing or FOREACH (which requires a
// container) but we do permit compare, deref and Overwrite(). This lets
// STLContainerBase::iterator be used generically even when objects are
// not in containers.
//
template<class SUB_BASE, typename VALUE_TYPE>
struct PointIterator : public STLContainerBase<SUB_BASE, VALUE_TYPE>::iterator_base
{
    VALUE_TYPE * element;

    PointIterator() :
        element(NULL) // means end-of-range
    {
    }

    PointIterator( VALUE_TYPE *i ) :
        element(i)
    {      
        ASSERT(i); // We don't allow NULL as input because it means end-of-range
    }

    PointIterator( VALUE_TYPE &i ) :
        element(&i)
    {
    }

	virtual shared_ptr<typename STLContainerBase<SUB_BASE, VALUE_TYPE>::iterator_base> Clone() const
	{
		shared_ptr<PointIterator> ni( new PointIterator(*this) );
		return ni;
	}

	virtual PointIterator &operator++()
	{
		element = NULL; // ie if we increment, we get to the end of the range
		return *this;
	}

	virtual VALUE_TYPE &operator*() const
	{
	    ASSERT(element)("Tried to dereference NULL PointIterator");
		return *element;
	}

	virtual VALUE_TYPE *operator->() const
	{
	    ASSERT(element)("Tried to dereference NULL PointIterator");
		return element;
	}

	virtual bool operator==( const typename STLContainerBase<SUB_BASE, VALUE_TYPE>::iterator_base &ib ) const
	{
		if( const PointIterator *pi = dynamic_cast<const PointIterator *>(&ib) )
			return pi->element == element;
		else
			return false; // comparing iterators of different types; must be from different containers
	}
	
	virtual void Overwrite( const VALUE_TYPE *v ) const
	{
	    *element = *v;
	}
};

#endif /* GENERICS_HPP */
