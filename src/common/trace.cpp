#include "trace.hpp"
#include <boost/assert.hpp>
#include <stdarg.h>
#include <string.h>

#ifdef __GLIBC__
#include <execinfo.h>
#endif

bool Tracer::require_endl = false;
bool Tracer::require_banner = true;
bool Tracer::enable = false; ///< call Tracer::Enable(true) to begin tracing
string Tracer::Descend::pre;
string Tracer::Descend::last_traced_pre, Tracer::Descend::leftmost_pre;

using namespace std;

void Tracer::Descend::Indent()
{
    // Detect cases where the indent level dropped and then went up again, without
    // any actual traces at the lower indent level. Just do a blank trace that leaves
    // a visible gap (the "<" was confusing; gap suffices). 
    if( leftmost_pre.size() < last_traced_pre.size() && leftmost_pre.size() < pre.size() )
        fprintf(stderr, "%s\n", leftmost_pre.c_str());
    last_traced_pre = leftmost_pre = pre;
    fprintf(stderr, "%s ", pre.c_str());
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
        fprintf( stderr, "\n");
        Descend::Indent();
        fprintf( stderr, "----ASSERTION FAILED: %s\n", c);
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


Tracer &Tracer::operator()(const char *fmt, ...)
{
    if( (flags & DISABLE) || !(enable || (flags & FORCE)) )
        return *this;

    va_list vl;
    va_start( vl, fmt );
    if( !require_endl ) 
    {    	
        MaybePrintBanner();
        Descend::Indent();
    }
    vfprintf( stderr, fmt, vl );
    va_end( vl );
    
    require_endl = (strlen(fmt)==0 || fmt[strlen(fmt)-1]!='\n');
    return *this;
}


Tracer &Tracer::operator()(const string &s)
{    
    return operator()("%s", s.c_str());
}


Tracer &Tracer::operator()(const Traceable &s)
{    
    return operator()(s.GetTrace());
}


Tracer &Tracer::operator()(bool b)
{    
    return operator()(b?"true":"false");
}


Tracer &Tracer::operator()(int i)
{    
    return operator()( to_string(i) );
}


Tracer &Tracer::operator()(const exception &e)
{
    return operator()( e.what() );
}


void Tracer::Enable( bool e )
{
    enable = e;
}


void Tracer::MaybePrintEndl()
{
    if( require_endl ) 
    {
        fprintf(stderr, "\n");
        require_endl = false;
    }   
}


void Tracer::MaybePrintBanner()
{
    if( require_banner && (file != "" || line != 0 || function != "") )
    {
        Descend::Indent();
        fprintf( stderr, "----%s:%d in %s()\n", file, line, function);
        require_banner = false;
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





// Make BOOST_ASSERT work (we don't use them but other code might)
void boost::assertion_failed(char const * expr, char const * function, char const * file, long line)
{
    Tracer::MaybePrintEndl();
    Tracer( file, line, function, Tracer::FORCE )( "BOOST ASSERTION FAILED: %s\n\n", expr );
    InfernoAbort();
}


