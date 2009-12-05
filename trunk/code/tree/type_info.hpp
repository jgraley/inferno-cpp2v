#ifndef TYPE_INFO_HPP
#define TYPE_INFO_HPP

#include <typeinfo>
#include <string>
#include "common/trace.hpp"
#include "match.hpp"

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

