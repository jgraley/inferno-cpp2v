#include "trace.hpp"
#include <boost/assert.hpp>
#include <stdarg.h>
#include "read_args.hpp"

bool Tracer::continuation = false;

inline void InfernoAbort()
{
    fflush( stdout ); // might help if the crash kills buffered output

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
        printf("\n    Assertion failed: %s\n", c);
        printf( "    %s:%d in %s()\n", file, line, function);
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
    if( (flags & DISABLE) || !(ReadArgs::trace || (flags & FORCE)) )
        return *this;
 
    EndContinuation();

    printf( "    %s:%d in %s()\n", file, line, function);
    
    return *this;
}

Tracer &Tracer::operator()(const char *fmt, ...)
{
    if( (flags & DISABLE) || !(ReadArgs::trace || (flags & FORCE)) )
        return *this;

    va_list vl;
    va_start( vl, fmt );
    if( !continuation ) 
        printf( "    %s:%d in %s()\n", file, line, function);
    vprintf( fmt, vl );
    va_end( vl );
    
    continuation = (fmt[strlen(fmt)-1]!='\n');
    return *this;
}

Tracer &Tracer::operator()(const string &s)
{    
    return operator()("%s", s.c_str());
}

void Tracer::EndContinuation()
{
    if( continuation ) 
    {
        printf("\n");
        continuation = false;
    }   
}

// Make BOOST_ASSERT work (we don't use them but other code might)
void boost::assertion_failed(char const * expr, char const * function, char const * file, long line)
{
    Tracer::EndContinuation();
    Tracer( file, line, function, Tracer::FORCE )( "Assertion failed: %s\n\n", expr );
    InfernoAbort();
}
