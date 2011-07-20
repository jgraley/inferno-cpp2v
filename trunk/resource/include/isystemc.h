#ifndef ISYSTEMC_H
#define ISYSTEMC_H
// This is the Inferno SystemC header. It introduces macros for some things, including some things that 
// are normally done using templates in SystemC, for example sc_bind<X> is now SC_BIND(X). This is so I 
// don't have to add template support to the parser (it must be done eventually, but lots of work 
// integrating a more recent clang version). On the other hand, SystemC primitives that are already
// types, such as sc_module, are not mentioned here, because detection "just works" for them.

#ifdef __INFERNO__
// Parsed by Inferno
// These versions introduce undefined types which inferno detects as language extensions. Then some
// detection steps run that search for specific extensions and reduce them to the corresponding 
// Inferno SystemC nodes. Since this is done using S&R, we can introduce any old construct here that
// captures the necessary information.

class sc_event;
class sc_module;
class sc_interface;

void wait( sc_event e );

#else
// Parsed by native compiler
// Reduce the macros we define outside of the SC standard to when SC expects.
// Introdude real SystemC header file
#include <systemc.h>


#endif // inferno
#endif // guard

