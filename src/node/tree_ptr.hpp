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

#define TREE_PTR_ORDERING_USE_SERIAL

//    
// This is the interface for TreePtr<>. It may be used like shared_ptr, with 
// template parameter VALUE_TYPE set to the pointed-to type. Various
// benefits accrue, including:
// - SerialNumber is used for inequality comparisons, for repeatability
// - Tracing support
// - Can obtain an archetype object for the type of an object
//
template<typename VALUE_TYPE>
struct TreePtr;

// An interface for our TreePtr object. This interface works regardless of pointed-to
// type; it also masquerades as a TreePtr to Node type, which should be the
// base class of the pointed-to things.
struct TreePtrInterface : virtual Itemiser::Element, public Traceable
{
    // Convert to and from shared_ptr<VALUE_INTERFACE> and TreePtr<VALUE_INTERFACE>
	virtual explicit operator shared_ptr<Node>() const = 0; 
	virtual explicit operator TreePtr<Node>() const = 0; 

    virtual explicit operator bool() const = 0; // for testing against nullptr
    virtual const SatelliteSerial &GetSS() const = 0;
    virtual Node *get() const = 0; // As per shared_ptr<>, ie gets the actual C pointer
    virtual Node &operator *() const = 0; 
    virtual TreePtrInterface &operator=( const TreePtrInterface &o );
    bool operator<(const TreePtrInterface &other) const;
    virtual TreePtr<Node> MakeValueArchetype() const = 0; // construct an object of the VALUE_TYPE type (NOT a clone)
    virtual string GetName() const = 0;
    virtual string GetShortName() const = 0;
    string GetTrace() const override;
};

