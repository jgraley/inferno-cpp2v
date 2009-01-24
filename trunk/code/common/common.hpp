#ifndef COMMON_HPP
#define COMMON_HPP

#include "trace.hpp"

#include <assert.h>
#include <stdio.h>
#include <vector>
#include <stack>
using namespace std;

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
using namespace boost;

// TODO use BOOST_FOREACH when cygwin boost gets past rev 1.33
// For now, use my rubbish version
#define FOREACH(M, C) \
    if( int __i=1 ) \
        for( bool __d = true; \
             __d && __i<=(C).size(); \
             __i++ ) \
            if( (__d=false) ) {} \
            else \
                for( M=(C)[__i-1]; !__d; __d=true ) 
  

#define COUNTOF( array ) ( sizeof( array )/sizeof( array[0] ) )


// Does a push then pops again in destructor
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

#endif
