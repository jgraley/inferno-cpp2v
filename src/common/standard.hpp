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
#include <unordered_map>

// JSG we now use boost's FOREACH which means we depend on Boost 1.34 (I think)
#include <boost/foreach.hpp>
#define FOREACH BOOST_FOREACH

using namespace std;

// How many members in an array
#define COUNTOF( array ) ( sizeof( array )/sizeof( array[0] ) )

class Traceable
{
public:
    static string CPPFilt( string s );
    static string Denamespace( string s );
    
    virtual string GetTypeName() const; 
    virtual string GetName() const; // used by parse, render etc
    virtual string GetTrace() const; // used for debug
};

// sprintf into a std::string
string VSSPrintf(string fmt, va_list vl);
string SSPrintf(string fmt, ...);

void CommonTest();

string Join( const list<string> &v, string pre="", string sep=" ", string post="" );

#endif
