#ifndef SYSTEMC_EXTENSIONS_H
#define SYSTEMC_EXTENSIONS_H

// Extend SystemC in a few ways.
// Included directly by rendered code for post-transform test builds. 
// Included indirectly (via isystemc.h) for pre-transform test builds.

#include <systemc.h>

#define SC_PORT(X) sc_port<X>
extern sc_trace_file *GetSCTraceFP();

void cease( unsigned char exit_code );

#endif // guard

