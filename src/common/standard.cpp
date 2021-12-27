#include "standard.hpp"
#include "trace.hpp"
#include <cxxabi.h>
#include <stdio.h>

//////////////////////////// Traceable ///////////////////////////////

string Traceable::CPPFilt( string s )
{
    int status;
    char *ps;
    // Use GCC extension to demangle based on the present ABI
    ps = abi::__cxa_demangle(s.c_str(), 0, 0, &status);
    s = ps;
    free(ps); // as ordained
    return s;
}


string Traceable::Denamespace( string s )
{
    // fprintf(stderr, "[[[%s]]]", s.c_str());

    // We wish to strip off namespaces to reduce clutter, but
    // we don't want to break template representations.
    // I.e. we'd like to turn "SR::Disjunction<CPPTree::Type>"
    // into "Disjunction<Type>". 
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
    return Denamespace( TypeIdName( *this ) );
}


string Traceable::GetName() const
{
    return GetTypeName();
}


string Traceable::GetTrace() const
{
    return GetName();
}

////////////////////////// Misc free functions //////////////////////////

string VSSPrintf(string fmt, va_list vl)
{
    char cs[1024];    
    vsnprintf( cs, sizeof(cs), fmt.c_str(), vl );  
    //cs[sizeof(cs)-1] = '\0';
    return string(cs);
}


string SSPrintf(string fmt, ...)
{
    va_list vl;
    va_start( vl, fmt );
    string s = VSSPrintf( fmt.c_str(), vl );
    va_end( vl );
    
    return s;
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


string Join( const list<string> &ls, string sep, string pre, string post )
{
    bool first = true;
    string s;
    s += pre;
    for( string si : ls )
    {
        ASSERT( !si.empty() )("Empty string in Join(); got ")(ls);
        if( !first )
            s += sep;
        first = false;
        s += si;
    }
    s += post;
    return s;
} 

