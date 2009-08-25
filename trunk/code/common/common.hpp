#ifndef COMMON_HPP
#define COMMON_HPP

#include "trace.hpp"

#include <assert.h>
#include <stdio.h>
#include <vector>
#include <stack>
#include <map>
using namespace std;
#include <stdarg.h> 

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/foreach.hpp>
using namespace boost;

#include "foreach.hpp"
/*
// TODO use BOOST_FOREACH when cygwin boost gets past rev 1.33
// For now, use my rubbish version. Use as in FOREACH( i, container ) {}
template<class IT>
struct ZR : IT
{
	ZR( const IT &it ) : IT( it ) {}
	operator bool()	{ return false;	}
};
#define FOREACH(M, C) \
	if( ZR<typeof((C).begin())> __i=(C).begin() ) {} else \
	    for( bool __d = true; \
	         __d && __i!=(C).end(); \
	         __i++ ) \
	        if( (__d=false) ) {} else \
	            for( M=*__i; !__d; __d=true )
  */
// How many members in an array
#define COUNTOF( array ) ( sizeof( array )/sizeof( array[0] ) )

// sprintf into a std::string
string SSPrintf(const char *fmt, ...);

// Pushes element t of type T onto stack s, then pops again in destructor
template< typename T >
class AutoPush
{
public:
    AutoPush( stack<T> &s, const T &t ) : st(s)
    {
        st.push(t);
        check_t = t;
    }    
    ~AutoPush()
    {
        ASSERT( st.top() == check_t );
        st.pop();
    }
    
private:
    std::stack<T> &st;
    T check_t; // just for checking
};

// Find out whether an element exists in the map, without the pain of iterators
template< typename KEY, typename DATA >
class Map : public map<KEY, DATA>
{
public:
    inline bool IsExist( const KEY &k )
    {
        return map<KEY, DATA>::find( k ) != map<KEY, DATA>::end();
    }
};

void CommonTest();

#endif
