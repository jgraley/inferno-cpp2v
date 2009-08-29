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
using namespace boost;

// TODO use BOOST_FOREACH when cygwin boost gets past rev 1.33
// For now, use my rubbish version.
#define USE_BOOST_FOREACH 0

#if USE_BOOST_FOREACH

#include <boost/foreach.hpp>
#define FOREACH BOOST_FOREACH

#else

// Use as in FOREACH( iterator i, container ) {}
template<class IT>
struct ZR : IT
{
	template<class IT2> ZR( const IT2 &it ) : IT( it ) {}
	operator bool()	{ return false;	}
};
template<typename T>
typename T::iterator GetIt( T &t )
{
	abort();
}
#define FOREACH(M, C) \
	if( ZR<typeof(GetIt(C))> __i=(C).begin() ) {} else \
	    for( bool __d = true; \
	         __d && !(__i==(C).end()); \
	         ++__i ) \
	        if( (__d=false) ) {} else \
	            for( M=*__i; !__d; __d=true )
#endif


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
