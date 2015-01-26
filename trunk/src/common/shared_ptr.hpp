/*
 * shared_ptr.hpp
 *
 *  Created on: 9 Jun 2010
 *      Author: jgraley
 */

#ifndef SHARED_PTR_HPP
#define SHARED_PTR_HPP

#include "common/common.hpp"
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

// Shared pointer wrapper with OO support

namespace OOStd {

template<typename SUB_BASE, typename VALUE_INTERFACE, typename VALUE_TYPE>
struct SharedPtr;

// An interface for our SharedPtr object. This interface works regardless of pointed-to
// type; it also masquerades as a SharedPtr to the VALUE_INTERFACE type, which should be the
// base class of the pointed-to things.
template<typename SUB_BASE, typename VALUE_INTERFACE>
struct SharedPtrInterface : virtual SUB_BASE, public Traceable
{
    // Convert to and from shared_ptr<VALUE_INTERFACE> and SharedPtr<VALUE_INTERFACE>
	virtual operator shared_ptr<VALUE_INTERFACE>() const = 0;
	virtual operator SharedPtr<SUB_BASE, VALUE_INTERFACE, VALUE_INTERFACE>() const = 0;

    virtual operator bool() const = 0; // for testing against NULL
    virtual VALUE_INTERFACE *get() const = 0; // As per shared_ptr<>, ie gets the actual C pointer
    virtual VALUE_INTERFACE &operator *() const = 0; 
    virtual SharedPtrInterface &operator=( const SharedPtrInterface &o )
    {
    	(void)SUB_BASE::operator=( o ); // vital for itemiser!
    	(void)Traceable::operator=( o );
    	return *this;
    }
    virtual SharedPtr<SUB_BASE, VALUE_INTERFACE, VALUE_INTERFACE> MakeValueArchitype() const = 0; // construct an object of the VALUE_TYPE type (NOT a clone 
                                                                                        // of the object we're pointing to) 
};

template<typename SUB_BASE, typename VALUE_INTERFACE, typename VALUE_TYPE>
struct SharedPtr : virtual SharedPtrInterface<SUB_BASE, VALUE_INTERFACE>, shared_ptr<VALUE_TYPE>,
                   SerialNumber
{
    inline SharedPtr() {}

    inline SharedPtr( VALUE_TYPE *o ) :
        shared_ptr<VALUE_TYPE>( o )
    {
    }

    template< typename OTHER >
    inline SharedPtr( const shared_ptr<OTHER> &o ) :
        shared_ptr<VALUE_TYPE>( o )
    {
    }

    template< typename OTHER >
    inline SharedPtr( const SharedPtr<SUB_BASE, VALUE_INTERFACE, OTHER> &o ) :
        shared_ptr<VALUE_TYPE>( (shared_ptr<OTHER>)(o) )
    {
    }

    virtual operator shared_ptr<VALUE_INTERFACE>() const
    {
        const shared_ptr<VALUE_TYPE> p = (const shared_ptr<VALUE_TYPE>)*this;
        return p;
    }

	virtual operator SharedPtr<SUB_BASE, VALUE_INTERFACE, VALUE_INTERFACE>() const
	{
        const shared_ptr<VALUE_TYPE> p1 = *(const shared_ptr<VALUE_TYPE> *)this;
        return SharedPtr<SUB_BASE, VALUE_INTERFACE, VALUE_INTERFACE>( p1 );
	}


    virtual VALUE_TYPE *get() const 
    {
    	VALUE_TYPE *e = shared_ptr<VALUE_TYPE>::get();
    	//TRACE("sp::get() returns %p\n", e );
    	return e;
    }

    virtual VALUE_TYPE &operator *() const 
    {
    	return shared_ptr<VALUE_TYPE>::operator *();
    }

    virtual SharedPtr &operator=( shared_ptr<VALUE_INTERFACE> n )
    {   
        if( n )
        {
            shared_ptr<VALUE_TYPE> p = dynamic_pointer_cast<VALUE_TYPE>(shared_ptr<VALUE_INTERFACE>(n));
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
    inline SharedPtr &operator=( SharedPtr<SUB_BASE, VALUE_INTERFACE, OTHER> n )
    {
    	(void)operator=( shared_ptr<OTHER>(n) );
    	return *this;
    }

    virtual SharedPtr &operator=( const SharedPtrInterface<SUB_BASE, VALUE_INTERFACE> &n )
    {
    	(void)operator=( shared_ptr<VALUE_INTERFACE>(n) );
    	return *this;
    }

    virtual operator bool() const
    {
    	return !!*(const shared_ptr<VALUE_TYPE> *)this;
    }

	static inline SharedPtr<SUB_BASE, VALUE_INTERFACE, VALUE_TYPE>
	    DynamicCast( const SharedPtrInterface<SUB_BASE, VALUE_INTERFACE> &g )
	{
		if( g )
		{
			shared_ptr<VALUE_TYPE> v = dynamic_pointer_cast<VALUE_TYPE>(shared_ptr<VALUE_INTERFACE>(g));
			return SharedPtr<SUB_BASE, VALUE_INTERFACE, VALUE_TYPE>(v);
		}
		else
		{
			return SharedPtr<SUB_BASE, VALUE_INTERFACE, VALUE_TYPE>();
		}
	}
	// For when OOStd itself needs to dyncast, as opposed to the user asking for it.
	static inline SharedPtr<SUB_BASE, VALUE_INTERFACE, VALUE_TYPE>
	    InferredDynamicCast( const SharedPtrInterface<SUB_BASE, VALUE_INTERFACE> &g )
	{
		if( g )
		{
			shared_ptr<VALUE_TYPE> v = dynamic_pointer_cast<VALUE_TYPE>(shared_ptr<VALUE_INTERFACE>(g));
			ASSERT( v )("OOStd inferred dynamic cast has failed: from ")(*g)
			           (" to type ")(Traceable::CPPFilt( typeid( VALUE_TYPE ).name() ))("\n");
			return SharedPtr<SUB_BASE, VALUE_INTERFACE, VALUE_TYPE>(v);
		}
		else
		{
		    // Null came in, null goes out.
			return SharedPtr<SUB_BASE, VALUE_INTERFACE, VALUE_TYPE>();
		}
	}
	virtual SharedPtr<SUB_BASE, VALUE_INTERFACE, VALUE_INTERFACE> MakeValueArchitype() const
	{
        ASSERTFAIL("MakeValueArchitype() not implemented for this SharedPtr\n");
    }

    inline bool operator<( const SharedPtr<SUB_BASE, VALUE_INTERFACE, VALUE_INTERFACE> &other )
    {
        return GetSerialNumber() < other.GetSerialNumber();
    }    
};

// Similar signature to boost shared_ptr operator==, and we restrict the pointers
// to having the same subbase and base target
template< typename SUB_BASE, typename VALUE_INTERFACE, typename X, typename Y >
inline bool operator==( const SharedPtr<SUB_BASE, VALUE_INTERFACE, X> &x,
		                const SharedPtr<SUB_BASE, VALUE_INTERFACE, Y> &y)
{
	return operator==( (const shared_ptr<X> &)x, (const shared_ptr<Y> &)y );
}

// Similar signature to boost shared_ptr operator==, and we restrict the pointers
// to having the same subbase and base target
template< typename SUB_BASE, typename VALUE_INTERFACE >
inline bool operator==( const SharedPtrInterface<SUB_BASE, VALUE_INTERFACE> &x,
		                const SharedPtrInterface<SUB_BASE, VALUE_INTERFACE> &y)
{
	return x.get() == y.get();
}

template< typename SUB_BASE, typename VALUE_INTERFACE, typename X, typename Y >
inline bool operator!=( const SharedPtr<SUB_BASE, VALUE_INTERFACE, X> &x,
		                const SharedPtr<SUB_BASE, VALUE_INTERFACE, Y> &y)
{
	return operator!=( (const shared_ptr<X> &)x, (const shared_ptr<Y> &)y );
}

}; // namespace

#endif /* SHARED_PTR_HPP */
