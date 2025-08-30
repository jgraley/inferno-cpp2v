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


//#define TREE_POINTER_REF_COUNTS
//#define TREE_POINTER_REF_TRACKING

template<typename VALUE_TYPE>
struct Sequence;

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

// -------------------------- TreePtrInterface ----------------------------    

// An interface for our TreePtr object. This interface works regardless of pointed-to
// type; it also masquerades as a TreePtr to Node type, which should be the
// base class of the pointed-to things.
struct TreePtrInterface : virtual Itemiser::Element
{
    // Convert to and from shared_ptr<VALUE_INTERFACE> and TreePtr<VALUE_INTERFACE>
    virtual explicit operator shared_ptr<Node>() const = 0; 
    virtual explicit operator TreePtr<Node>() const = 0; 

    virtual explicit operator bool() const = 0; // for testing against nullptr
    virtual const SatelliteSerial &GetSS() const = 0;
    virtual Node *get() const = 0; // As per shared_ptr<>, ie gets the actual C pointer
    virtual Node &operator *() const = 0; 
    virtual TreePtrInterface &operator=( const TreePtrInterface &o );
    virtual TreePtrInterface &operator=( nullptr_t n ) = 0;
    bool operator<(const TreePtrInterface &other) const;
    bool operator==(const TreePtrInterface &other) const;
    bool operator!=(const TreePtrInterface &other) const;
    static Orderable::Diff Compare3Way(const TreePtrInterface &l, const TreePtrInterface &r);
    static Orderable::Diff Compare3WayIdentity(const TreePtrInterface &l, const TreePtrInterface &r);
    virtual TreePtr<Node> MakeValueArchetype() const = 0; // construct an object of the VALUE_TYPE type (NOT a clone)

#ifdef TREE_POINTER_REF_COUNTS
	virtual void AddRef(const Traceable *ref) const = 0;
	virtual void RemoveRef(const Traceable *ref) const = 0;
#endif
    virtual string GetSerialString() const = 0;
    virtual string GetName() const = 0;
    virtual string GetShortName() const = 0;
};

// -------------------------- TreePtrCommon ----------------------------    

struct TreePtrCommon : virtual TreePtrInterface, public SatelliteSerial
{
	using TreePtrInterface::operator=;
    TreePtrCommon();
#ifdef TREE_POINTER_REF_COUNTS
    virtual ~TreePtrCommon();
#endif
    TreePtrCommon(const TreePtrCommon &other);
    TreePtrCommon &operator=(const TreePtrCommon &other);
    TreePtrCommon(TreePtrCommon &&other) = delete;
    TreePtrCommon &operator=(TreePtrCommon &&other) = delete;
    
    explicit TreePtrCommon( Node *o );
    TreePtrCommon( nullptr_t o );
    
    template< typename OTHER >
    explicit TreePtrCommon( const shared_ptr<OTHER> &o ) :
        SatelliteSerial( o.get(), this )
    {
    }

    template< typename OTHER >
    TreePtrCommon( const TreePtr<OTHER> &o ) :
        SatelliteSerial(o)
    {
    }

#ifdef TREE_POINTER_REF_COUNTS
    void AddRef(const Traceable *ref) const final;
    void RemoveRef(const Traceable *ref) const final;    
#endif

    const SatelliteSerial &GetSS() const override;
    string GetSerialString() const final;
    string GetName() const final;
    string GetShortName() const;    
    string GetTrace() const override;
    
#ifdef TREE_POINTER_REF_COUNTS
	mutable unsigned ref_count = 0;
#ifdef TREE_POINTER_REF_TRACKING
	mutable set<const Traceable *> references;
#endif	
#endif
};

// -------------------------- TreePtr template ----------------------------    
#ifdef TREE_POINTER_REF_COUNTS
extern list<string> deletions;
#endif

template<typename VALUE_TYPE>
struct TreePtr : virtual TreePtrCommon, 
                 shared_ptr<VALUE_TYPE>
{
	using TreePtrInterface::operator=;
	
    typedef VALUE_TYPE value_type;
    
    TreePtr() {}

#ifdef TREE_POINTER_REF_COUNTS
	~TreePtr() final
	{
		deletions.push_front( Trace(*this) );
	}
#endif

    explicit TreePtr( VALUE_TYPE *o ) : 
        TreePtrCommon( o ),
        shared_ptr<VALUE_TYPE>( o )
    {
    }

    TreePtr( nullptr_t o ) : 
        TreePtrCommon( o ),
        shared_ptr<VALUE_TYPE>( nullptr )
    {
    }

    template< typename OTHER >
    explicit TreePtr( const shared_ptr<OTHER> &o ) :
        TreePtrCommon( o ),
        shared_ptr<VALUE_TYPE>( o )
    {
    }

    template< typename OTHER >
    TreePtr( const TreePtr<OTHER> &o ) :
        TreePtrCommon( o ),
        shared_ptr<VALUE_TYPE>( (shared_ptr<OTHER>)(o) )
    {
    }

    operator shared_ptr<Node>() const override
    {
        const shared_ptr<VALUE_TYPE> p = (const shared_ptr<VALUE_TYPE>)*this;
        return p;
    }

    operator TreePtr<Node>() const override
    {
        const shared_ptr<VALUE_TYPE> p1 = *(const shared_ptr<VALUE_TYPE> *)this;
        return TreePtr<Node>( p1 );
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
    
    TreePtr &operator=( const TreePtrInterface &n ) final
    {
        (void)TreePtr::operator=( shared_ptr<Node>(n) );        
        return *this;
    }
    
    TreePtrInterface &operator=( nullptr_t n ) final
    {
        (void)shared_ptr<VALUE_TYPE>::operator=( n );
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

	string GetTrace() const
	{
		if( !operator bool() )           
			return string("NULL");

#ifdef SUPPRESS_SATELLITE_NUMBERS
		string s = "&";
#else
		// Use the serial string of the TreePtr itself #625
		string s = SatelliteSerial::GetSerialString() + "->";
#endif  
		
		s += get()->GetTrace();
		return s;
	} 
};

// -------------------------- Extra bits ----------------------------    

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
    return !operator==( (const shared_ptr<X> &)x, (const shared_ptr<Y> &)y );
}


// Handy typing saver for creating objects and TreePtrs to them.
template<typename VALUE_TYPE, typename ... CP>
TreePtr<VALUE_TYPE> MakeTreeNode(const CP &...cp) 
{
    return TreePtr<VALUE_TYPE>( new VALUE_TYPE(cp...) );
}

// The hashing functions we require in order to use links as keys to
// unordered_set, unordered_map
namespace std
{
    template<typename X> struct hash<TreePtr<X>>
    {
        size_t operator()(const TreePtr<X> &tp_x) const noexcept
        {
            return std::hash<decltype(tp_x.get())>()(tp_x.get()) >> HASHING_POINTERS_ALIGNMENT_BITS; 
        }
    };
}


#endif 
