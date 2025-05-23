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


// If defined, make Mutate() perform erase+insert as with associative containers.
// This should ensure that the TreePtr<X> instance is deleted and re-created, 
// rather than just modified. This invalidates any TreePtrInterface * or XLink
// that pointed to it. The aliassing shared pointer in the XLink doesn't help
// because it only keeps the node alive, not the contqainer element.  
//#define FORCE_DEEP_MUTATE

//----------------------- ContainerInterface -------------------------

class ContainerInterface : public virtual Itemiser::Element
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
        virtual void Mutate( const TreePtrInterface *v ) const = 0;
        virtual bool IsOrdered() const = 0;
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
        void Mutate( const value_type *v ) const;
        bool IsOrdered() const;
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
    virtual const iterator &begin() = 0;
    virtual const iterator &end() = 0;
    virtual const iterator &insert( const TreePtrInterface &gx ) = 0;
    virtual const iterator &insert( const iterator_interface &pos, const TreePtrInterface &gx ) = 0;
    const iterator &insert( const iterator &pos, const TreePtrInterface &gx );
    virtual const iterator &insert_front( const TreePtrInterface &gx ) = 0;
    virtual const TreePtrInterface &front();
    virtual const TreePtrInterface &back();
    virtual const iterator &erase( const iterator_interface &it ) = 0;
    const iterator &erase( const iterator &it );
    virtual bool empty();
    virtual int size() const;
    //virtual int count( const TreePtrInterface &gx ) = 0;
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

    bool empty() override
    {
        return Impl::empty();
    }
    int size() const override
    {
        return Impl::size();
    }
    virtual int count( const TreePtrInterface &gx )
    {
        value_type sx( value_type::InferredDynamicCast(gx) );
        return std::count( Impl::begin(), Impl::end(), sx );
    }
    void clear() override
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
            ASSERTTHIS();
            return Impl::iterator::operator*();
        }
        virtual value_type *operator->() const
        {
            ASSERTTHIS();
            return Impl::iterator::operator->();
        }
        virtual unique_ptr<typename ContainerInterface::iterator_interface> Clone() const
        {
            // Avoid delegating to ContainerInterface::iterator.
            auto ni = make_unique<iterator>();
            ni->Impl::iterator::operator=( (typename Impl::iterator &)*this );
#ifdef FORCE_DEEP_MUTATE
			ni->owner = owner;
#endif			
            return ni;
        }
        virtual void Mutate( const TreePtrInterface *v ) const
        {
            // JSG Mutate() just writes through the pointer got from dereferencing the iterator,
            // because in Sequences (ordererd containers) elements may be modified.
            // Avoid delegating to ContainerInterface::iterator.
            value_type x( value_type::InferredDynamicCast(*v) );
#ifdef FORCE_DEEP_MUTATE
    		auto it_pos = ((Impl *)owner)->erase( *this );
		    *(typename Impl::iterator *)this = ((Impl *)owner)->insert( it_pos, x ); // become an iterator for the newly inserted element
#else
		    Impl::iterator::operator*() = x;
#endif        
		}
        virtual bool IsOrdered() const
        {
            return true; // yes, Sequences are ordered
        }
#ifdef FORCE_DEEP_MUTATE
        Sequential<VALUE_TYPE> *owner = nullptr;
