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

#include <memory>
#include <algorithm>
using namespace std; // This is done twice, in order to make certain it "takes"

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


// Union two maps. Second overrules first!!
template< typename KEY, typename DATA >
inline map<KEY, DATA> MapUnion( const map<KEY, DATA> m1, const map<KEY, DATA> m2 )
{
	typedef pair<KEY, DATA> Pair;
    map<KEY, DATA> result = m1;
    FOREACH( Pair x, m2 )
    {
		result[x.first] = x.second;
    }
    return result; 
}


// Union two sets, without the pain of iterators
template< typename KEY >
inline set<KEY> SetUnion( const set<KEY> s1, const set<KEY> s2 )
{
    set<KEY> result;
    std::set_union( s1.begin(), s1.end(), 
                    s2.begin(), s2.end(),
                    std::inserter(result, result.end()) );
    return result; 
}


// Intersect two sets, without the pain of iterators
template< typename KEY >
inline set<KEY> SetIntersection( const set<KEY> s1, const set<KEY> s2 )
{
    set<KEY> result;
    std::set_intersection( s1.begin(), s1.end(), 
                            s2.begin(), s2.end(),
                            std::inserter(result, result.end()) );
    return result; 
}


// Intersect set with complement, without the pain of iterators
template< typename KEY >
inline set<KEY> SetDifference( const set<KEY> s1, const set<KEY> s2 )
{
    set<KEY> result;
    std::set_difference( s1.begin(), s1.end(), 
                         s2.begin(), s2.end(),
                         std::inserter(result, result.end()) );
    return result; // There, much nicer!
}    

void CommonTest();

class Traceable
{
public:
    static string CPPFilt( string s );
    virtual string GetTypeName() const; 
    virtual string GetName() const; // used by parse, render etc
    virtual string GetAddr() const; 
    virtual string GetTrace() const; // used for debug
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
    void Construct();
    inline SerialNumber() 
    { 
        Construct();       
    }
    inline SerialNumber( const SerialNumber &other ) 
    {
        // Identity semantics: ignore "other"
        Construct();        
    }
    inline SerialNumber &operator=( const SerialNumber &other )
    {
        // Identity semantics: ignore "other"
        return *this;
    }
    inline SNType GetSerialNumber() const 
    {
        return serial;
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
