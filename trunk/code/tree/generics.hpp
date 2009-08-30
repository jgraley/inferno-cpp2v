#ifndef TREE_GENERICS_HPP
#define TREE_GENERICS_HPP

#include "common/refcount.hpp"
#include "common/containers.hpp"
#include <deque>
#include <set>
#include <iterator>
#include "itemise_members.hpp"
#include "type_info.hpp"
#include "clone.hpp"
#include "common/magic.hpp"

// Shared pointer wrapper with generic support

struct Node;

struct GenericSharedPtr : Itemiser::Element
{
    virtual shared_ptr<Node> GetNodePtr() const = 0;
    virtual void SetNodePtr( shared_ptr<Node> n ) = 0;
    virtual operator bool() const = 0; // for testing against NULL
    virtual Node *get() = 0; // As per shared_ptr<>, ie gets the actual C pointer

    // Nice operators, make less typing, make you happy. Yay!
    operator shared_ptr<Node>() const
    {
    	return GetNodePtr();
    }

    GenericSharedPtr &operator=( shared_ptr<Node> n )
    {
    	SetNodePtr( n );
    	return *this;
    }
};

template<typename ELEMENT>
struct SharedPtr : GenericSharedPtr, shared_ptr<ELEMENT> 
{
    virtual shared_ptr<Node> GetNodePtr() const
    {
        const shared_ptr<ELEMENT> *p = (const shared_ptr<ELEMENT> *)this;
        ASSERT(p);
        shared_ptr<Node> n = (shared_ptr<Node>)*p;
        return n;
    }

    virtual ELEMENT *get()
    {
    	return shared_ptr<ELEMENT>::get();
    }

    virtual void SetNodePtr( shared_ptr<Node> n )
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

    virtual operator bool() const
    {
    	return !!*(const shared_ptr<ELEMENT> *)this;
    }

    template< typename OTHER >
    SharedPtr( const shared_ptr<OTHER> &o ) : 
        shared_ptr<ELEMENT>(o) {}
    SharedPtr() {}
    SharedPtr( const GenericSharedPtr &g ) :
    	shared_ptr<ELEMENT>( dynamic_pointer_cast<ELEMENT>(g.GetNodePtr()) )
    {
    	ASSERT( *this )("Tried to convert GenericSharedPtr to wrong sort of SharedPtr<>");
    }
};           

// Inferno tree containers

typedef STLContainerBase<Itemiser::Element, GenericSharedPtr> GenericContainer;

struct GenericSequence : virtual GenericContainer
{
    virtual GenericSharedPtr &operator[]( int i ) = 0;
};

template<typename ELEMENT>
struct Sequence : virtual GenericSequence, virtual Container< Itemiser::Element, GenericSharedPtr, deque< SharedPtr<ELEMENT> > >
{
	typedef deque< SharedPtr<ELEMENT> > RawSequence;
    virtual typename RawSequence::value_type &operator[]( int i )
    {
    	return RawSequence::operator[](i);
    }
};

struct GenericCollection : virtual GenericContainer
{
	virtual void insert( const GenericSharedPtr &gx ) = 0;
	virtual void erase( const GenericSharedPtr &gx ) = 0;
	virtual bool IsExist( const GenericSharedPtr &gx ) = 0;
};

template<typename ELEMENT>
struct Collection : GenericCollection, Container< Itemiser::Element, GenericSharedPtr, set< SharedPtr<ELEMENT> > >
{
	typedef set< SharedPtr<ELEMENT> > RawCollection;
	virtual void insert( const GenericSharedPtr &gx )
	{
		typename RawCollection::value_type sx(gx);
		RawCollection::insert( sx );
	}
	virtual void erase( const GenericSharedPtr &gx )
	{
		typename RawCollection::value_type sx(gx);
		RawCollection::erase( sx );
	}
	virtual bool IsExist( const GenericSharedPtr &gx )
	{
		typename RawCollection::value_type sx(gx);
		typename RawCollection::iterator it = RawCollection::find( sx );
		return it != RawCollection::end();
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
