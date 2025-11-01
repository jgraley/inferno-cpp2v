#include "trace.hpp"

#include "progress.hpp"
#include "read_args.hpp"

#include <stdarg.h>
#include <string.h>

#ifdef __GLIBC__
#include <execinfo.h>
#endif

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cinttypes>

using namespace std;

////////////////////////// Trace() free functions //////////////////////////

string Trace(const Traceable &t)
{    
    return t.GetTrace();
}


string Trace(string s)
{
    return "\""+s+"\""; // quoted so we can spot an empty string TODO un-escape
}


string Trace(const StringNoQuotes &snq)
{
    return (string)snq;
}


// Note: In JSON, everything is ordered (it's based on text files) 
// including maps (what JSON calls objects) and it will be usedful to 
// be able to trace a map-like thing that's order-preserving. Do this 
// by specialising eg Trace( list<KeyValuePair> )
string Trace(const KeyValuePair &kvp)
{
    return Trace(kvp.key) + ": " + string(kvp.value);
}


string Trace(bool b)
{    
    return b?string("true"):string("false");
}


string Trace(int i)
{    
    return to_string(i);
}


string Trace(unsigned i)
{    
    return to_string(i);
}


string Trace(size_t i)
{    
    return to_string(i);
}


string Trace(float f)
{    
    return to_string(f);
}


string Trace(const exception &e)
{
    return string( e.what() ? e.what() : "exception:what()=NULL" );
}


string Trace(const void *p)
{
    if( p )
#ifdef SUPPRESS_ADDRESSES
        return "void-ptr";
#else    
        return SSPrintf("%" PRIxPTR, (uintptr_t)p);
#endif        
    else
        return "NULL";
}


string Trace(const Progress &progress)
{
    return progress.GetPrefix();
}


string Trace(const std::type_info &ti)
{
    return Traceable::CPPFilt(string(ti.name())) + "#" + to_string(ti.hash_code());
}


string GetTrace()
{
    return "::";
}

////////////////////////// Misc free functions //////////////////////////

inline void InfernoAbort()
{
    fflush( stderr ); 
    abort(); 
}

////////////////////////// NewtonsCradle //////////////////////////

NewtonsCradle &NewtonsCradle::operator()()
{
    return operator()(string());
}


NewtonsCradle &NewtonsCradle::operator()(const char *fmt, ...)
{
    va_list vl;
    va_start( vl, fmt );
    string s = VSSPrintf( fmt, vl );
    va_end( vl );

    return operator()(s);
}

////////////////////////// Tracer //////////////////////////

Tracer::Tracer( const char *f, int l, string in, const char *fu, Flags fl, char const *cond ) :
    file( f ),
    line( l ),
    instance( in ),
    function( fu ),
    flags( fl )
{
    // If we're going to abort, get this out first, then usual trace message if required as a continuation
    if( flags & ABORT )
    {
        MaybePrintEndl();
        clog << endl;
        PrintPrefix();
        MaybePrintBanner();
        clog << SSPrintf( "---- ASSERTION FAILED: %s", cond ) << endl;
    }
}


Tracer::Tracer( Flags fl, char const *c ) :
    Tracer( "", 0, "", "", fl, c )
{
}


Tracer::~Tracer()
{
    if( flags & ABORT )
    {
        MaybePrintEndl();
        InfernoAbort();
    }
    require_banner = true;
}


Tracer &Tracer::operator()()
{
    if( (flags & DISABLE) || !(IsEnabled() || (flags & FORCE)) )
        return *this;
 
    MaybePrintEndl();
    MaybePrintBanner();
    
    return *this;
}


