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
class ContainerBase : public virtual SUB_BASE
{
protected:
	// Abstract base class for the iterators in sub-containers. This is just to get
	// virtual calls - this is not the generic iterator.
	struct iterator_base
	{
		// TODO const iterator and const versions of begin(), end()
		virtual shared_ptr<iterator_base> Clone() const = 0; // Make another copy of the present iterator
		virtual iterator_base &operator++() = 0;
		virtual VALUE_TYPE &operator*() = 0; // GenericSharedPtr
		virtual bool operator==( const iterator_base &ib ) = 0;
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

		bool operator==( const iterator &i )
		{
			return pib->operator==( *(i.pib) );
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
struct Container : virtual ContainerBase<SUB_BASE, VALUE_TYPE>, STLCONTAINER
{
	typedef STLCONTAINER STLContainer;
	typedef ContainerBase<SUB_BASE, VALUE_TYPE> Base;

	struct iterator : public STLContainer::iterator, public Base::iterator_base
	{
		virtual shared_ptr<typename Base::iterator_base> Clone() const
		{
			shared_ptr<iterator> ni( new iterator );
			*ni = *this;
			return ni;
		}

		virtual iterator &operator++()
		{
		    STLContainer::iterator::operator++();
		    return *this;
		}

		virtual typename STLContainer::value_type &operator*()
		{
			return STLContainer::iterator::operator*();
		}

		virtual bool operator==( const typename Base::iterator_base &ib )
		{
			if( const iterator *pi = dynamic_cast<const iterator *>(&ib) )
				return *pi == *this;
			else
				return false; // comparing iterators of different types; must be from different containers
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
    	my_begin.STLContainer::iterator::operator=( STLContainer::begin() );
    	return my_begin;
    }
    virtual const iterator &end()
    {
    	my_end.STLContainer::iterator::operator=( STLContainer::end() );
    	return my_end;
    }
    virtual int size() const
    {
        return STLContainer::size();
    }
    virtual void clear()
    {
    	return STLContainer::clear();
    }
};



#endif /* GENERICS_HPP */
