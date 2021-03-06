/*
 * tree_ptr.hpp
 *
 *  Created on: 9 Jun 2010
 *      Author: jgraley
 */

#ifndef TREE_PTR_HPP
#define TREE_PTR_HPP

#include "common/common.hpp"
#include "common/magic.hpp"
#include "itemise.hpp"
#include "node.hpp"

// Covariant nullptr pointer bug
//
// JSG: There's an unfortunate bug in GCC 3.4.4 on cygwin whereby a covariant return thunk
// for a pointer goes wrong when the pointer is nullptr. We can end up dereferencing a nullptr (or offset-from-nullptr)
// pointer inside the thunk itself which is opaque code, not a lot of fun overall.
//
// It seems to be OK on GCC4 on Linux, and c++/20746 (http://gcc.gnu.org/bugzilla/show_bug.cgi?id=20746) seems to have a fix,
// but I think it only applies to GCC4 (4.0.2 and 4.1).
//
// So I've just hacked covariant returns to not be covariant whenever I get a problem (just returns same as
// base class, is this "isovariant"? No, "invariant")
//
// See #268

//    
// This is the interface for TreePtr<>. It may be used like shared_ptr, with 
// template parameter VALUE_TYPE set to the pointed-to type. Various
// benefits accrue, including:
// - SerialNumber is used for inequality comparisons, for repeatability
// - Tracing support
// - Can obtain an architype object for the type of an object
//
template<typename VALUE_TYPE>
struct TreePtr;

// An interface for our TreePtr object. This interface works regardless of pointed-to
// type; it also masquerades as a TreePtr to the VALUE_INTERFACE type, which should be the
// base class of the pointed-to things.
struct TreePtrInterface : virtual Itemiser::Element, public Traceable
{
    // Convert to and from shared_ptr<VALUE_INTERFACE> and TreePtr<VALUE_INTERFACE>
	virtual explicit operator shared_ptr<Node>() const = 0; // TODO dangerous; see #201
	virtual explicit operator TreePtr<Node>() const = 0; // TODO dangerous; see #201

    virtual explicit operator bool() const = 0; // for testing against nullptr
    virtual Node *get() const = 0; // As per shared_ptr<>, ie gets the actual C pointer
    virtual Node &operator *() const = 0; 
    virtual TreePtrInterface &operator=( const TreePtrInterface &o )
    {
    	(void)Itemiser::Element::operator=( o ); 
    	(void)Traceable::operator=( o );
    	return *this;
    }
    virtual TreePtr<Node> MakeValueArchitype() const = 0; // construct an object of the VALUE_TYPE type (NOT a clone)
    virtual string GetTrace() const
    {
        return string("&") + operator*().GetTrace();
    }  
};

template<typename VALUE_TYPE>
struct TreePtr : virtual TreePtrInterface, shared_ptr<VALUE_TYPE>
{
    inline TreePtr() {}

    explicit inline TreePtr( VALUE_TYPE *o ) : // dangerous - make explicit
        shared_ptr<VALUE_TYPE>( o )
    {
    }

    inline TreePtr( nullptr_t o ) : // safe - leave implicit
        shared_ptr<VALUE_TYPE>( o )
    {
    }

    template< typename OTHER >
    explicit inline TreePtr( const shared_ptr<OTHER> &o ) :
        shared_ptr<VALUE_TYPE>( o )
    {
    }

    template< typename OTHER >
    inline TreePtr( const TreePtr<OTHER> &o ) :
        shared_ptr<VALUE_TYPE>( (shared_ptr<OTHER>)(o) )
    {
    }

    virtual operator shared_ptr<Node>() const
    {
        const shared_ptr<VALUE_TYPE> p = (const shared_ptr<VALUE_TYPE>)*this;
        return p;
    }

	virtual operator TreePtr<Node>() const
	{
        const shared_ptr<VALUE_TYPE> p1 = *(const shared_ptr<VALUE_TYPE> *)this;
        return TreePtr<Node>( p1 );
	}

    virtual VALUE_TYPE *get() const 
    {
    	VALUE_TYPE *e = shared_ptr<VALUE_TYPE>::get();
    	return e;
    }

    virtual VALUE_TYPE &operator *() const 
    {
    	return shared_ptr<VALUE_TYPE>::operator *();
    }

    virtual TreePtr &operator=( const TreePtrInterface &n )
    {
    	(void)TreePtr::operator=( shared_ptr<Node>(n) );
    	return *this;
    }

    virtual TreePtr &operator=( const shared_ptr<Node> &n )
    {   
        if( n )
        {
            shared_ptr<VALUE_TYPE> p = dynamic_pointer_cast<VALUE_TYPE>(n);
            ASSERT( p )("OOStd inferred dynamic cast has failed: from ")(*n)
			           (" to type ")(Traceable::CPPFilt( typeid( VALUE_TYPE ).name() ))("\n");
         	(void)shared_ptr<VALUE_TYPE>::operator=( p );
        }
        else
        {
            (void)shared_ptr<VALUE_TYPE>::operator=( shared_ptr<VALUE_TYPE>() );
        }
    	return *this;
    }

    template< typename OTHER >
    inline TreePtr &operator=( const shared_ptr<OTHER> &n )
    {
    	(void)shared_ptr<VALUE_TYPE>::operator=( (n) );
    	return *this;
    }

    virtual operator bool() const
    {
    	return !!*(const shared_ptr<VALUE_TYPE> *)this;
    }

