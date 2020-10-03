#include "trace.hpp"
#include <boost/assert.hpp>
#include <stdarg.h>
#include <string.h>

bool Tracer::continuation = false;
bool Tracer::enable = false; ///< call Tracer::Enable(true) to begin tracing
string Tracer::Descend::pre;
string Tracer::Descend::last_traced_pre, Tracer::Descend::leftmost_pre;

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

inline void InfernoAbort()
{
    fflush( stderr ); 

    // The C library provides abort(), but I'm not getting a stack dump under cygwin
    (*(int*)-1)++; 
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
        EndContinuation();
        fprintf( stderr, "\n");
        Descend::Indent();
        fprintf( stderr, "----Assertion failed: %s\n", c);
        Descend::Indent();
        fprintf( stderr, "----%s:%d in %s()\n", file, line, function);
        continuation = true;
    }
}


Tracer::Tracer( Flags fl, char const *c ) :
    file( "" ),
    line( 0 ),
    function( "" ),
    flags( fl )
{
    // If we're going to abort, get this out first, then usual trace message if required as a continuation
    if( flags & ABORT )
    {
        EndContinuation();
        fprintf( stderr, "\n");
        Descend::Indent();
        fprintf( stderr, "----Assertion failed: %s\n", c);
        continuation = true;
    }
}


Tracer::~Tracer()
{
    if( flags & ABORT )
    {
        EndContinuation();
        InfernoAbort();
    }
}


Tracer &Tracer::operator()()
{
    if( (flags & DISABLE) || !(enable || (flags & FORCE)) )
        return *this;
 
    EndContinuation();
    if( file != "" || line != 0 || function != "" )
    {
        Descend::Indent();
        fprintf( stderr, "----%s:%d in %s()\n", file, line, function);
    }
    
    return *this;
}


Tracer &Tracer::operator()(const char *fmt, ...)
{
    if( (flags & DISABLE) || !(enable || (flags & FORCE)) )
        return *this;

    va_list vl;
    va_start( vl, fmt );
    if( !continuation ) 
    {    	
        if( file != "" || line != 0 || function != "" )
        {
            Descend::Indent();
            fprintf( stderr, "----%s:%d in %s()\n", file, line, function);
        }
        Descend::Indent();
    }
    vfprintf( stderr, fmt, vl );
    va_end( vl );
    
    continuation = (fmt[strlen(fmt)-1]!='\n');
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


Tracer &Tracer::operator()(const exception &e)
{
    return operator()( e.what() );
}


void Tracer::EndContinuation()
{
    if( continuation ) 
    {
        fprintf(stderr, "\n");
        continuation = false;
    }   
}


void Tracer::Enable( bool e )
{
    enable = e;
}


Tracer::Descend::Descend( string s ) : 
    os(pre.size()) 
{ 
    pre += s; 
    Tracer::EndContinuation(); 
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
    Tracer::EndContinuation();
    Tracer( file, line, function, Tracer::FORCE )( "Assertion failed: %s\n\n", expr );
    InfernoAbort();
}


