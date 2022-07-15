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
#include "tree_ptr.hpp"

#include <list>
#include <set>
#include <iterator>
#include <algorithm>

#define ASSOCIATIVE_IMPL multiset
#define SEQUENCE_IMPL list

//----------------------- ContainerInterface -------------------------

class ContainerInterface : public virtual Traceable, public virtual Itemiser::Element
{
public:
	// Abstract base class for the implementation-specific iterators in containers.
	struct iterator_interface : public Traceable
	{
		// TODO const iterator and const versions of begin(), end()
		virtual unique_ptr<iterator_interface> Clone() const = 0; // Make another copy of the present iterator
		virtual iterator_interface &operator++() = 0;
		virtual iterator_interface &operator--();
		virtual const TreePtrInterface &operator*() const = 0;
		virtual const TreePtrInterface *operator->() const = 0;
		virtual bool operator==( const iterator_interface &ib ) const = 0;
		virtual void Overwrite( const TreePtrInterface *v ) const = 0;
		virtual const bool IsOrdered() const = 0;
	};

public:
	// Wrapper for iterator_interface, uses std::unique_ptr<> and Clone() to manage the real iterator
	// and forwards all the operations using co-variance where possible. These can be passed around
	// by value, and always-copy semantics are used for simplicity.
    // Since C++ range-for uses the return type of begin() to determine loop index type, we have
    // to return a reference to this class (not iterator_interface) from begin() on generic container
    // types. And to retain covariant return s from begin() and end(), we have to include this class 
    // in the hierarchy of iterators. So we ensure that those iterators aren't unwittingly delegating 
    // into our methods using CHECK_NOT_REACHED_ON_SUBCLASS.
	class iterator : public iterator_interface, public std::iterator<forward_iterator_tag, const TreePtrInterface>
	{
	public:
		iterator();
		iterator( const iterator &ib );
		iterator &operator=( const iterator &ib );
		virtual ~iterator();
        
		iterator( const iterator_interface &ib );
		iterator &operator=( const iterator_interface &ib );
		iterator &operator++();
		iterator &operator--();
		const value_type &operator*() const;
		const value_type *operator->() const;
        bool operator==( const iterator_interface &ib ) const; // isovariant param
        bool operator==( const iterator &i ) const; // covariant param;
		bool operator!=( const iterator_interface &ib ) const; // isovariant param;
		bool operator!=( const iterator &i ) const; // covariant param;
		void Overwrite( const value_type *v ) const;
		const bool IsOrdered() const;
		iterator_interface *GetUnderlyingIterator() const;
		explicit operator string() const;
		explicit operator bool() const;
	
    protected:
   		virtual unique_ptr<iterator_interface> Clone() const;
    
    private:
		unique_ptr<iterator_interface> pib;
	};
	typedef iterator const_iterator; // TODO const iterators properly

	// These direct calls to the container are designed to support co-variance.
    virtual void insert( const TreePtrInterface &gx ) = 0;
	virtual const iterator &begin() = 0;
    virtual const iterator &end() = 0;
    virtual const TreePtrInterface &front();
    virtual const TreePtrInterface &back();
    virtual void erase( const iterator_interface &it ) = 0;
    virtual bool empty();
    virtual int size() const;
    virtual void clear() = 0;
};


struct SequenceInterface : virtual ContainerInterface
{
};


struct CollectionInterface : virtual ContainerInterface
{
};

//
// Abstract template for containers that will be use any STL container as
// the actual implementation.
// Params as for ContainerInterface except we now have to fill in Impl
// as the stl container class eg std::list<blah>
//
template<class CONTAINER_IMPL>
struct ContainerCommon : virtual ContainerInterface, CONTAINER_IMPL
{
    typedef CONTAINER_IMPL Impl;
    typedef typename Impl::value_type value_type;
    
    // C++11 fix
    ContainerCommon& operator=(const ContainerCommon& other)
    {
        (void)Impl::operator=(other);
        (void)ContainerInterface::operator=(other); // For Itemise
        return *this;
    }
    