	static inline TreePtr<VALUE_TYPE>
	    DynamicCast( const TreePtrInterface &g )
	{
		if( g )
		{
			shared_ptr<VALUE_TYPE> v = dynamic_pointer_cast<VALUE_TYPE>(shared_ptr<Node>(g));
			return TreePtr<VALUE_TYPE>(v);
		}
		else
		{
			return TreePtr<VALUE_TYPE>();
		}
	}
	// For when OOStd itself needs to dyncast, as opposed to the user asking for it.
	static inline TreePtr<VALUE_TYPE>
	    InferredDynamicCast( const TreePtrInterface &g )
	{
		if( g )
		{
			shared_ptr<VALUE_TYPE> v = dynamic_pointer_cast<VALUE_TYPE>(shared_ptr<Node>(g));
			ASSERT( v )("OOStd inferred dynamic cast has failed: from ")(*g)
			           (" to type ")(Traceable::CPPFilt( typeid( VALUE_TYPE ).name() ))("\n");
			return TreePtr<VALUE_TYPE>(v);
		}
		else
		{
		    // Null came in, null goes out.
			return TreePtr<VALUE_TYPE>();
		}
	}
	virtual TreePtr<Node> MakeValueArchitype() const
	{
        return TreePtr<Node>(new VALUE_TYPE); // means VALUE_TYPE must be constructable
    }

    //inline bool operator<( const TreePtr<VALUE_INTERFACE, SUB_BASE, VALUE_INTERFACE> &other )
    //{
    //    return SerialNumber::operator<(other);
    //}    
    //string GetSerialString() const
    //{
    //    return SerialNumber::GetSerialString(); // avoiding the need for virtual inheritance
   // }
};


template<typename VALUE_TYPE>
inline TreePtr<VALUE_TYPE> DynamicTreePtrCast( const TreePtrInterface &g )
{
    return TreePtr<VALUE_TYPE>::DynamicCast(g);
}


// Similar signature to boost shared_ptr operator==, and we restrict the pointers
// to having the same subbase and base target
template< typename X, typename Y >
inline bool operator==( const TreePtr<X> &x,
		                const TreePtr<Y> &y)
{
	return operator==( (const shared_ptr<X> &)x, (const shared_ptr<Y> &)y );
}

template< typename X, typename Y >
inline bool operator!=( const TreePtr<X> &x,
		                const TreePtr<Y> &y)
{
	return operator!=( (const shared_ptr<X> &)x, (const shared_ptr<Y> &)y );
}

// Similar signature to boost shared_ptr operator==, and we restrict the pointers
// to having the same subbase and base target
inline bool operator==( const TreePtrInterface &x,
		                const TreePtrInterface &y)
{
	return x.get() == y.get();
}

inline bool operator!=( const TreePtrInterface &x,
		                const TreePtrInterface &y)
{
	return x.get() != y.get();
}

inline bool operator<( const TreePtrInterface &x,
		               const TreePtrInterface &y)
{
	return x.get() < y.get();
}


// Handy typing saver for creating objects and SharedPtrs to them.
// MakeTreePtr<X> may be constructed in the same way as X, but will then
// masquerade as a TreePtr<X> where the pointed-to X has been allocated
// using new. Similar to Boost's make_shared<>() except that being an object
// with a constructor, rather than a free function, it may be used in a
// declaration as well as in a function-like way. So both of the following
// are OK:
// existing_shared_ptr = MakeTreePtr<X>(10); // as per Boost: construction of temporary looks like function call
// MakeTreePtr<X> new_shared_ptr(10); // new Inferno form: new_shared_ptr may now be used like a TreePtr<X>
// But of course with the auto keyword, the benefit is smaller than the cognitive burden of introducing a new type 
template<typename VALUE_TYPE>
struct MakeTreePtr : TreePtr<VALUE_TYPE>
{
	MakeTreePtr() : TreePtr<VALUE_TYPE>( new VALUE_TYPE ) {}
	template<typename CP0>
	MakeTreePtr(const CP0 &cp0) : TreePtr<VALUE_TYPE>( new VALUE_TYPE(cp0) ) {}
	template<typename CP0, typename CP1>
	MakeTreePtr(const CP0 &cp0, const CP1 &cp1) : TreePtr<VALUE_TYPE>( new VALUE_TYPE(cp0, cp1) ) {}
	template<typename CP0, typename CP1, typename CP2>
	MakeTreePtr(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2) : TreePtr<VALUE_TYPE>( new VALUE_TYPE(cp0, cp1, cp2) ) {}
	// Add more params as needed...
};

template<>
struct MakeTreePtr<Node> : TreePtr<Node>
{
	MakeTreePtr() : TreePtr<Node>( new Node ) {}
	template<typename CP0>
	MakeTreePtr(const CP0 &cp0) : TreePtr<Node>( new Node(cp0) ) {}
	template<typename CP0, typename CP1>
	MakeTreePtr(const CP0 &cp0, const CP1 &cp1) : TreePtr<Node>( new Node(cp0, cp1) ) {}
	template<typename CP0, typename CP1, typename CP2>
	MakeTreePtr(const CP0 &cp0, const CP1 &cp1, const CP2 &cp2) : TreePtr<Node>( new Node(cp0, cp1, cp2) ) {}
	// Add more params as needed...
};

#endif /* SHARED_PTR_HPP */
