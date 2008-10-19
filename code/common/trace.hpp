
#ifndef TRACE_HPP
#define TRACE_HPP

#include <stdarg.h>
#include "read_args.hpp"

class Tracer
{
public:
    Tracer( const char *f, int l, const char *fu ) :
        file( f ),
        line( l ),
        function( fu )        
    {
    }
    void operator()()
    {
        printf("%s:%d in %s()\n", file, line, function);
    }
    void operator()(const char *fmt, ...)
    {
        if( !ReadArgs::trace )
            return;
        
        va_list vl;
        va_start( vl, fmt );
        printf("%s:%d in %s()\n    ", file, line, function);
        vprintf( fmt, vl );
        va_end( vl );
    }
    // TODO ostream support?
private:
    const char * const file;
    const int line;
    const char * const function;
};

#define TRACE Tracer( __FILE__, __LINE__, __func__ )

#endif
