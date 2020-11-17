#include "trace.hpp"
#include <boost/assert.hpp>
#include <stdarg.h>
#include <string.h>

#ifdef __GLIBC__
#include <execinfo.h>
#endif

#include <iostream>

bool Tracer::require_endl = false;
bool Tracer::require_banner = true;
bool Tracer::enable = false; ///< call Tracer::Enable(true) to begin tracing
string Tracer::Descend::pre;
string Tracer::Descend::last_traced_pre, Tracer::Descend::leftmost_pre;

using namespace std;

string MyTrace(const Traceable &t)
{    
    return t.GetTrace();
}


string MyTrace(bool b)
{    
    return b?string("true"):string("false");
}


string MyTrace(int i)
{    
    return to_string(i);
}


string MyTrace(size_t i)
{    
    return to_string(i);
}


string MyTrace(const exception &e)
{
    return string( e.what() );
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


Tracer::Tracer( const char *f, int l, const char *fu, Flags fl, char const *c ) :
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
        clog << SSPrintf( "----ASSERTION FAILED: %s", c ) << endl;
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


Tracer &Tracer::operator()(const char *fmt, ...)
{
    va_list vl;
    va_start( vl, fmt );
    string s = VSSPrintf( fmt, vl );
    va_end( vl );

    return operator()(s);
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

void Tracer::SetStep( int s )
{
    current_step = s;
}


void Tracer::PrintPrefix()
{
    switch( current_step )
    {
        case -4: // inputting
            break;
        case -3: // inputting
            clog << "I  ";
            break;
        case -2: // outputting
            clog << "O  ";
            break;
        case -1: // planning
            clog << "P  ";
            break;
        default: // during a step
            clog << SSPrintf( "%03d", current_step );  
            break;
    }
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




// Make BOOST_ASSERT work (we don't use them but other code might)
void boost::assertion_failed(char const * expr, char const * function, char const * file, long line)
{
    Tracer::MaybePrintEndl();
    Tracer( file, line, function, Tracer::FORCE )( "BOOST ASSERTION FAILED: %s\n\n", expr );
    InfernoAbort();
}


int Tracer::current_step = -4; // -4 is disable


