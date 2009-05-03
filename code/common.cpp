#include "trace.hpp"
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <stack>
#include <stdarg.h>

string SSPrintf(const char *fmt, ...)
{
    char cs[256];

    va_list vl;
    va_start( vl, fmt );
    vsnprintf( cs, sizeof(cs), fmt, vl );
    va_end( vl );
    
    return string(cs);
}

AutoPush::AutoPush( stack<T> &s, const T &t ) : st(s)
{
    st.push(t);
    check_t = t;
}

AutoPush::~AutoPush()
{
    ASSERT( st.top() == check_t );
    st.pop();
}

#endif
