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
#include <stdint.h>

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

class Traceable
{
public:
    static string CPPFilt( string s );
    virtual string GetName() const; // used by parse, render etc
    virtual string GetAddr() const; 
    virtual operator string() const; // used for debug
};

//
// The idea is to provide an alternative to raw pointers for ordering our sets, miltsets etc.
// This should be more repeatable - i.e. a slight disturbance to the dynamic memory allocator
// should not cause everything to come out in a different order.
//
// We construct the new ordering as follows:
// 1st: the step the object was constructed during (as specified by the top level)
// 2nd: the location in the code where the object was constructed, foced down into 
//      a sequential ordering to protect against changes in the location of code
// 3rd: the count of objects constructed at that point.
//
// Note the 2nd criterion should differentiate between different object types, so 
// no further action needed there.
//
class SerialNumber
{
    typedef uint64_t SNType;
    static SNType master_location_serial;
    static int step;
    static map<void *, SNType> location_serial;
    static map<void *, SNType> master_serial;
    SNType serial;
    SNType location;

protected:
    SerialNumber();
    inline SNType GetSerialNumber() const 
    {
        return serial;
    }
    inline SerialNumber &operator=( const SerialNumber &other )
    {
        return *this;
    }
    string GetAddr() const; 
    inline bool operator<( const SerialNumber &o )
    {
        if( step != o.step )
            return step < o.step;
        else if( location < o.location )
            return location < o.location;
        else
            return serial < o.serial;
    }
public:
    static void SetStep( int s );
};

#endif
