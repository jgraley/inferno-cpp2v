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
    }
    
    TypeInfo( shared_ptr<Matcher> p ) :
        architype( p.get() )
    {
    }
    
    TypeInfo( const Matcher &p ) :
        architype( &p )
    {
    }
       
    inline string name() const
    {
    	if( architype )
    	{
			const char *s = typeid( *architype ).name();
			while( s[0]>='0' && s[0]<='9' )
			   s++;
			return s;
    	}
    	else
    	{
    		return string("NULL");
    	}
    }
};


#endif
