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
    char cs[10240];    
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
}

// Max number of characters Join() will generate before deciding to 
// insert newline after each separator 
#define MAX_INLINE_JOIN 100

string Join( const list<string> &ls, string sep, string pre, string post )
{
    const bool sep_has_nl = sep.size() > 0 && sep.at(sep.size()-1)=='\n';
    bool first = true;
    string s;
    s += pre;
    for( string si : ls )
    {
        if( !first )
            s += sep;
        first = false;
        s += si;
        if( !sep_has_nl && s.size() > MAX_INLINE_JOIN )
            return Join( ls, sep+string("\n"), pre, post );
    }
    s += post;
    return s;
} 


string JoinInstanceFunction( string instance, string function )
{
    string indot;
    if( instance != "" && instance.substr(instance. size()-2) != "::" )
        indot = instance+".";
    return SSPrintf("%s%s()", indot.c_str(), function.c_str());
}


string OrdinalString( int i )
{
	string s = to_string(i);
	if( i%10==1 && i%100!=11 )
		return s+"st";
	if( i%10==2 && i%100!=12 )
		return s+"nd";
	return s+"th";	
}