	struct iterator : public Impl::iterator,
	                  public ContainerInterface::iterator
	{
		iterator() {}

        iterator( const iterator &ib ) :
            Impl::iterator( (typename Impl::iterator &)ib )
        {
            // Avoid delegating to ContainerInterface::iterator
        }
        
		iterator &operator=( const iterator &ib )
        {
            Impl::iterator::operator=( (typename Impl::iterator &)ib );
            // Avoid delegating to ContainerInterface::iterator
            return *this;
        }
        
        virtual iterator &operator++()
		{
			Impl::iterator::operator++();
		    return *this;
		}

		virtual iterator &operator--()
		{
			Impl::iterator::operator--();
		    return *this;
		}

		virtual const value_type &operator*() const
		{
			return Impl::iterator::operator*();
		}

		virtual const value_type *operator->() const
		{
			return Impl::iterator::operator->();
		}

		virtual bool operator==( const typename ContainerInterface::iterator_interface &ib ) const
		{
		    const typename Impl::iterator *pi = dynamic_cast<const typename Impl::iterator *>(&ib);
		    ASSERT(pi)("Comparing iterators of different type");
			return *(const typename Impl::iterator *)this == *pi;
		}
		virtual bool operator!=( const typename ContainerInterface::iterator_interface &ib ) const
		{
			return !operator==(ib);
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
        Impl::erase( *(typename Impl::iterator *)cit );    
    }
    virtual bool empty() 
    {
        return Impl::empty();
    }
    virtual int size() const
    {
        return Impl::size();
    }
    virtual int count( const TreePtrInterface &gx )
    {
		value_type sx( value_type::InferredDynamicCast(gx) );
        return std::count( Impl::begin(), Impl::end(), sx );
    }
    virtual void clear()
    {
    	return Impl::clear();
    }
	virtual operator string() const
	{
        return TYPE_ID_NAME( value_type );
	}
};


//
// Template for containers that use ordered STL containers as implementation
// (basically vector, deque etc). Instantiate as per ContainerCommon.
//
template<class VALUE_TYPE>
struct Sequential : virtual ContainerCommon< SEQUENCE_IMPL< TreePtr<VALUE_TYPE> > >
{
    typedef SEQUENCE_IMPL< TreePtr<VALUE_TYPE> > Impl;
    typedef TreePtr<VALUE_TYPE> value_type;
    
    // C++11 fix
    Sequential& operator=(const Sequential& other)
    {
        (void)ContainerCommon<Impl>::operator=(other);
        return *this;
    }

    using Impl::insert; // due to silly C++ rule where different overloads hide each other
    inline Sequential<VALUE_TYPE>() {}
	struct iterator : public ContainerCommon<Impl>::iterator
    {
		inline iterator( typename Impl::iterator &i ) : Impl::iterator(i) {}
		inline iterator() {}
		virtual value_type &operator*() const
		{
            ASSERT(this);
			return Impl::iterator::operator*();
		}
		virtual value_type *operator->() const
		{
            ASSERT(this);
			return Impl::iterator::operator->();
		}
		virtual unique_ptr<typename ContainerInterface::iterator_interface> Clone() const
		{
            // Avoid delegating to ContainerInterface::iterator.
			auto ni = make_unique<iterator>();
            ni->Impl::iterator::operator=( (typename Impl::iterator &)*this );
			return ni;
		}
    	virtual void Overwrite( const TreePtrInterface *v ) const
		{
		    // JSG Overwrite() just writes through the pointer got from dereferencing the iterator,
		    // because in Sequences (ordererd containers) elements may be modified.
            // Avoid delegating to ContainerInterface::iterator.
    		value_type x( value_type::InferredDynamicCast(*v) );
		    Impl::iterator::operator*() = x;
		}
    	virtual const bool IsOrdered() const
    	{
    		return true; // yes, Sequences are ordered
    	}
	};

