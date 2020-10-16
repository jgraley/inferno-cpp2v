/*
 * containers.hpp
 *
 *  Created on: 30 Aug 2009
 *      Author: jgraley
 */

#ifndef CONTAINERS_HPP
#define CONTAINERS_HPP

#include "common/common.hpp"
#include "itemise.hpp"
#include "shared_ptr.hpp"

/// OOStd namespace contains wrappers for STL and Boost features adding run-time polymorphism
namespace OOStd {

//
// Template for a base class for STL containers with forward iterators.
// Supports direct calls and iterators.
//
// VALUE_INTERFACE should
// be a base (or compatible type) for the elements of all sub-containers.
//

class ContainerInterface : public virtual Traceable, public virtual Itemiser::Element
{
public:
	// Abstract base class for the implementation-specific iterators in containers.
	struct iterator_interface : public Traceable
	{
		// TODO const iterator and const versions of begin(), end()
		virtual shared_ptr<iterator_interface> Clone() const = 0; // Make another copy of the present iterator
		virtual iterator_interface &operator++() = 0;
		virtual iterator_interface &operator--() { ASSERTFAIL("Only on reversible iterator"); };
		const virtual SharedPtrInterface &operator*() const = 0;
		const virtual SharedPtrInterface *operator->() const = 0;
		virtual bool operator==( const iterator_interface &ib ) const = 0;
		virtual void Overwrite( const SharedPtrInterface *v ) const = 0;
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
		typedef SharedPtrInterface value_type;
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

		iterator &operator--()
		{
			ASSERT(pib)("Attempt to increment uninitialised iterator");
			EnsureUnique();
			pib->operator--();
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
			ASSERT(pib && i.pib)("Attempt to compare uninitialised iterator %s==%s", pib?"i":"U", i.pib?"i":"U");
			return pib->operator==( *(i.pib) );
		}

		bool operator!=( const iterator &i ) const
		{
			ASSERT(pib && i.pib)("Attempt to compare uninitialised iterator %s==%s", pib?"i":"U", i.pib?"i":"U");
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

		iterator_interface *GetUnderlyingIterator() const
		{
			if( pib )
				return pib.get();
			else
				return nullptr;
		}
		
		virtual shared_ptr<iterator_interface> Clone() // TODO does this need to be virtual?
		{
			ASSERT(pib)("Attempt to Clone() uninitialised iterator");
			return pib->Clone();
		}
		
		operator string()
		{   
		    if( pib )
		        return (string)(Traceable::CPPFilt( typeid( *pib ).name() ));
		    else 
		        return (string)("no-impl");
		}
	private:
		void EnsureUnique()
		{
			// Call this before modifying the underlying iterator - Performs a deep copy
			// if required to make sure there are no other refs.
			if( pib && !pib.unique() )
				pib = Clone();
			ASSERT( !pib || pib.unique() );
		}

		shared_ptr<iterator_interface> pib;
	};
	typedef iterator const_iterator; // TODO const iterators properly

	// These direct calls to the container are designed to support co-variance.
    virtual void insert( const SharedPtrInterface &gx ) = 0;
	virtual const iterator_interface &begin() = 0;
    virtual const iterator_interface &end() = 0;
    virtual void erase( const iterator_interface &it ) = 0;
    virtual bool empty() { return begin()==end(); }
    virtual int size() const
    {
    	// TODO support const_interator properly and get rid of this const_cast
    	ContainerInterface *nct = const_cast<ContainerInterface *>(this);
    	int n=0;
    	FOREACH( const SharedPtrInterface &x, *nct )
    	    n++;
    	return n;
    }
    virtual void clear() = 0;
};


struct SequenceInterface : virtual ContainerInterface
{

    virtual void push_back( const SharedPtrInterface &gx ) = 0;
};


struct SimpleAssociativeContainerInterface : virtual ContainerInterface
{
	virtual int erase( const SharedPtrInterface &gx ) = 0;
};


//
// Abstract template for containers that will be use any STL container as
// the actual implementation.
// Params as for ContainerInterface except we now have to fill in CONTAINER_IMPL
// as the stl container class eg std::list<blah>
//
template<class CONTAINER_IMPL>
struct ContainerCommon : virtual ContainerInterface, CONTAINER_IMPL
{
    // C++11 fix
    ContainerCommon& operator=(const ContainerCommon& other)
    {
        (void)CONTAINER_IMPL::operator=(other);
        return *this;
    }
    
