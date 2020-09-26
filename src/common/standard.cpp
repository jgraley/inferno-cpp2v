#include "standard.hpp"
#include "trace.hpp"
#include <stdarg.h>
#include <cxxabi.h>
#include <stdio.h>

string SSPrintf(const char *fmt, ...)
{
    char cs[256];

    va_list vl;
    va_start( vl, fmt );
    vsnprintf( cs, sizeof(cs), fmt, vl );
    va_end( vl );
    
    return string(cs);
}

void CommonTest()
{
	// Test the FOREACH macro in case it's our dodgy home made one
	deque<int> d;
	deque<int> *pd = &d;
	d.push_back(100);
	d.push_back(101);
	d.push_back(102);
	d.push_back(103);
	deque<int> r;
	FOREACH( int i, *pd )
	    r.push_back(i);
	ASSERT( r.size() == 4 );
    ASSERT( r[0] == 100 );
    ASSERT( r[1] == 101 );
    ASSERT( r[2] == 102 );
    ASSERT( r[3] == 103 );
}


string Traceable::CPPFilt( string s )
{
    int status;
    char *ps;
    // Use GCC extension to demangle based on the present ABI
    ps = abi::__cxa_demangle(s.c_str(), 0, 0, &status);
    s = ps;
    free(ps); // as ordained

    // fprintf(stderr, "[[[%s]]]", s.c_str());

    // We wish to strip off namespaces to reduce clutter, but
    // we don't want to break template representations.
    // I.e. we'd like to turn "SR::MatchAny<CPPTree::Type>"
    // into "MatchAny<Type>". 
    list<string> parts;
    while(1)
    {
        size_t p = s.rfind("<");
        if( p == string::npos )
            break;
        parts.push_back( s.substr(0, p+1) );
        s = s.substr( p+1 );
    }
    parts.push_back( s );

    s.clear();
    for( string part : parts )
    {
        size_t p = part.rfind("::");
        if( p != string::npos )
            part = part.substr(p+2);
        s += part;
    }
    
    
    return s;
}


string Traceable::GetTypeName() const
{
    return CPPFilt( typeid( *this ).name() );
}


string Traceable::GetName() const
{
    return GetTypeName();
}


string Traceable::GetAddr() const
{
    return string();
}


string Traceable::GetTrace() const
{
    return GetName() + GetAddr(); // name plus pointer
}


SerialNumber::SNType SerialNumber::master_location_serial;
int SerialNumber::step;
map<void *, SerialNumber::SNType> SerialNumber::location_serial;
map<void *, SerialNumber::SNType> SerialNumber::master_serial;

void SerialNumber::Construct()
{
    // Get the point in the code where we were constructed 
    void *lp = __builtin_return_address(1); 
    
    // See if we know about this location
    map<void *, SNType>::iterator it = location_serial.find(lp);
    if( it == location_serial.end() )
    {
        // We don't know about this location, so produce a new location seriel number and start the construction count 
        location_serial.insert( pair<void *, SerialNumber::SNType>(lp, master_location_serial) );
        master_serial.insert( pair<void *, SerialNumber::SNType>(lp, 0) );
        master_location_serial++;
    }
        
    // Remember values for this object
    serial = master_serial[lp];
    location = location_serial[lp];
    
    // produce a new construction serial number
    master_serial[lp]++;
}    

void SerialNumber::SetStep( int s )
{
    step = s;
    // Just bin the structures we built up - this forces step to be primary ordering
    location_serial = map<void *, SNType>();
    master_serial = map<void *, SNType>();
}

string SerialNumber::GetAddr() const
{
    string ss = SSPrintf("#%d/%lu/%lu", step, location, serial);  
    //ss += SSPrintf("@%p", this);
    return ss;
}