template<typename VALUE_TYPE>
struct TreePtr : virtual TreePtrInterface, 
                 shared_ptr<VALUE_TYPE>,
                 SatelliteSerial
{
    TreePtr() {}

    explicit TreePtr( VALUE_TYPE *o ) : // dangerous - make explicit
        shared_ptr<VALUE_TYPE>( o ),
        SatelliteSerial( o, this )
    {
    }

    TreePtr( nullptr_t o ) : // safe - leave implicit
        shared_ptr<VALUE_TYPE>( nullptr ),
        SatelliteSerial( nullptr, this )
    {
    }

    template< typename OTHER >
    explicit TreePtr( const shared_ptr<OTHER> &o ) :
        shared_ptr<VALUE_TYPE>( o ),
        SatelliteSerial( o.get(), this )
    {
    }

    template< typename OTHER >
    TreePtr( const TreePtr<OTHER> &o ) :
        shared_ptr<VALUE_TYPE>( (shared_ptr<OTHER>)(o) ),
        SatelliteSerial(o)
    {
    }

    operator shared_ptr<Node>() const final
    {
        const shared_ptr<VALUE_TYPE> p = (const shared_ptr<VALUE_TYPE>)*this;
        return p;
    }

	operator TreePtr<Node>() const final
	{
        const shared_ptr<VALUE_TYPE> p1 = *(const shared_ptr<VALUE_TYPE> *)this;
        return TreePtr<Node>( p1 );
	}

    const SatelliteSerial &GetSS() const override
    {
        return *this;
    }

    VALUE_TYPE *get() const final
    {
    	VALUE_TYPE *e = shared_ptr<VALUE_TYPE>::get();
    	return e;
    }

    VALUE_TYPE &operator *() const final
    {
    	return shared_ptr<VALUE_TYPE>::operator *();
    }

    TreePtr &operator=( const TreePtrInterface &n ) final
    {
    	(void)TreePtr::operator=( shared_ptr<Node>(n) );        
    	return *this;
    }

    TreePtr &operator=( const shared_ptr<Node> &n )
    {   
        if( n )
        {
            shared_ptr<VALUE_TYPE> p = dynamic_pointer_cast<VALUE_TYPE>(n);
            ASSERT( p )("TreePtr inferred dynamic cast has failed: from ")(*n)
			           (" to type ")(TYPE_ID_NAME( VALUE_TYPE ))("\n");
         	(void)shared_ptr<VALUE_TYPE>::operator=( p );
        }
        else
        {
            (void)shared_ptr<VALUE_TYPE>::operator=( shared_ptr<VALUE_TYPE>() );
        }
        (void)SatelliteSerial::operator=( SatelliteSerial( n.get(), this ) );
    	return *this;
    }

    template< typename OTHER >
    TreePtr &operator=( const shared_ptr<OTHER> &n )
    {
    	(void)shared_ptr<VALUE_TYPE>::operator=( (n) );
    	return *this;
    }

    bool operator<(const TreePtr<VALUE_TYPE> &other) const
    {
#ifdef TREE_PTR_ORDERING_USE_SERIAL
        // Nullness is primary ordering
        if( !other.get() )
            return false; // for == and > case
        else if( !get() )
            return true; // for remaining < case
            
        // Pointed-to node serial number is secondary ordering
        if( get()->VALUE_TYPE::operator<(*(other.get())) )
            return true;
        if( other.get()->VALUE_TYPE::operator<(*get()) )
            return false;

        return false; // they are equal under this ordering
#else 
        // Fall back to shared_ptr which I think just uses address
        return this->get() < other.get();
#endif        
    }

    operator bool() const final
    {
    	return !!*(const shared_ptr<VALUE_TYPE> *)this;
    }

	static TreePtr<VALUE_TYPE>
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
	static TreePtr<VALUE_TYPE>
	    InferredDynamicCast( const TreePtrInterface &g )
	{
		if( g )
		{
			shared_ptr<VALUE_TYPE> v = dynamic_pointer_cast<VALUE_TYPE>(shared_ptr<Node>(g));
			ASSERTS( v )("OOStd inferred dynamic cast has failed: from ")(*g)
			            (" to type ")(TYPE_ID_NAME( VALUE_TYPE ))("\n");
			return TreePtr<VALUE_TYPE>(v);
		}
		else
		{
		    // Null came in, null goes out.
			return TreePtr<VALUE_TYPE>();
		}
	}
	TreePtr<Node> MakeValueArchetype() const final
	{
        return TreePtr<Node>(new VALUE_TYPE); // means VALUE_TYPE must be constructable
    }

    string GetName() const final
    {
        if( !operator bool() )           
            return string("NULL");

#ifdef SUPPRESS_SATELLITE_NUMBERS
        string s = "#?->";
#else
        // Use the serial string of the TreePtr itself #625
        string s = SatelliteSerial::GetSerialString() + "->";
#endif  
        
        s += get()->GetName();
        s += get()->GetSerialString();
        return s;
    }  
    
    string GetShortName() const final
    {
        if( !operator bool() )           
            return string("NULL");

#ifdef SUPPRESS_SATELLITE_NUMBERS
        string s = "#?->";
#else
        // Use the serial string of the TreePtr itself #625
        string s = SatelliteSerial::GetSerialString() + "->";
#endif  
        
        s += get()->GetSerialString(); 
        return s;
    }  
};


template<typename VALUE_TYPE>
TreePtr<VALUE_TYPE> DynamicTreePtrCast( const TreePtrInterface &g )
{
    return TreePtr<VALUE_TYPE>::DynamicCast(g);
}


// Similar signature to shared_ptr operator==, and we restrict the pointers
// to having the same subbase and base target
template< typename X, typename Y >
bool operator==( const TreePtr<X> &x,
		                const TreePtr<Y> &y)
{
	return operator==( (const shared_ptr<X> &)x, (const shared_ptr<Y> &)y );
}


template< typename X, typename Y >
bool operator!=( const TreePtr<X> &x,
		                const TreePtr<Y> &y)
{
	return operator!=( (const shared_ptr<X> &)x, (const shared_ptr<Y> &)y );
}


// Similar signature to shared_ptr operator==, and we restrict the pointers
// to having the same subbase and base target
bool operator==( const TreePtrInterface &x,
		         const TreePtrInterface &y);
bool operator!=( const TreePtrInterface &x,
		         const TreePtrInterface &y);
bool operator<( const TreePtrInterface &x,
		        const TreePtrInterface &y);


// Handy typing saver for creating objects and TreePtrs to them.
template<typename VALUE_TYPE, typename ... CP>
TreePtr<VALUE_TYPE> MakeTreeNode(const CP &...cp) 
{
    return TreePtr<VALUE_TYPE>( new VALUE_TYPE(cp...) );
}

#endif 