#endif
    };

    using ContainerInterface::erase;
    virtual int erase( const TreePtrInterface &gx ) // Simulating the SimpleAssociatedContaner API 
    {
        // Like multiset, we erase all matching elemnts. 
        value_type sx( value_type::InferredDynamicCast(gx) );
        typename Impl::iterator it;
        int n = 0;
        for( it=begin(); it != end(); )
        {
            if( *it==sx )
            {
                it = Impl::erase(it);
                n++;
            }
            else
            {
                ++it;
            }
        }
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
    virtual void push_front( const TreePtrInterface &gx )
    {
        value_type sx( value_type::InferredDynamicCast(gx) );
        Impl::push_front( sx );
    }
    template<typename OTHER>
    inline void push_front( const OTHER &gx )
    {
        value_type sx(gx);
        Impl::push_front( sx );
    }

    const iterator &begin() override
    {
        my_begin.Impl::iterator::operator=( Impl::begin() );
#ifdef FORCE_DEEP_MUTATE
		ASSERTTHIS();
    	my_begin.owner = this;
#endif  
        return my_begin;
    }
    const iterator &end() override
    {
        my_end.Impl::iterator::operator=( Impl::end() );
#ifdef FORCE_DEEP_MUTATE
		ASSERTTHIS();
    	my_end.owner = this;
#endif 
        return my_end;
    }  
    const iterator &insert( const TreePtrInterface &gx ) override // Simulating the SimpleAssociatedContaner API 
    {
        // Like multiset, we do allow more than one copy of the same element
        value_type sx( value_type::InferredDynamicCast(gx) );
        Impl::push_back( sx );
        my_inserted.Impl::iterator::operator=( prev(Impl::end()) );
#ifdef FORCE_DEEP_MUTATE
		ASSERTTHIS();
    	my_inserted.owner = this;
#endif 
        return my_inserted;
    }
    const iterator &insert( const ContainerInterface::iterator_interface &pos, 
                            const TreePtrInterface &gx ) override // Simulating the SimpleAssociatedContaner API 
    {
        // Like multiset, we do allow more than one copy of the same element        
        auto posit = dynamic_cast<const iterator *>( &pos );
        ASSERT( posit ); // if this fails, you passed insert() the wrong kind of iterator
        value_type sx( value_type::InferredDynamicCast(gx) );
        my_inserted.Impl::iterator::operator=( Impl::insert( *(typename Impl::iterator *)posit, sx ) );
#ifdef FORCE_DEEP_MUTATE
		ASSERTTHIS();
    	my_inserted.owner = this;
#endif 
        return my_inserted;
    }
    const iterator &insert_front( const TreePtrInterface &gx ) override // Simulating the SimpleAssociatedContaner API 
    {
        // Like multiset, we do allow more than one copy of the same element
        value_type sx( value_type::InferredDynamicCast(gx) );
        Impl::push_front( sx );
        my_inserted.Impl::iterator::operator=( prev(Impl::end()) );
#ifdef FORCE_DEEP_MUTATE
		ASSERTTHIS();
    	my_inserted.owner = this;
#endif 
        return my_inserted;
    }

    const TreePtr<VALUE_TYPE> &front() override
    {
        ASSERT( !ContainerCommon<Impl>::empty() )("Attempting to obtain front() of an empty sequence");
        return Impl::front();
    }
    const TreePtr<VALUE_TYPE> &back() override
    {
        ASSERT( !ContainerCommon<Impl>::empty() )("Attempting to obtain back() of an empty sequence");
        return Impl::back();
    }

    const iterator &erase( const typename ContainerInterface::iterator_interface &it ) override
    {
        auto cit = dynamic_cast<const iterator *>( &it );
        ASSERT( cit ); // if this fails, you passed erase() the wrong kind of iterator        
        my_erased.Impl::iterator::operator=( Impl::erase( *(typename Impl::iterator *)cit ) );   
#ifdef FORCE_DEEP_MUTATE
		ASSERTTHIS();
    	my_erased.owner = this;
#endif 
        return my_erased;
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

    // Covariant style only works with refs and pointers, so force begin/end to return refs safely
    // This complies with STL's thread safety model. To quote SGI,
    // "The SGI implementation of STL is thread-safe only in the sense that simultaneous accesses
    // to distinct containers are safe, and simultaneous read accesses to to shared containers are
    // safe. If multiple threads access a single container, and at least one thread may potentially
    // write, then the user is responsible for ensuring mutual exclusion between the threads during
    // the container accesses."
    // So that's OK then.
    iterator my_begin, my_end, my_inserted, my_erased;
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
        virtual void Mutate( const TreePtrInterface *v ) const
        {
            // SimpleAssociativeContainers (unordered containers) do not allow elements to be modified
            // because the internal data structure depends on element values. So we 
            // erase the old element and insert the new one; thus, Mutate() should not be assumed O(1)
            // Avoid delegating to ContainerInterface::iterator.
            value_type s( value_type::InferredDynamicCast(*v) );
            ((Impl *)owner)->erase( *this );
            *(typename Impl::iterator *)this = ((Impl *)owner)->insert( s ); // become an iterator for the newly inserted element
         }
        virtual bool IsOrdered() const
        {
            return false; // no, SimpleAssociativeContainers are not ordered
        }
        SimpleAssociativeContainer<VALUE_TYPE> *owner;
    };

    using ContainerCommon<Impl>::erase;
    int erase( const TreePtrInterface &gx ) override
    {
        value_type sx( value_type::InferredDynamicCast(gx) );
        return Impl::erase( sx );
    }

    const iterator &begin() override
    {
        my_begin.Impl::iterator::operator=( Impl::begin() );
        my_begin.owner = this;
        return my_begin;
    }
    const iterator &end() override
    {
        my_end.Impl::iterator::operator=( Impl::end() );
        my_end.owner = this;
        return my_end;
    }
    const iterator &insert( const TreePtrInterface &gx ) override
    {
        value_type sx( value_type::InferredDynamicCast(gx) );
        auto p = Impl::insert( sx );
        if( p.second )
            my_inserted.Impl::iterator::operator=( p.first );
        else
            my_inserted.Impl::iterator::operator=( Impl::end() );
        my_inserted.owner = this;
        return my_inserted;
    }
    const iterator &insert( const ContainerInterface::iterator_interface &pos, 
                            const TreePtrInterface &gx ) override
    {
        auto posit = dynamic_cast<const iterator *>( &pos );
        ASSERT( posit ); // if this fails, you passed insert() the wrong kind of iterator
        value_type sx( value_type::InferredDynamicCast(gx) );
        auto p = Impl::insert( *(typename Impl::iterator *)posit, sx );
        my_inserted.Impl::iterator::operator=( p.first );
        my_inserted.owner = this;
        return my_inserted;
    }
    const iterator &insert_front( const TreePtrInterface &gx ) override
    {
        return insert(gx);
    }
    template<typename OTHER>
    const iterator &insert( const OTHER &gx )
    {
        value_type sx(gx);
        auto p = Impl::insert( sx );
        if( p.second )
            my_inserted.Impl::iterator::operator=( p.first );
        else
            my_inserted.Impl::iterator::operator=( Impl::end() );
        my_inserted.owner = this;
        return my_inserted;
    }
    template<typename OTHER>
    const iterator &insert( const typename ContainerInterface::iterator_interface &pos, 
                            const OTHER &gx )
    {
        value_type sx( value_type::InferredDynamicCast(gx) );
        auto p = Impl::insert( pos, sx );
        my_inserted.Impl::iterator::operator=( p.first );
        my_inserted.owner = this;
        return my_inserted;
    }
    template<typename OTHER>
    const iterator &insert_front( const OTHER &gx )
    {
        return insert(gx);
    }

    const iterator &erase( const typename ContainerInterface::iterator_interface &it ) override
    {
        auto cit = dynamic_cast<const iterator *>( &it );
        ASSERT( cit ); // if this fails, you passed erase() the wrong kind of iterator        
        my_erased.Impl::iterator::operator=( Impl::erase( *(typename Impl::iterator *)cit ) );   
        return my_erased;
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
    
    // Permit return by reference for covariance.
    iterator my_begin, my_end, my_inserted, my_erased;
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
//     using Sequential< VALUE_TYPE >::Sequential;

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


struct ScaffoldBase
{
    Sequence<Node> child_ptrs;    
};


// Scaffold nodes for temporary insertion into the tree.
// Scaffold nodes need to match the type of an existing TreePtr, so we have
// to template them and create them from TreePtrs.
template<typename VALUE_TYPE>
struct Scaffold : VALUE_TYPE, ScaffoldBase
{
    NODE_FUNCTIONS_FINAL
};


class TreeUtilsInterface
{
public:
    virtual TreePtr<Node> MakeScaffold() const = 0;	
};


template<typename VALUE_TYPE>
class TreeUtils : public TreeUtilsInterface
{
    TreePtr<Node> MakeScaffold() const final
	{
		// Don't instance TreePtr<Scaffold<X>> because very bad things happen
		// including gcc 10.5 spinning forever chewing up memory (presumably
		// it's contemplating TreePtr<Scaffold<Scaffold<X>>> etc). 
		auto scaffold_sp = make_shared<Scaffold<VALUE_TYPE>>(); 
		TreePtr<Node> scaffold( scaffold_sp );
		return scaffold;
	}
};


template< class TYPE >
const TreeUtilsInterface *MakeTPStatic( const TYPE *source )  
{ 
	return new TreeUtils<TYPE>();
}


template< class TYPE >
const TreeUtilsInterface *MakeTPStatic( const Scaffold<TYPE> *source )  
{ 
	return new TreeUtils<TYPE>();
}





#endif /* GENERICS_HPP */