Tracer &Tracer::operator()(const string &s)
{    
    if( (flags & DISABLE) || !(IsEnabled() || (flags & FORCE)) )
        return *this;

    if( !require_endl_at_destruct ) 
        MaybePrintBanner();
    
    stringstream ss(s);
    string segment;
    bool first = true;
    int local_indent = 0;
    while( getline(ss, segment, '\n') )
    {
        if( !first )
            clog << endl; // put back the endls that getline() removed

        if( !require_endl_at_destruct || !first) 
            PrintPrefix(local_indent); // provide prefix if we're in home column
        
        clog << segment;

        local_indent += count(segment.begin(), segment.end(), '(');
        local_indent += count(segment.begin(), segment.end(), '[');
        local_indent += count(segment.begin(), segment.end(), '{');
        local_indent -= count(segment.begin(), segment.end(), ')');
        local_indent -= count(segment.begin(), segment.end(), ']');
        local_indent -= count(segment.begin(), segment.end(), '}');

        first = false;        
    }
    
    // Will we end up out of home column?
    require_endl_at_destruct = true;
    if( !s.empty() && s.back() == '\n' )
    {
        // getline didn't give us an empty line for the last \n
        clog << endl; // put back the endl that getline() removed
        require_endl_at_destruct = false;
    }
    return *this;    
}


void Tracer::Enable( bool e )
{
    enable = e;
}


void Tracer::MaybePrintEndl()
{
    if( require_endl_at_destruct ) 
    {
        clog << endl;
        require_endl_at_destruct = false;
    }   
}


Tracer::Descend::Descend( string s ) : 
    os(pre.size()),
    num_exceptions( uncaught_exceptions() )
{ 
    ASSERT( s.length()>=1 );
    pre += s; 
    Tracer::MaybePrintEndl(); 
} 


Tracer::Descend::~Descend() 
{ 
    if(Tracer::IsEnabled())
    {
		int nne = uncaught_exceptions();
        if( nne>num_exceptions ) // is there at least one new exception?
            Tracer()("Ouch!\n");
        else                    
            Tracer()("OK\n");
    }
    
    pre = pre.substr(0, os); 
    if( pre.size() < leftmost_pre.size() )
        leftmost_pre = pre;
}


void Tracer::Descend::Indent(string sprogress)
{
    // Detect cases where the indent level dropped and then went up again, without
    // any actual traces at the lower indent level. Just do a blank trace that leaves
    // a visible gap (the "<" was confusing; gap suffices). 
    if( leftmost_pre.size() < last_traced_pre.size() && leftmost_pre.size() < pre.size() )
        clog << sprogress << leftmost_pre << endl;

    clog << sprogress << pre.c_str() << " ";

    last_traced_pre = leftmost_pre = pre;
}


void Tracer::PrintPrefix(int local_indent)
{
    string sprogress = Progress::GetCurrent().GetPrefix(4) + " ";
    if( ReadArgs::trace_no_stack )
        clog << sprogress;
    else
        Descend::Indent( sprogress );
    clog << string(local_indent*4, ' ');
}


void Tracer::MaybePrintBanner()
{
    if( require_banner && (strcmp(file, "") != 0 || line != 0 || instance != "" || strcmp(function, "") != 0) )
    {
        string i_f = JoinInstanceFunction( instance, function );
        PrintPrefix();
        clog << SSPrintf("---- %s:%d in %s", file, line, i_f.c_str()) << endl;
        require_banner = false;
    }    
}

bool Tracer::require_endl_at_destruct = false;
bool Tracer::require_banner = true;
bool Tracer::enable = false; ///< call Tracer::Enable(true) to begin tracing
bool Tracer::disable = false;
string Tracer::Descend::pre;
string Tracer::Descend::last_traced_pre, Tracer::Descend::leftmost_pre;

////////////////////////// TraceTo //////////////////////////

TraceTo::TraceTo( string &str ) :
    p_str( &str ),
    p_osm( nullptr )
{
}


TraceTo::TraceTo( ostream &osm ) :
    p_str( nullptr ),
    p_osm( &osm )
{
}


TraceTo &TraceTo::operator()(const string &s)
{
    if( p_str )
        *p_str += s;
    else if( p_osm )
        *p_osm << s;
    else
        ASSERTFAIL();
    return *this;
} 
