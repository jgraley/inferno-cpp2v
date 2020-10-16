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

// Shared pointer wrapper with OO support
namespace OOStd {

//    
// This is the interface for SharedPtr<>. It may be used like shared_ptr, with 
// template parameter VALUE_TYPE set to the pointed-to type. Various
// benefits accrue, including:
// - SerialNumber is used for inequality comparisons, for repeatability
// - Tracing support
// - Can obtain an architype object for the type of an object
//
template<typename VALUE_TYPE>
struct SharedPtr;

// An interface for our SharedPtr object. This interface works regardless of pointed-to
// type; it also masquerades as a SharedPtr to the VALUE_INTERFACE type, which should be the
// base class of the pointed-to things.
struct SharedPtrInterface : virtual Itemiser::Element, public Traceable
{
    // Convert to and from shared_ptr<VALUE_INTERFACE> and SharedPtr<VALUE_INTERFACE>
	virtual operator shared_ptr<Node>() const = 0;
	virtual operator SharedPtr<Node>() const = 0;

    virtual operator bool() const = 0; // for testing against nullptr
    virtual Node *get() const = 0; // As per shared_ptr<>, ie gets the actual C pointer
    virtual Node &operator *() const = 0; 
    virtual SharedPtrInterface &operator=( const SharedPtrInterface &o )
    {
    	(void)Itemiser::Element::operator=( o ); 
    	(void)Traceable::operator=( o );
    	return *this;
    }
    virtual SharedPtr<Node> MakeValueArchitype() const = 0; // construct an object of the VALUE_TYPE type (NOT a clone)
    virtual string GetTrace() const
    {
        return string("&") + operator*().GetTrace();
    }  
};

template<typename VALUE_TYPE>
struct SharedPtr : virtual SharedPtrInterface, shared_ptr<VALUE_TYPE>
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
    inline SharedPtr( const SharedPtr<OTHER> &o ) :
        shared_ptr<VALUE_TYPE>( (shared_ptr<OTHER>)(o) )
    {
    }

    virtual operator shared_ptr<Node>() const
    {
        const shared_ptr<VALUE_TYPE> p = (const shared_ptr<VALUE_TYPE>)*this;
        return p;
    }

	virtual operator SharedPtr<Node>() const
	{
        const shared_ptr<VALUE_TYPE> p1 = *(const shared_ptr<VALUE_TYPE> *)this;
        return SharedPtr<Node>( p1 );
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

    virtual SharedPtr &operator=( const SharedPtrInterface &n )
    {
    	(void)SharedPtr::operator=( shared_ptr<Node>(n) );
    	return *this;
    }

    virtual SharedPtr &operator=( const shared_ptr<Node> &n )
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
    inline SharedPtr &operator=( const shared_ptr<OTHER> &n )
    {
    	(void)shared_ptr<VALUE_TYPE>::operator=( (n) );
    	return *this;
    }

    virtual operator bool() const
    {
    	return !!*(const shared_ptr<VALUE_TYPE> *)this;
    }

	static inline SharedPtr<VALUE_TYPE>
	    DynamicCast( const SharedPtrInterface &g )
	{
		if( g )
		{
			shared_ptr<VALUE_TYPE> v = dynamic_pointer_cast<VALUE_TYPE>(shared_ptr<Node>(g));
			return SharedPtr<VALUE_TYPE>(v);
		}
		else
		{
			return SharedPtr<VALUE_TYPE>();
		}
	}
	// For when OOStd itself needs to dyncast, as opposed to the user asking for it.
	static inline SharedPtr<VALUE_TYPE>
	    InferredDynamicCast( const SharedPtrInterface &g )
	{
		if( g )
		{
			shared_ptr<VALUE_TYPE> v = dynamic_pointer_cast<VALUE_TYPE>(shared_ptr<Node>(g));
			ASSERT( v )("OOStd inferred dynamic cast has failed: from ")(*g)
			           (" to type ")(Traceable::CPPFilt( typeid( VALUE_TYPE ).name() ))("\n");
			return SharedPtr<VALUE_TYPE>(v);
		}
		else
		{
		    // Null came in, null goes out.
			return SharedPtr<VALUE_TYPE>();
		}
	}
	virtual SharedPtr<Node> MakeValueArchitype() const
	{
        return new VALUE_TYPE; // means VALUE_TYPE must be constructable
    }

    //inline bool operator<( const SharedPtr<VALUE_INTERFACE, SUB_BASE, VALUE_INTERFACE> &other )
    //{
    //    return SerialNumber::operator<(other);
    //}    
    //string GetAddr() const
    //{
    //    return SerialNumber::GetAddr(); // avoiding the need for virtual inheritance
   // }
};

}; // namespace


// TODO optimise SharedPtr, it seems to be somewhat slower than shared_ptr!!!
typedef OOStd::SharedPtrInterface TreePtrInterface;
#define TreePtr OOStd::SharedPtr

/*template<typename VALUE_TYPE>
class TreePtr : public OOStd::SharedPtr<VALUE_TYPE>
{
    typedef OOStd::SharedPtr<VALUE_TYPE> SPType; // just to save typing!
public:
	inline TreePtr() : SPType() {}
	inline TreePtr( VALUE_TYPE *o ) : SPType(o) {}
	inline TreePtr( const TreePtrInterface &g ) : SPType(g) {}
    inline operator TreePtr<Node>() const { return SPType::operator OOStd::SharedPtr<Node>(); }
	inline TreePtr( const SPType &g ) : SPType(g) {}
	template< typename OTHER >
	inline TreePtr( const shared_ptr<OTHER> &o ) : SPType(o) {}
	template< typename OTHER >
	inline TreePtr( const TreePtr<OTHER> &o ) : SPType(o) {}
	static inline TreePtr<VALUE_TYPE> DynamicCast( const TreePtrInterface &g )
	{
		return SPType::DynamicCast(g);
	}
	virtual OOStd::SharedPtr<Node> MakeValueArchitype() const
    {
        return new VALUE_TYPE; // means VALUE_TYPE must be constructable
    }
private:    
};
*/

// Similar signature to boost shared_ptr operator==, and we restrict the pointers
// to having the same subbase and base target
template< typename X, typename Y >
inline bool operator==( const TreePtr<X> &x,
		                const TreePtr<Y> &y)
{
	return operator==( (const shared_ptr<X> &)x, (const shared_ptr<Y> &)y );
}

// Similar signature to boost shared_ptr operator==, and we restrict the pointers
// to having the same subbase and base target
inline bool operator==( const TreePtrInterface &x,
		                const TreePtrInterface &y)
{
	return x.get() == y.get();
}

template< typename X, typename Y >
inline bool operator!=( const TreePtr<X> &x,
		                const TreePtr<Y> &y)
{
	return operator!=( (const shared_ptr<X> &)x, (const shared_ptr<Y> &)y );
}





#endif /* SHARED_PTR_HPP */
