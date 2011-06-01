#ifndef STANDARD_HPP
#define STANDARD_HPP

#include <assert.h>
#include <stdio.h>
#include <vector>
#include <stack>
#include <map>
#include <set>
using namespace std;
#include <stdarg.h> 

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
using namespace boost;

// JSG we now use boost's FOREACH which means we depend on Boost 1.34 (I think)
#include <boost/foreach.hpp>
#define FOREACH BOOST_FOREACH

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
    }    
    ~AutoPush()
    {
        st.pop();
    }
    
private:
    std::stack<T> &st;
};

// Find out whether an element exists in the map, without the pain of iterators
template< typename KEY, typename DATA >
class Map : public map<KEY, DATA>
{
public:
    inline bool IsExist( const KEY &k ) const 
    {
        return map<KEY, DATA>::find( k ) != map<KEY, DATA>::end();
    }
};

template< typename KEY >
class Set : public set<KEY>
{
public:
    inline bool IsExist( const KEY &k ) const 
    {
        return set<KEY>::find( k ) != set<KEY>::end();
    }
};

void CommonTest();

#endif
