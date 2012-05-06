#include "standard.hpp"
#include "trace.hpp"
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

void CommonTest()
{
	// Test the FOREACH macro in case it's our dodgy home made one
	deque<int> d;
	deque<int> *pd = &d;
	d.push_back(100);
	d.push_back(101);
	d.push_back(102);
	d.push_back(103);
	deque<int> r;
	FOREACH( int i, *pd )
	    r.push_back(i);
	ASSERT( r.size() == 4 );
    ASSERT( r[0] == 100 );
    ASSERT( r[1] == 101 );
    ASSERT( r[2] == 102 );
    ASSERT( r[3] == 103 );
}

string Traceable::GetName() const
{
    return CPPFilt( typeid( *this ).name() );
}

string Traceable::GetAddr() const
{
    return SSPrintf("@%p", this);
}

Traceable::operator string() const
{
    return GetName() + GetAddr(); // name plus pointer
}

