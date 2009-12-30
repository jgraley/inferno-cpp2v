#ifndef TREE_GENERICS_HPP
#define TREE_GENERICS_HPP

#include "common/refcount.hpp"
#include "common/containers.hpp"
#include <deque>
#include <set>
#include <iterator>
#include "itemise.hpp"
#include "type_info.hpp"
#include "clone.hpp"
#include "common/magic.hpp"
#include "match.hpp"

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

struct GenericSharedPtr : Itemiser::Element, Traceable
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
                TRACE( );
            ASSERT( pe )
                  ("Tried to Set() wrong type of node via GenericSharedPtr\nType was ")((string)*n)("; I am ")(typeid(ELEMENT).name());
            *(shared_ptr<ELEMENT> *)this = pe;        
        }
    }

    virtual operator bool() const
    {
    	return !!*(const shared_ptr<ELEMENT> *)this;
    }

    SharedPtr() {}
    SharedPtr( ELEMENT *o ) : 
        shared_ptr<ELEMENT>( o )
    {
    }
    template< typename OTHER >
    SharedPtr( const shared_ptr<OTHER> &o ) : 
        shared_ptr<ELEMENT>( dynamic_pointer_cast<ELEMENT>(o) )
    {
        // TODO support NULL pointers as input!
    	if( o )
    	    ASSERT( *this )("Cannot convert shared_ptr<")(typeid(OTHER).name())("> to SharedPtr<")(typeid(ELEMENT).name())(">");
    }
    SharedPtr( const GenericSharedPtr &g ) :
    	shared_ptr<ELEMENT>( dynamic_pointer_cast<ELEMENT>(g.GetNodePtr()) )
    {
        // TODO support NULL pointers as input!
    	if( g )
    	    ASSERT( *this )("Cannot convert GenericSharedPtr that points to a ")((string)*(g.GetNodePtr()))(" to SharedPtr<")(typeid(ELEMENT).name())(">");
    }
	virtual operator string() const
	{
        return CPPFilt( typeid( ELEMENT ).name() );
	}
};           

// Inferno tree containers

typedef STLContainerBase<Itemiser::Element, GenericSharedPtr> GenericContainer;

typedef PointIterator<Itemiser::Element, GenericSharedPtr> GenericPointIterator;
typedef CountingIterator<Itemiser::Element, GenericSharedPtr> GenericCountingIterator;

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
	Sequence() {}
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
	virtual operator string() const
	{
        return CPPFilt( typeid( ELEMENT ).name() );
	}
	Sequence( const Sequence<Node> &cns )
	{
		// TODO support const_interator properly and get rid of this const_cast
		Sequence<Node> *ns = const_cast< Sequence<Node> * >( &cns );
		for( Sequence<Node>::iterator i=ns->begin();
		     i != ns->end();
		     ++i )
		{
            typename RawSequence::value_type sx(*i);
		    RawSequence::push_back( sx );
		}
	}
	Sequence operator=( const SharedPtr<Node> &nx )
	{
        typename RawSequence::value_type sx(nx);
        RawSequence::clear();
        RawSequence::push_back( sx );
        return *this;
	}
};

struct GenericCollection : virtual GenericContainer
{
	// TOOD for these to work in practice, may need to make them more like
	// push_back() in Sequence<>
	virtual void insert( const GenericSharedPtr &gx ) = 0;
	virtual void insert( const shared_ptr<Node> &gx ) = 0;
	virtual int erase( const GenericSharedPtr &gx ) = 0;
	virtual int erase( const shared_ptr<Node> &gx ) = 0;
	virtual bool IsExist( const GenericSharedPtr &gx ) = 0;
};

template<typename ELEMENT>
struct Collection : GenericCollection, STLCollection< Itemiser::Element, GenericSharedPtr, set< SharedPtr<ELEMENT> > >
{
    inline Collection<ELEMENT>() {}
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
	virtual operator string() const
	{
        return CPPFilt( typeid( ELEMENT ).name() );
	}
    Collection( const Sequence<Node> &cns )
	{
		// TODO support const_interator properly and get rid of this const_cast
		Sequence<Node> *ns = const_cast< Sequence<Node> * >( &cns );
		for( Sequence<Node>::iterator i=ns->begin();
		     i != ns->end();
		     ++i )
		{
            typename RawCollection::value_type sx(*i);
            RawCollection::insert( sx );
		}
	}
    Collection operator=( const SharedPtr<Node> &nx )
	{
        typename RawCollection::value_type sx(nx);
        RawCollection::clear();
        RawCollection::insert( sx );
        return *this;
	}
};


template<class LELEMENT, class RELEMENT>
inline Sequence<Node> operator,( const SharedPtr<LELEMENT> &l, const SharedPtr<RELEMENT> &r )
{
    Sequence<Node> seq;
    seq.push_back( (const GenericSharedPtr &)l );
    seq.push_back( (const GenericSharedPtr &)r );
    return seq;
}

template<class RELEMENT>
inline Sequence<Node> operator,( const Sequence<Node> &l, const SharedPtr<RELEMENT> &r )
{
    Sequence<Node> seq = l;
    seq.push_back( (const GenericSharedPtr &)r );
    return seq;
}

template<typename ELEMENT>
struct SharedNew : SharedPtr<ELEMENT>
{
	SharedNew() : SharedPtr<ELEMENT>( new ELEMENT )
	{
	}
};



//////////////////////////// Node Model ////////////////////////////

#define FINAL_FUNCTION(F) virtual bool IsFinal() { return (F); }

// Mix together the bounce classes for the benefit of the tree
// TODO figure out how to enforce finality in NODE_FUNCTIONS_FINAL
#define NODE_FUNCTIONS ITEMISE_FUNCTION MATCHER_FUNCTION CLONE_FUNCTION FINAL_FUNCTION(false)
#define NODE_FUNCTIONS_FINAL ITEMISE_FUNCTION MATCHER_FUNCTION CLONE_FUNCTION FINAL_FUNCTION(true)
struct NodeBases : Magic,
                   Traceable,
                   Matcher,
                   Itemiser,
                   Cloner
{
};

// Base class for all tree nodes and nodes in search/replace
// patterns etc. Convention is to use "struct" for derived
// node classes so that everything is public (inferno tree nodes
// are more like records in a database, they have only minimal
// functionality). Also, all derived structs should contain the
// NODE_FUNCTIONS macro which expands to a few virtual functions
// required for common ("bounced") functionality. Where multiple
// inheritance diamonds arise, Node should be derived virtually
// (we always want the set-restricting model of inheritance in
// the inferno tree node hierarchy).
struct Node : NodeBases
{
    NODE_FUNCTIONS

    virtual ~Node(){}  // be a virtual hierarchy
    // Node must be inherited virtually, to allow MI diamonds
    // without making Node ambiguous
};

extern void GenericsTest();

#endif
