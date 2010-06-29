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

// An interface for our SharedPtr object. This interface works regardless of pointed-to
// type; it also masquerades as a SharedPtr to the VALUE_INTERFACE type, which should be the
// base class of the pointed-to things.
template<typename SUB_BASE, typename VALUE_INTERFACE>
struct SharedPtrInterface : virtual SUB_BASE, public Traceable
{
    // Convert to and from shared_ptr<VALUE_INTERFACE>
	virtual operator shared_ptr<VALUE_INTERFACE>() const = 0;

    virtual operator bool() const = 0; // for testing against NULL
    virtual VALUE_INTERFACE *get() const = 0; // As per shared_ptr<>, ie gets the actual C pointer
};


template<typename SUB_BASE, typename VALUE_INTERFACE, typename VALUE_TYPE>
struct SharedPtr : virtual SharedPtrInterface<SUB_BASE, VALUE_INTERFACE>, shared_ptr<VALUE_TYPE>
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

    inline SharedPtr( const SharedPtrInterface<SUB_BASE, VALUE_INTERFACE> &g ) :
    	shared_ptr<VALUE_TYPE>( dynamic_pointer_cast<VALUE_TYPE>(shared_ptr<VALUE_INTERFACE>(g)) )
    {
    	if( g )
    	    ASSERT( *this )("Cannot convert GenericSharedPtr that points to a ")((string)*(shared_ptr<VALUE_INTERFACE>(g)))(" to SharedPtr<")(typeid(VALUE_TYPE).name())(">");
    }

    virtual operator shared_ptr<VALUE_INTERFACE>() const
    {
        const shared_ptr<VALUE_TYPE> *p = (const shared_ptr<VALUE_TYPE> *)this;
        return *p;
    }

    virtual VALUE_INTERFACE *get() const // TODO should return VALUE_TYPE, hacked due to covariant NULL pointer bug, see comment at top of file
    {
    	VALUE_TYPE *e = shared_ptr<VALUE_TYPE>::get();
    	//TRACE("sp::get() returns %p\n", e );
    	return e;
    }

    virtual SharedPtr &operator=( shared_ptr<VALUE_INTERFACE> n )
    {
    	(void)operator=<VALUE_INTERFACE>( n );
    	return *this;
    }

    template< typename OTHER >
    inline SharedPtr &operator=( SharedPtr<SUB_BASE, VALUE_INTERFACE, OTHER> n )
    {
    	(void)operator=( shared_ptr<OTHER>(n) );
    	return *this;
    }

    inline SharedPtr &operator=( const SharedPtrInterface<SUB_BASE, VALUE_INTERFACE> &n )
    {
    	(void)operator=( shared_ptr<VALUE_INTERFACE>(n) );
    	return *this;
    }

    virtual operator bool() const
    {
    	return !!*(const shared_ptr<VALUE_TYPE> *)this;
    }
};

}

#endif /* SHARED_PTR_HPP */