	virtual void insert( const TreePtrInterface &gx ) // Simulating the SimpleAssociatedContaner API 
	{
        // Like multiset, we do allow more than one copy of the same element
		push_back( gx );
	}
    using ContainerCommon<Impl>::erase;
    virtual int erase( const TreePtrInterface &gx ) // Simulating the SimpleAssociatedContaner API 
    {
        // Like multiset, we erase all matching elemnts. Doing this though the API, bearing in 
        // mind validity rules post-erase, is horrible.
        value_type sx( value_type::InferredDynamicCast(gx) );
        typename Impl::iterator it;
        int n = 0;
        do 
        {
            for( it=begin(); it != end(); ++it )
            {
                if( *it==sx )
                {
                    Impl::erase(it);
                    n++;
                    break;
                }
            }
        }
        while( it != end() );
        return n;
    }
	virtual void push_back( const TreePtrInterface &gx )
	{
		value_type sx( value_type::InferredDynamicCast(gx) );
		Impl::push_back( sx );
	}
	template<typename OTHER>
	inline void push_back( const OTHER &gx )
	{
		value_type sx(gx);
		Impl::push_back( sx );
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
    	my_begin.Impl::iterator::operator=( Impl::begin() );
    	return my_begin;
    }
    virtual const iterator &end()
    {
    	my_end.Impl::iterator::operator=( Impl::end() );
    	return my_end;
    }  
    virtual const TreePtr<VALUE_TYPE> &front()
    {
        ASSERT( !ContainerCommon<Impl>::empty() )("Attempting to obtain front() of an empty sequence");
        return Impl::front();
    }
    virtual const TreePtr<VALUE_TYPE> &back()
    {
        ASSERT( !ContainerCommon<Impl>::empty() )("Attempting to obtain back() of an empty sequence");
        return Impl::back();
    }

	Sequential( const ContainerInterface &cns )
	{
		// TODO support const_interator properly and get rid of this const_cast
		ContainerInterface *ns = const_cast< ContainerInterface * >( &cns );
		for( typename ContainerInterface::iterator i=ns->begin();
		     i != ns->end();
		     ++i )
		{
            Impl::push_back( (value_type)*i );
		}
	}
	explicit Sequential( const TreePtrInterface &nx )
	{
		value_type sx( value_type::InferredDynamicCast(nx) );
		Impl::push_back( sx );
	}
	template<typename L, typename R>
	explicit inline Sequential( const pair<L, R> &p )
	{
		*this = Sequential<VALUE_TYPE>( p.first );
		Sequential<VALUE_TYPE> t( p.second );

		for( typename Sequential<VALUE_TYPE>::iterator i=t.begin();
		     i != t.end();
		     ++i )
		{
            Impl::push_back( *i );
		}
	}
	explicit inline Sequential( const value_type &v )
	{
		push_back( v );
	}
};


//
// Template for containers that use unordered STL containers as implementation
// (basically associative containers). Instantiate as per ContainerCommon.
//
template<class VALUE_TYPE>
struct SimpleAssociativeContainer : virtual ContainerCommon< ASSOCIATIVE_IMPL< TreePtr<VALUE_TYPE> > >
{
    typedef ASSOCIATIVE_IMPL< TreePtr<VALUE_TYPE> > Impl;
    typedef TreePtr<VALUE_TYPE> value_type;

    inline SimpleAssociativeContainer<VALUE_TYPE>() {}
	struct iterator : public ContainerCommon<Impl>::iterator
    {
		inline iterator( typename Impl::iterator &i ) : Impl::iterator(i) {}
		inline iterator() {}
		virtual unique_ptr<typename ContainerInterface::iterator_interface> Clone() const
		{
            // Avoid delegating to ContainerInterface::iterator.
			auto ni = make_unique<iterator>();
            ni->Impl::iterator::operator=( (typename Impl::iterator &)*this );
			return ni;
		}
    	virtual void Overwrite( const TreePtrInterface *v ) const
		{
		    // SimpleAssociativeContainers (unordered containers) do not allow elements to be modified
		    // because the internal data structure depends on element values. So we 
		    // erase the old element and insert the new one; thus, Overwrite() should not be assumed O(1)
            // Avoid delegating to ContainerInterface::iterator.
    		value_type s( value_type::InferredDynamicCast(*v) );
    		((Impl *)owner)->erase( *this );
		    *(typename Impl::iterator *)this = ((Impl *)owner)->insert( s ); // become an iterator for the newly inserted element
 		}
    	virtual const bool IsOrdered() const
    	{
    		return false; // no, SimpleAssociativeContainers are not ordered
    	}
        SimpleAssociativeContainer<VALUE_TYPE> *owner;
	};

