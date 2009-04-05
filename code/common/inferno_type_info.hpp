#ifndef INFERNO_TYPE_INFO_HPP
#define INFERNO_TYPE_INFO_HPP

#include <typeinfo>
#include "trace.hpp"

/* to use:

Put ITYPE_INFO_FUNCTION inside each class that will be used. Create a typeinfo using

ITypeInfo(X) where X is an object, pointer or shared_ptr

Provides name() and a full set of inequalities based on the set model, as follows:

== same class (as per C++'s type_info)
!= not same class (as per C++'s type_info)
>= non-strict superclass (superclass or same)
> strict superclass
<= non-strict subclass
< strict subclass

Explaination

Inheritance has a set-theory interpretation in which each class is a set
and subclasses are subsets. MI implies overlap between the superclasses.
This model fits with virtual inheritance as opposed to non-virtual.
  
Note that *ALL* the usual identities hold *EXCEPT* that !(A >= B) does not 
imply A < B and !(A > B) does not imply A <= B. This is because A and B can 
be disjoint or partially overlapping, in which case all inequalities 
return false.
*/


#define ITYPE_INFO_FUNCTION \
    virtual bool IsDynamicMatch( const ITypeInfo::TypeBase *source ) const \
    { \
        return ITypeInfo::IsDynamicMatch( this, source ); \
    }

class ITypeInfo 
{
public:
    struct TypeBase
    {   
        ITYPE_INFO_FUNCTION
        virtual ~TypeBase() {}
    };

private:
    const TypeBase * const pt;
    
public:
    template< class DEST >    
    static inline bool IsDynamicMatch( const DEST *dest, const TypeBase *source ) 
    { 
        (void)dest; // don't care about value; just want the type
        return !!dynamic_cast<const DEST *>(source); 
    }

    ITypeInfo( const TypeBase *p ) :
        pt( p )
    {
        ASSERT(pt);
    }
    
    ITypeInfo( shared_ptr<TypeBase> p ) :
        pt( p.get() )
    {
        ASSERT(pt);
    }
    
    ITypeInfo( const TypeBase &p ) :
        pt( &p )
    {
        ASSERT(pt);
    }
       
    inline bool operator==(const ITypeInfo& rhs) const
    {
        return typeid( *pt ) == typeid( *(rhs.pt) );
    }
    
    inline bool operator!=(const ITypeInfo& rhs) const
    {
        return !(*this==rhs);
    }
    
    inline bool operator>=(const ITypeInfo& rhs) const
    {
        return pt->IsDynamicMatch( rhs.pt );
    }

    inline bool operator>(const ITypeInfo& rhs) const
    {
        return (*this>=rhs) && (*this!=rhs);
    }

    inline bool operator<=(const ITypeInfo& rhs) const
    {
        return rhs>=*this;
    }

    inline bool operator<(const ITypeInfo& rhs) const
    {
        return rhs>*this;
    }

    inline const char* name() const
    {
        return typeid( *pt ).name(); // TODO clean up output
    }
};

#endif

