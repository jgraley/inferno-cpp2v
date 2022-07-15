#ifndef STANDARD_HPP
#define STANDARD_HPP

#include <vector>
#include <list>
#include <stack>
#include <map>
#include <set>
#include <bits/stdint-uintn.h>
#include <memory>
#include <stdarg.h>

// needed for get_current_dir_name() in parse.h (lol)
#include <boost/foreach.hpp>

using namespace std;

// How many members in an array
#define COUNTOF( array ) ( sizeof( array )/sizeof( array[0] ) )

// Single source of truth for how to get printable name of a type, but
// if you have an object, please use Traceable::TypeIdName()
#define TYPE_ID_NAME(TYPE) ( CPPFilt( typeid(TYPE).name() ) )

class Traceable
{
public:
    static string CPPFilt( string s );
    static string Denamespace( string s );
    template<typename T>
    static string TypeIdName( const T &t )
    {
        return TYPE_ID_NAME( t );
    }
    
    virtual string GetTypeName() const; 
    virtual string GetName() const; // used by parse, render etc
    virtual string GetTrace() const; // used for debug
};

// sprintf into a std::string
string VSSPrintf(string fmt, va_list vl);
string SSPrintf(string fmt, ...);

void CommonTest();

string Join( const list<string> &v, string sep=" ", string pre="", string post="" );

string JoinInstanceFunction( string instance, string function );

#endif