	virtual void insert( const TreePtrInterface &gx )
	{
		value_type sx( value_type::InferredDynamicCast(gx) );
		Impl::insert( sx );
	}
	template<typename OTHER>
	inline void insert( const OTHER &gx )
	{
		value_type sx(gx);
		Impl::insert( sx );
	}
    using ContainerCommon<Impl>::erase;
	virtual int erase( const TreePtrInterface &gx )
	{
		value_type sx( value_type::InferredDynamicCast(gx) );
		return Impl::erase( sx );
	}

	iterator my_begin, my_end;
    virtual const iterator &begin()
    {
    	my_begin.Impl::iterator::operator=( Impl::begin() );
    	my_begin.owner = this;
    	return my_begin;
    }
    virtual const iterator &end()
    {
    	my_end.Impl::iterator::operator=( Impl::end() );
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
            Impl::insert( *i );
		}
	}
    explicit SimpleAssociativeContainer( const TreePtrInterface &nx )
	{
		value_type sx( value_type::InferredDynamicCast(nx) );
        Impl::insert( sx );
	}
	template<typename L, typename R>
	explicit inline SimpleAssociativeContainer( const pair<L, R> &p )
	{
		*this = SimpleAssociativeContainer<VALUE_TYPE>( p.first );
		SimpleAssociativeContainer<VALUE_TYPE> t( p.second );

		for( typename SimpleAssociativeContainer<VALUE_TYPE>::iterator i=t.begin();
		     i != t.end();
		     ++i )
		{
            Impl::insert( *i );
		}
	}
	explicit inline SimpleAssociativeContainer( const value_type &v )
	{
		insert( v );
	}
};


// Allow operator, to be used to create pairs. Also handy for maps.
template<typename L, typename R>
inline pair<L,R> operator,( const L &l, const R &r )
{
    return pair<L,R>(l,r);
}

#define USE_LIST_FOR_COLLECTION 1

// Inferno tree shared pointers

#if USE_LIST_FOR_COLLECTION
#define COLLECTION_BASE Sequential
#else
#define COLLECTION_BASE SimpleAssociativeContainer
#endif

template<typename VALUE_TYPE>
struct Sequence : virtual Sequential< VALUE_TYPE >,
                  virtual SequenceInterface
{
// 	using Sequential< VALUE_TYPE >::Sequential;

	inline Sequence() {}
	template<typename L, typename R>
	inline Sequence( const pair<L, R> &p ) : // explicit missing due usage in steps
		Sequential< VALUE_TYPE >( p ) {}
	template< typename OTHER >
	inline Sequence( const TreePtr<OTHER> &v ) : // explicit missing due usage in steps
		Sequential< VALUE_TYPE >( v ) {}
};

template<typename VALUE_TYPE> 
struct Collection : virtual COLLECTION_BASE< VALUE_TYPE >,
                    virtual CollectionInterface
{
 	inline Collection<VALUE_TYPE>() {}
	template<typename L, typename R>
	inline Collection( const pair<L, R> &p ) : // explicit missing due usage in steps
		COLLECTION_BASE< VALUE_TYPE >( p ) {}
	template< typename OTHER >
	inline Collection( const TreePtr<OTHER> &v ) : // explicit missing due usage in steps
		COLLECTION_BASE< VALUE_TYPE >( v ) {}
};

#endif /* GENERICS_HPP */
