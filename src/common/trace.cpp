#include "trace.hpp"

#include "progress.hpp"
#include "read_args.hpp"

#include <boost/assert.hpp>
#include <stdarg.h>
#include <string.h>

#ifdef __GLIBC__
#include <execinfo.h>
#endif

#include <iostream>
#include <sstream>

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


string Trace(size_t i)
{    
    return to_string(i);
}


string Trace(const exception &e)
{
    return string( e.what() ? e.what() : "exception:what()=NULL" );
}


string Trace(const void *p)
{
    if( p )
        return SSPrintf("%p", p);
    else
        return "NULL";
}


string Trace(const Progress &progress)
{
    return progress.GetPrefix();
}


string GetTrace()
{
    return "::";
}

////////////////////////// Misc free functions //////////////////////////

#ifdef __GLIBC__
inline void InfernoBacktrace()
{
  void *return_addresses[1000];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(return_addresses, 1000);

  // print out all the frames to stderr
  backtrace_symbols_fd(return_addresses, size, STDERR_FILENO);
}
#endif


inline void InfernoAbort()
{
#ifdef __GLIBC__
    //InfernoBacktrace();
    // Doesn't work :(
#endif    
    fflush( stderr ); 
    abort(); 
}


// Make BOOST_ASSERT work (we don't use them but other code might)
void boost::assertion_failed(char const * expr, char const * function, char const * file, long line)
{
    Tracer::MaybePrintEndl();
    Tracer( file, line, "", function, Tracer::FORCE )( "BOOST ASSERTION FAILED: %s\n\n", expr );
    InfernoAbort();
}


void boost::assertion_failed_msg(char const * expr, char const * msg, char const * function, char const * file, long line)
{
    Tracer::MaybePrintEndl();
    Tracer( file, line, "", function, Tracer::FORCE )( "BOOST ASSERTION FAILED: %s\n%s\n", expr, msg );
    InfernoAbort();
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

    if( !require_endl ) 
    {    	
        MaybePrintBanner();
        PrintPrefix();
    }
    
    clog << s;
    
    require_endl = (s.empty() || s.back() != '\n');
    return *this;    
}


void Tracer::Enable( bool e )
{
    enable = e;
}


void Tracer::MaybePrintEndl()
{
    if( require_endl ) 
    {
        clog << endl;
        require_endl = false;
    }   
}


Tracer::Descend::Descend( string s ) : 
    os(pre.size()) 
{ 
    ASSERT( s.length()>=1 );
    ASSERT( !isspace(s[0]) );
    pre += s; 
    Tracer::MaybePrintEndl(); 
} 


Tracer::Descend::~Descend() 
{ 
    if(Tracer::IsEnabled())
    {
        if( uncaught_exception() )
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


void Tracer::PrintPrefix()
{
    string sprogress = Progress::GetCurrent().GetPrefix(4) + " ";
    if( ReadArgs::trace_no_stack )
        clog << sprogress;
    else
        Descend::Indent( sprogress );
}


void Tracer::MaybePrintBanner()
{
    if( require_banner && (strcmp(file, "") != 0 || line != 0 || instance != "" || strcmp(function, "") != 0) )
    {
        string indot;
        if( instance != "" && instance.substr(instance. size()-2) != "::" )
            indot = instance+".";
        PrintPrefix();
        clog << SSPrintf("---- %s:%d in %s%s()", file, line, indot.c_str(), function) << endl;
        require_banner = false;
    }    
}

bool Tracer::require_endl = false;
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
