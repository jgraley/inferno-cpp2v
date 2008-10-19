#include "trace.hpp"
#include <boost/assert.hpp>
#include <stdarg.h>
#include "read_args.hpp"

Tracer::Tracer( const char *f, int l, const char *fu ) :
    file( f ),
    line( l ),
    function( fu )        
{
}

void Tracer::operator()()
{
    if( !ReadArgs::trace )
        return;

    printf("%s:%d in %s()\n", file, line, function);
}

void Tracer::operator()(const char *fmt, ...)
{
    if( !ReadArgs::trace )
        return;
    
    va_list vl;
    va_start( vl, fmt );
    printf("%s:%d in %s()\n    ", file, line, function);
    vprintf( fmt, vl );
    va_end( vl );
}

void boost::assertion_failed(char const * expr, char const * function, char const * file, long line)
{
    Tracer( file, line, function )( "Assertion failed: %s\n\n", expr );

    // The C library provides abort(), but I'm not getting a stack dump under cygwin
    (*(int*)-1)++; 
}
