#ifndef GENERICS_HPP
#define GENERICS_HPP

#include "common/refcount.hpp"
#include <deque>
#include <set>
#include <iterator>
#include "itemise_members.hpp"
#include "type_info.hpp"
#include "clone.hpp"
#include "common/magic.hpp"

// TODO refactor:
// Seperate this file into the "pure" generics (basically GenericContainer and Container templated
// on the base member type) and the "inferno" generics (everything else, instancing the above templates
// with GenericSharedPtr). Former goes in common/generics.hpp, latter goes in tree/containers.hpp

struct Node;

struct GenericSharedPtr : Itemiser::Element
{
    virtual shared_ptr<Node> Get() const = 0;
    virtual void Set( shared_ptr<Node> n ) = 0;
};

struct GenericContainer : virtual Itemiser::Element
{
	struct iterator_base
	{
		// TODO const iterator and const versions of begin(), end()
		virtual shared_ptr<iterator_base> Clone() const = 0; // Make another copy of the present iterator
		virtual iterator_base &operator++() = 0;
		virtual GenericSharedPtr &operator*() = 0;
		virtual bool operator==( const iterator_base &ib ) = 0;
	};
	class iterator
	{
	public:
		typedef forward_iterator_tag iterator_category;
		typedef GenericSharedPtr value_type;
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
    virtual const iterator_base &begin() = 0;
    virtual const iterator_base &end() = 0;
    virtual int size() const = 0;
    virtual void clear() = 0;
};


struct GenericSequence : virtual GenericContainer
{
    virtual GenericSharedPtr &operator[]( int i ) = 0;
};

struct GenericCollection : virtual GenericContainer
{
	virtual void insert( const GenericSharedPtr &gx ) = 0;
	virtual void erase( const GenericSharedPtr &gx ) = 0;
	virtual bool IsExist( const GenericSharedPtr &gx ) = 0;
};

template<typename ELEMENT>
struct SharedPtr : GenericSharedPtr, shared_ptr<ELEMENT> 
{
    virtual shared_ptr<Node> Get() const
    {
        const shared_ptr<ELEMENT> *p = (const shared_ptr<ELEMENT> *)this;
        ASSERT(p);
        shared_ptr<Node> n = (shared_ptr<Node>)*p;
        return n;
    }
    virtual void Set( shared_ptr<Node> n )
    {
        if( !n )
        {
            *(shared_ptr<ELEMENT> *)this = shared_ptr<ELEMENT>(); // handle NULL explicitly since dyn cast uses NULL to indicate wrong type   
        }
        else
        {
            shared_ptr<ELEMENT> pe = dynamic_pointer_cast<ELEMENT>(n);
            if( !pe )
                TRACE("Type was %s; I am %s\n", TypeInfo(n).name().c_str(), typeid(ELEMENT).name() );
            ASSERT( pe && "Tried to Set() wrong type of node via GenericSharedPtr" );
            *(shared_ptr<ELEMENT> *)this = pe;        
        }
    }
    template< typename OTHER >
    SharedPtr( const shared_ptr<OTHER> &o ) : 
        shared_ptr<ELEMENT>(o) {}
    SharedPtr() {}
    SharedPtr( const GenericSharedPtr &g ) :
    	shared_ptr<ELEMENT>( dynamic_pointer_cast<ELEMENT>(g.Get()) )
    {
    	ASSERT( *this )("Tried to convert GenericSharedPtr to wrong sort of SharedPtr<>");
    }
};           

template<typename STLCONTAINER>
struct Container : virtual GenericContainer, STLCONTAINER
{
	typedef STLCONTAINER STLContainer;

	struct iterator : public STLContainer::iterator, public GenericContainer::iterator_base
	{
		virtual shared_ptr<iterator_base> Clone() const
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

		virtual bool operator==( const iterator_base &ib )
		{
			if( const iterator *pi = dynamic_cast<const iterator *>(&ib) )
				return *pi == *this;
			else
				return false; // comparing iterators of different types; must be from different containers
		}
	};

	typedef iterator const_iterator;

    // Covarient style only works with refs and pointers, so force begin/end to return refs safely
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

template<typename ELEMENT>
struct Sequence : virtual GenericSequence, virtual Container< deque< SharedPtr<ELEMENT> > >
{
	typedef deque< SharedPtr<ELEMENT> > STLSequence;
    virtual typename STLSequence::value_type &operator[]( int i )
    {
    	return STLSequence::operator[](i);
    }
};

template<typename ELEMENT>
struct Collection : GenericCollection, Container< set< SharedPtr<ELEMENT> > >
{
	typedef set< SharedPtr<ELEMENT> > STLCollection;
	virtual void insert( const GenericSharedPtr &gx )
	{
		typename STLCollection::value_type sx(gx);
		STLCollection::insert( sx );
	}
	virtual void erase( const GenericSharedPtr &gx )
	{
		typename STLCollection::value_type sx(gx);
		STLCollection::erase( sx );
	}
	virtual bool IsExist( const GenericSharedPtr &gx )
	{
		typename STLCollection::value_type sx(gx);
		typename STLCollection::iterator it = STLCollection::find( sx );
		return it != STLCollection::end();
	}
};

// Mix together the bounce classes for the benefit of the tree
#define NODE_FUNCTIONS ITEMISE_FUNCTION TYPE_INFO_FUNCTION CLONE_FUNCTION
struct NodeBases : Magic,
                   TypeInfo::TypeBase,
                   Itemiser,
                   Cloner
{
};

extern void GenericsTest();


#endif
