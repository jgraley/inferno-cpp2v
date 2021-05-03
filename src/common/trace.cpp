#include "trace.hpp"

#include "progress.hpp"

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
    return SSPrintf("(void *)%p", p);
}


string Trace(const Progress &progress)
{
    return progress.GetPrefix();
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
    Tracer( file, line, function, Tracer::FORCE )( "BOOST ASSERTION FAILED: %s\n\n", expr );
    InfernoAbort();
}


void boost::assertion_failed_msg(char const * expr, char const * msg, char const * function, char const * file, long line)
{
    Tracer::MaybePrintEndl();
    Tracer( file, line, function, Tracer::FORCE )( "BOOST ASSERTION FAILED: %s\n%s\n", expr, msg );
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

Tracer::Tracer( const char *f, int l, const char *fu, Flags fl, char const *cond ) :
    file( f ),
    line( l ),
    function( fu ),
    flags( fl )
{
    // If we're going to abort, get this out first, then usual trace message if required as a continuation
    if( flags & ABORT )
    {
        MaybePrintEndl();
        clog << endl;
        PrintPrefix();
        clog << SSPrintf( "----ASSERTION FAILED: %s", cond ) << endl;
        MaybePrintBanner();
    }
}


Tracer::Tracer( Flags fl, char const *c ) :
    Tracer( "", 0, "", fl, c )
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
    if( (flags & DISABLE) || !(enable || (flags & FORCE)) )
        return *this;
 
    MaybePrintEndl();
    MaybePrintBanner();
    
    return *this;
}


Tracer &Tracer::operator()(const string &s)
{    
    if( (flags & DISABLE) || !(enable || (flags & FORCE)) )
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


void Tracer::Descend::Indent()
{
    // Detect cases where the indent level dropped and then went up again, without
    // any actual traces at the lower indent level. Just do a blank trace that leaves
    // a visible gap (the "<" was confusing; gap suffices). 
    if( leftmost_pre.size() < last_traced_pre.size() && leftmost_pre.size() < pre.size() )
        clog << leftmost_pre << endl;
    last_traced_pre = leftmost_pre = pre;
    clog << pre.c_str() << " ";
}


void Tracer::PrintPrefix()
{
    clog << Progress::GetCurrent().GetPrefix(4) << " ";
    Descend::Indent();
}


void Tracer::MaybePrintBanner()
{
    if( require_banner && (file != "" || line != 0 || function != "") )
    {
        PrintPrefix();
        clog << SSPrintf("----%s:%d in %s()", file, line, function) << endl;
        require_banner = false;
    }    
}

bool Tracer::require_endl = false;
bool Tracer::require_banner = true;
bool Tracer::enable = false; ///< call Tracer::Enable(true) to begin tracing
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
