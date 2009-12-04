#ifndef TYPE_INFO_HPP
#define TYPE_INFO_HPP

#include <typeinfo>
#include <string>
#include "common/trace.hpp"

struct Matcher
{
	virtual bool IsSubclass( const Matcher &source_architype ) const = 0;
	virtual bool IsLocalMatch( const Matcher &candidate ) const
	{
		// Default local matching criterion checks only the type of the candidate. If the
		// candidate's class is a (non-strict) subclass of this class, we have a match.
		return IsSubclass( candidate );
	}
    virtual ~Matcher() {}
    template< class TARGET_TYPE >
    static inline bool IsSubclassStatic( const TARGET_TYPE &target_architype, const Matcher &source_architype )
    {
        (void)target_architype; // don't care about value of architypes; just want the type
        return !!dynamic_cast<const TARGET_TYPE *>(&source_architype);
    }
};

#define TYPE_INFO_FUNCTION \
    virtual bool IsSubclass( const Matcher &source_architype ) const \
    { \
        return IsSubclassStatic( *this, source_architype ); \
    }


class TypeInfo 
{
private:
    const Matcher * const architype;
    
public:
    TypeInfo( const Matcher *p ) :
        architype( p )
    {
        ASSERT(architype);
    }
    
    TypeInfo( shared_ptr<Matcher> p ) :
        architype( p.get() )
    {
        ASSERT(architype);
    }
    
    TypeInfo( const Matcher &p ) :
        architype( &p )
    {
        ASSERT(architype);
    }
       
    inline string name() const
    {
        const char *s = typeid( *architype ).name();
        while( s[0]>='0' && s[0]<='9' )
           s++;
        return s;
    }
};


#endif

