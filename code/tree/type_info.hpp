#ifndef TYPE_INFO_HPP
#define TYPE_INFO_HPP

#include <typeinfo>
#include <string>
#include "common/trace.hpp"

/* to use:

Put ITYPE_INFO_FUNCTION inside each class that will be used. Create a typeinfo using

TypeInfo(X) where X is an object, pointer or shared_ptr

Provides name() and a full set of inequalities based on the set model, as follows:

== same class (as per C++'s type_info)
!= not same class (as per C++'s type_info)
>= non-strict superclass (superclass or same)
> strict superclass
<= non-strict subclass
< strict subclass

Explanation

Inheritance has a set-theory interpretation in which each class is a set
and subclasses are subsets. MI implies overlap between the superclasses.
This model fits with virtual inheritance as opposed to non-virtual.
  
Note that *ALL* the usual identities hold *EXCEPT* that !(A >= B) does not 
imply A < B and !(A > B) does not imply A <= B. This is because A and B can 
be disjoint or partially overlapping, in which case all inequalities 
return false.
*/

class TypeInfo 
{
public:
    struct TypeBase
    {   
    	virtual bool IsDynamicMatchVirtual( const TypeInfo::TypeBase *source_architype ) const = 0;
        virtual ~TypeBase() {}
    };

private:
    const TypeBase * const architype;
    
public:
    template< class TARGET_TYPE >    
    static inline bool IsDynamicMatchStatic( const TARGET_TYPE *target_architype, const TypeBase *source_architype ) 
    { 
        (void)target_architype; // don't care about value of architypes; just want the type
        return !!dynamic_cast<const TARGET_TYPE *>(source_architype); 
    }

    TypeInfo( const TypeBase *p ) :
        architype( p )
    {
        ASSERT(architype);
    }
    
    TypeInfo( shared_ptr<TypeBase> p ) :
        architype( p.get() )
    {
        ASSERT(architype);
    }
    
    TypeInfo( const TypeBase &p ) :
        architype( &p )
    {
        ASSERT(architype);
    }
       
    inline bool operator==(const TypeInfo& rhs) const
    {
        return typeid( *architype ) == typeid( *(rhs.architype) );
    }
    
    inline bool operator!=(const TypeInfo& rhs) const
    {
        return !(*this==rhs);
    }
    
    inline bool operator>=(const TypeInfo& rhs) const
    {
        return architype->IsDynamicMatchVirtual( rhs.architype );
    }

    inline bool operator>(const TypeInfo& rhs) const
    {
        return (*this>=rhs) && (*this!=rhs);
    }

    inline bool operator<=(const TypeInfo& rhs) const
    {
        return rhs>=*this;
    }

    inline bool operator<(const TypeInfo& rhs) const
    {
        return rhs>*this;
    }

    inline string name() const
    {
        const char *s = typeid( *architype ).name();
        while( s[0]>='0' && s[0]<='9' )
           s++;
        return s;
    }
};

#define TYPE_INFO_FUNCTION \
	private: friend class TypeInfo; \
    virtual bool IsDynamicMatchVirtual( const TypeInfo::TypeBase *source_architype ) const \
    { \
        return TypeInfo::IsDynamicMatchStatic( this, source_architype ); \
    } \
    public:

#endif