	struct iterator : public CONTAINER_IMPL::iterator,
	                  public ContainerInterface::iterator_interface
	{
		virtual iterator &operator++()
		{
			CONTAINER_IMPL::iterator::operator++();
		    return *this;
		}

		virtual iterator &operator--()
		{
			CONTAINER_IMPL::iterator::operator--();
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

		virtual bool operator==( const typename ContainerInterface::iterator_interface &ib ) const
		{
		    const typename CONTAINER_IMPL::iterator *pi = dynamic_cast<const typename CONTAINER_IMPL::iterator *>(&ib);
		    ASSERT(pi)("Comparing iterators of different type");
			return *(const typename CONTAINER_IMPL::iterator *)this == *pi;
		}
	};

	typedef iterator const_iterator;

    virtual void erase( const typename ContainerInterface::iterator &it )
    {
        erase( *it.GetUnderlyingIterator() );
    }
    virtual void erase( const typename ContainerInterface::iterator_interface &it )
    {
        auto cit = dynamic_cast<const iterator *>( &it );
        ASSERT( cit ); // if this fails, you passed erase() the wrong kind of iterator
        CONTAINER_IMPL::erase( *(typename CONTAINER_IMPL::iterator *)cit );    
    }
    virtual bool empty() 
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
template<class CONTAINER_IMPL>
struct Sequence : virtual ContainerCommon<CONTAINER_IMPL>, virtual SequenceInterface
{
    // C++11 fix
    Sequence& operator=(const Sequence& other)
    {
        (void)ContainerCommon<CONTAINER_IMPL>::operator=(other);
        return *this;
    }

    using typename CONTAINER_IMPL::insert; // due to silly C++ rule where different overloads hide each other
    inline Sequence<CONTAINER_IMPL>() {}
	struct iterator : public ContainerCommon<CONTAINER_IMPL>::iterator
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
		virtual shared_ptr<typename ContainerInterface::iterator_interface> Clone() const
		{
			shared_ptr<iterator> ni( new iterator );
			*ni = *this;
			return ni;
		}
    	virtual void Overwrite( const SharedPtrInterface *v ) const
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

	virtual void insert( const SharedPtrInterface &gx ) // Simulating the SimpleAssociatedContaner API 
	{
        // Like multiset, we do allow more than one copy of the same element
		push_back( gx );
	}
    using ContainerCommon<CONTAINER_IMPL>::erase;
    virtual int erase( const SharedPtrInterface &gx ) // Simulating the SimpleAssociatedContaner API 
    {
        // Like multiset, we erase all matching elemnts. Doing this though the API, bearing in 
        // mind validity rules post-erase, is horrible.
        typename CONTAINER_IMPL::value_type sx( CONTAINER_IMPL::value_type::InferredDynamicCast(gx) );
        typename CONTAINER_IMPL::iterator it;
        int count = 0;
        do 
        {
            for( it=begin(); it != end(); ++it )
            {
                if( *it==sx )
                {
                    CONTAINER_IMPL::erase(it);
                    count++;
                    break;
                }
            }
        }
        while( it != end() );
        return count;
    }
	virtual void push_back( const SharedPtrInterface &gx )
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
	Sequence( const SequenceInterface &cns )
	{
		// TODO support const_interator properly and get rid of this const_cast
		SequenceInterface *ns = const_cast< SequenceInterface * >( &cns );
		for( typename SequenceInterface::iterator i=ns->begin();
		     i != ns->end();
		     ++i )
		{
            CONTAINER_IMPL::push_back( (typename CONTAINER_IMPL::value_type)*i );
		}
	}
	Sequence( const SharedPtrInterface &nx )
	{
		typename CONTAINER_IMPL::value_type sx( CONTAINER_IMPL::value_type::InferredDynamicCast(nx) );
		CONTAINER_IMPL::push_back( sx );
	}
	template<typename L, typename R>
	inline Sequence( const pair<L, R> &p )
	{
		*this = Sequence<CONTAINER_IMPL>( p.first );
		Sequence<CONTAINER_IMPL> t( p.second );

		for( typename Sequence<CONTAINER_IMPL>::iterator i=t.begin();
		     i != t.end();
		     ++i )
		{
            CONTAINER_IMPL::push_back( *i );
		}
	}
    Sequence& operator=( std::initializer_list<typename CONTAINER_IMPL::value_type> ilv )
    {
        CONTAINER_IMPL::clear();
		for( const typename CONTAINER_IMPL::value_type &v : ilv )
		{
            CONTAINER_IMPL::push_back( v );
		}        
        return *this;
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
template<class CONTAINER_IMPL>
struct SimpleAssociativeContainer : virtual ContainerCommon<CONTAINER_IMPL>, virtual SimpleAssociativeContainerInterface
{
    inline SimpleAssociativeContainer<CONTAINER_IMPL>() {}
	struct iterator : public ContainerCommon<CONTAINER_IMPL>::iterator
    {
		inline iterator( typename CONTAINER_IMPL::iterator &i ) : CONTAINER_IMPL::iterator(i) {}
		inline iterator() {}
		virtual shared_ptr<typename ContainerInterface::iterator_interface> Clone() const
		{
			shared_ptr<iterator> ni( new iterator );
			*ni = *this;
			return ni;
		}
    	virtual void Overwrite( const SharedPtrInterface *v ) const
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
        SimpleAssociativeContainer<CONTAINER_IMPL> *owner;
	};

	virtual void insert( const SharedPtrInterface &gx )
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
    using ContainerCommon<CONTAINER_IMPL>::erase;
	virtual int erase( const SharedPtrInterface &gx )
	{
		typename CONTAINER_IMPL::value_type sx( CONTAINER_IMPL::value_type::InferredDynamicCast(gx) );
		return CONTAINER_IMPL::erase( sx );
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
    SimpleAssociativeContainer( const ContainerInterface &cns )
	{
		// TODO support const_interator properly and get rid of this const_cast
    	ContainerInterface *ns = const_cast< ContainerInterface * >( &cns );
		for( typename ContainerInterface::iterator i=ns->begin();
		     i != ns->end();
		     ++i )
		{
            CONTAINER_IMPL::insert( *i );
		}
	}
    SimpleAssociativeContainer( const SharedPtrInterface &nx )
	{
		typename CONTAINER_IMPL::value_type sx( CONTAINER_IMPL::value_type::InferredDynamicCast(nx) );
        CONTAINER_IMPL::insert( sx );
	}
	template<typename L, typename R>
	inline SimpleAssociativeContainer( const pair<L, R> &p )
	{
		*this = SimpleAssociativeContainer<CONTAINER_IMPL>( p.first );
		SimpleAssociativeContainer<CONTAINER_IMPL> t( p.second );

		for( typename SimpleAssociativeContainer<CONTAINER_IMPL>::iterator i=t.begin();
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
struct PointIterator : public ContainerInterface::iterator_interface
{
    SharedPtrInterface * element;

    PointIterator() :
        element(nullptr) // means end-of-range
    {
    }

    PointIterator( const PointIterator &other ) :
        element(other.element)
    {
    }

    PointIterator( SharedPtrInterface *i ) :
        element(i)
    {      
        ASSERT(i); // We don't allow nullptr as input because it means end-of-range
    }

	virtual shared_ptr<typename ContainerInterface::iterator_interface> Clone() const
	{
		shared_ptr<PointIterator> ni( new PointIterator(*this) );
		return ni;
	}

	virtual PointIterator &operator++()
	{
		element = nullptr; // ie if we increment, we get to the end of the range
		return *this;
	}

	virtual SharedPtrInterface &operator*() const
	{
	    ASSERT(element)("Tried to dereference nullptr PointIterator");
		return *element;
	}

	virtual SharedPtrInterface *operator->() const
	{
	    ASSERT(element)("Tried to dereference nullptr PointIterator");
		return element;
	}

	virtual bool operator==( const typename ContainerInterface::iterator_interface &ib ) const
	{
		const PointIterator *pi = dynamic_cast<const PointIterator *>(&ib);
		ASSERT(pi)("Comparing point iterator with something else ")(ib);
		return pi->element == element;
	}
	
	virtual void Overwrite( const SharedPtrInterface *v ) const
	{
	    *element = *v;
	}

	virtual const bool IsOrdered() const
	{
		return true; // shouldn't matter what we return here
	}
};

struct CountingIterator : public ContainerInterface::iterator_interface
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

	virtual shared_ptr<typename ContainerInterface::iterator_interface> Clone() const
	{
		shared_ptr<CountingIterator> ni( new CountingIterator(*this) );
		return ni;
	}

	virtual CountingIterator &operator++()
	{
		element++;
		return *this;
	}

	virtual CountingIterator &operator--()
	{
		element--;
		return *this;
	}

	virtual SharedPtrInterface &operator*() const
	{
	    ASSERTFAIL("Cannot dereference CountingIterator, use GetCount instead");
	}

	const virtual SharedPtrInterface *operator->() const
	{
		ASSERTFAIL("Cannot dereference CountingIterator, use GetCount instead");
	}

	virtual bool operator==( const typename ContainerInterface::iterator_interface &ib ) const
	{
		const CountingIterator *pi = dynamic_cast<const CountingIterator *>(&ib);
		ASSERT(pi)("Comparing counting iterator with something else ")( ib );
		return pi->element == element;
	}

	virtual void Overwrite( const SharedPtrInterface *v ) const
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
