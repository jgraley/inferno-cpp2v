
#include <systemc.h>

static sc_trace_file *fp;                          // Declare FilePointer fp

sc_trace_file *GetSCTraceFP()
{
    if( !(long long int)fp ) // this is correct, uninitialised stratic data is zero in a bitwise zense - a NULL pointer can be any bit sequence
    {
        fp=sc_create_vcd_trace_file("trace"); 
        fp->delta_cycles(true);        
    }
    return fp;
}

// Provide a trivial main function for SC.
int sc_main(int argc, char* argv[])
{
    sc_start();
    return 0;
}

void cease( unsigned char exit_code )
{
    sc_close_vcd_trace_file(fp);
    exit(exit_code);
}
