#ifndef STANDARD_HPP
#define STANDARD_HPP

#include <vector>
#include <list>
#include <stack>
#include <map>
#include <set>
#include <bits/stdint-uintn.h>
#include <memory>
#include <algorithm>
#include <stdarg.h>

// JSG we now use boost's FOREACH which means we depend on Boost 1.34 (I think)
#include <boost/foreach.hpp>
#define FOREACH BOOST_FOREACH

using namespace std;


// How many members in an array
#define COUNTOF( array ) ( sizeof( array )/sizeof( array[0] ) )

// sprintf into a std::string
string VSSPrintf(const char *fmt, va_list vl);
string SSPrintf(const char *fmt, ...);

void CommonTest();

class Traceable
{
public:
    static string CPPFilt( string s );
    static string Denamespace( string s );
    
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
public:
    typedef uint64_t SNType;
    
private:    
    static SNType master_location_serial;
    static int current_step;
    static map<void *, SNType> location_serial;
    static map<SNType, void *> location_readback;
    static map<void *, SNType> master_serial;
    SNType serial;
    SNType location;
    SNType step;

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
    static void *GetLocation( SNType location );
};


string Join( const list<string> &v, string pre="", string sep=" ", string post="" );

#endif
