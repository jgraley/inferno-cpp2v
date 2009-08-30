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

struct Node;

struct GenericSharedPtr : Itemiser::Element
{
    virtual shared_ptr<Node> Get() const = 0;
    virtual void Set( shared_ptr<Node> n ) = 0;
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

typedef STLContainerBase<Itemiser::Element, GenericSharedPtr> GenericContainer;


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
struct Sequence : virtual GenericSequence, virtual Container< Itemiser::Element, GenericSharedPtr, deque< SharedPtr<ELEMENT> > >
{
	typedef deque< SharedPtr<ELEMENT> > STLSequence;
    virtual typename STLSequence::value_type &operator[]( int i )
    {
    	return STLSequence::operator[](i);
    }
};

template<typename ELEMENT>
struct Collection : GenericCollection, Container< Itemiser::Element, GenericSharedPtr, set< SharedPtr<ELEMENT> > >
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
