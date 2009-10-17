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

// Covariant NULL pointer bug
//
// JSG: There's an unfortunate bug in GCC 3.4.4 on cygwin whereby a covariant return thunk
// for a pointer goes wrong when the pointer is NULL. We can end up dereferencing a NULL (or offset-from-NULL) 
// pointer inside the thunk itself which is opaque code, not a lot of fun overall.
//
// It seems to be OK on GCC4 on Linux, and c++/20746 (http://gcc.gnu.org/bugzilla/show_bug.cgi?id=20746) seems to have a fix, 
// but I think it only applies to GCC4 (4.0.2 and 4.1). 
//
// So I've just hacked covariant returns to not be covariant whenever I get a problem (just returns same as 
// base class, is this "isovariant"?)
//

// Shared pointer wrapper with generic support

struct Node;

struct GenericSharedPtr : Itemiser::Element
{
    virtual shared_ptr<Node> GetNodePtr() const = 0;
    virtual void SetNodePtr( shared_ptr<Node> n ) = 0;
    virtual operator bool() const = 0; // for testing against NULL
    virtual Node *get() const = 0; // As per shared_ptr<>, ie gets the actual C pointer

    // Nice operators, make less typing, make you happy. Yay!
    operator shared_ptr<Node>() const
    {
    	return GetNodePtr();
    }

    GenericSharedPtr &operator=( const GenericSharedPtr &n )
    {
    	*(Itemiser::Element *)this = *(Itemiser::Element *)&n; // so assign gets to itemiser
    	SetNodePtr( n.GetNodePtr() );
    	return *this;
    }
    GenericSharedPtr &operator=( shared_ptr<Node> n )
    {
    	TRACE();
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

    virtual Node *get() const // TODO should return ELEMENT, hacked due to covariant NULL pointer bug, see comment at top of file
    {
    	ELEMENT *e = shared_ptr<ELEMENT>::get();
    	//TRACE("sp::get() returns %p\n", e );
    	return e;
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

    SharedPtr() {}
    template< typename OTHER >
    SharedPtr( const shared_ptr<OTHER> &o ) : 
        shared_ptr<ELEMENT>( dynamic_pointer_cast<ELEMENT>(o) )
    {
        // TODO support NULL pointers as input!
    	if( o )
    	    ASSERT( *this )("Tried to convert shared_ptr<> to wrong sort of SharedPtr<>");
    }
    SharedPtr( const GenericSharedPtr &g ) :
    	shared_ptr<ELEMENT>( dynamic_pointer_cast<ELEMENT>(g.GetNodePtr()) )
    {
        // TODO support NULL pointers as input!
    	if( g )
    	    ASSERT( *this )("Tried to convert GenericSharedPtr to wrong sort of SharedPtr<>");
    }
};           

// Inferno tree containers

typedef STLContainerBase<Itemiser::Element, GenericSharedPtr> GenericContainer;

typedef PointIterator<Itemiser::Element, GenericSharedPtr> GenericPointIterator;

struct GenericSequence : virtual GenericContainer
{
    virtual GenericSharedPtr &operator[]( int i ) = 0;
    virtual void push_back( const GenericSharedPtr &gx ) = 0;
	virtual void push_back( const shared_ptr<Node> &gx ) = 0;
};


template<typename ELEMENT>
struct Sequence : virtual GenericSequence, virtual STLSequence< Itemiser::Element, GenericSharedPtr, deque< SharedPtr<ELEMENT> > >
{
	typedef deque< SharedPtr<ELEMENT> > RawSequence;
    virtual typename RawSequence::value_type &operator[]( int i )
    {
    	return RawSequence::operator[](i);
    }
	virtual void push_back( const GenericSharedPtr &gx )
	{
		typename RawSequence::value_type sx(gx);
		RawSequence::push_back( sx );
	}
	virtual void push_back( const shared_ptr<Node> &gx )
	{
		typename RawSequence::value_type sx(gx);
		RawSequence::push_back( sx );
	}
};

struct GenericCollection : virtual GenericContainer
{
	// TOOD for these to work in practice, may need to make them more like
	// push_back() in Sequence<>
	virtual void insert( const GenericSharedPtr &gx ) = 0;
	virtual int erase( const GenericSharedPtr &gx ) = 0;
	virtual bool IsExist( const GenericSharedPtr &gx ) = 0;
};

template<typename ELEMENT>
struct Collection : GenericCollection, STLCollection< Itemiser::Element, GenericSharedPtr, set< SharedPtr<ELEMENT> > >
{
	typedef set< SharedPtr<ELEMENT> > RawCollection;
	virtual void insert( const GenericSharedPtr &gx )
	{
		typename RawCollection::value_type sx(gx);
		RawCollection::insert( sx );
	}
	virtual void insert( const shared_ptr<Node> &gx )
	{
		typename RawCollection::value_type sx(gx);
		RawCollection::insert( sx );
	}
	virtual int erase( const GenericSharedPtr &gx )
	{
		typename RawCollection::value_type sx(gx);
		return RawCollection::erase( sx );
	}
	virtual int erase( const shared_ptr<Node> &gx )
	{
		typename RawCollection::value_type sx(gx);
		return RawCollection::erase( sx );
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
