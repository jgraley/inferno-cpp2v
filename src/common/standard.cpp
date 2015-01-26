#include "standard.hpp"
#include "trace.hpp"
#include <stdarg.h>
#include <cxxabi.h>

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
    return string();
}

Traceable::operator string() const
{
    return GetName() + GetAddr(); // name plus pointer
}

string Traceable::CPPFilt( string s )
{
	int status;
	char *ps;
	// Use GCC extension to demangle based on the present ABI
	ps = abi::__cxa_demangle(s.c_str(), 0, 0, &status);
    s = ps;
    free(ps); // as ordained
    return s;
}

SerialNumber::SNType SerialNumber::master_serial;

SerialNumber::SerialNumber() :
    serial( master_serial )
{
#if 0
    static FILE *fp;
    if( master_serial==0 )
    {
        fp = fopen("tps.txt", "r");
    }
    if( fp )
    {
        void *a;
        int n = fscanf(fp, "%p\n", &a);
        if( n==1 )
            ASSERT( __builtin_return_address(1)==a )("Mismatch at serial %lu: got %p expcted %p", master_serial, __builtin_return_address(1), a);
    }
    printf("%p\n", __builtin_return_address(1));
#endif    
    master_serial++;
}    

