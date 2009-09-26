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
		virtual VALUE_TYPE &operator*() = 0; 
		virtual VALUE_TYPE *operator->() = 0; 
		virtual bool operator==( const iterator_base &ib ) = 0;
		virtual void Overwrite( VALUE_TYPE &v ) = 0;
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
			pib( i.pib->Clone() ) {}

		iterator &operator++()
		{
			pib->operator++();
			return *this;
		}

		value_type &operator*()
		{
			return pib->operator*();
		}

		value_type *operator->()
		{
			return pib->operator->();
		}

		bool operator==( const iterator &i )
		{
			return pib->operator==( *(i.pib) );
		}

		bool operator!=( const iterator &i )
		{
			return !operator==( i );
		}

		void Overwrite( VALUE_TYPE &v )
		{
		    pib->Overwrite( v );
		}
		void Overwrite( VALUE_TYPE *v )
		{
		    pib->Overwrite( *v );
		}
				
	private:
		shared_ptr<iterator_base> pib;
	};
	typedef iterator const_iterator; // TODO const iterators properly

	// These direct calls to the container are designed to support co-variance.
	virtual const iterator_base &begin() = 0;
    virtual const iterator_base &end() = 0;
    virtual int size() const = 0;
    virtual void clear() = 0;
};


//
// Template for containers that derive from ContainerBase.
// Params as for ContainerBase except we now have to fill in STLCONTAINER
// as the stl container class eg std::list<blah>
//
template<class SUB_BASE, typename VALUE_TYPE, class STLCONTAINER>
struct Container : virtual STLContainerBase<SUB_BASE, VALUE_TYPE>, STLCONTAINER
{
	struct iterator : public STLCONTAINER::iterator,
	                  public STLContainerBase<SUB_BASE, VALUE_TYPE>::iterator_base
	{
		virtual shared_ptr<typename STLContainerBase<SUB_BASE, VALUE_TYPE>::iterator_base> Clone() const
		{
			shared_ptr<iterator> ni( new iterator );
			*ni = *this;
			return ni;
		}

		virtual iterator &operator++()
		{
			STLCONTAINER::iterator::operator++();
		    return *this;
		}

		virtual typename STLCONTAINER::value_type &operator*()
		{
			return STLCONTAINER::iterator::operator*();
		}

		virtual typename STLCONTAINER::value_type *operator->()
		{
			return STLCONTAINER::iterator::operator->();
		}

		virtual bool operator==( const typename STLContainerBase<SUB_BASE, VALUE_TYPE>::iterator_base &ib )
		{
			if( const iterator *pi = dynamic_cast<const iterator *>(&ib) )
				return *pi == *this;
			else
				return false; // comparing iterators of different types; must be from different containers
		}
		
		virtual void Overwrite( VALUE_TYPE &v )
		{
		    // JSG this is the canonical behaviour for Overwrite(). But when a container doesn't allow
		    // non-const access to elements (eg because it uses them for internal ordering) we will
		    // do a delete()/insert() cycle. Thus, Overwrite() should not be assumed O(1)
		    **this = v;
		}
	};

	typedef iterator const_iterator;

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
    	my_begin.STLCONTAINER::iterator::operator=( STLCONTAINER::begin() );
    	return my_begin;
    }
    virtual const iterator &end()
    {
    	my_end.STLCONTAINER::iterator::operator=( STLCONTAINER::end() );
    	return my_end;
    }
    virtual int size() const
    {
        return STLCONTAINER::size();
    }
    virtual void clear()
    {
    	return STLCONTAINER::clear();
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
    VALUE_TYPE * const element;

    PointIterator( VALUE_TYPE *i ) :
        element(i)
    {      
        ASSERT(i); // We don't support NULL  
    }

	virtual shared_ptr<typename STLContainerBase<SUB_BASE, VALUE_TYPE>::iterator_base> Clone() const
	{
		shared_ptr<PointIterator> ni( new PointIterator(element) );
		return ni;
	}

	virtual PointIterator &operator++()
	{
		ASSERTFAIL("Increment not allowed on point iterator");
	}

	virtual VALUE_TYPE &operator*()
	{
	    ASSERT(element)("Tried to dereference NULL PointIterator");
		return *element;
	}

	virtual VALUE_TYPE *operator->()
	{
	    ASSERT(element)("Tried to dereference NULL PointIterator");
		return element;
	}

	virtual bool operator==( const typename STLContainerBase<SUB_BASE, VALUE_TYPE>::iterator_base &ib )
	{
		if( const PointIterator *pi = dynamic_cast<const PointIterator *>(&ib) )
			return pi->element == element;
		else
			return false; // comparing iterators of different types; must be from different containers
	}
	
	virtual void Overwrite( VALUE_TYPE &v )
	{
	    *element = v;
	}
};

#endif /* GENERICS_HPP */
