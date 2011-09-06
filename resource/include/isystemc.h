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

class sc_event
{ 
    void notify( void p1 );
};

class sc_module;
#define SC_MODULE(X) class X : public sc_module

class sc_interface;

void wait( void p1 );
void next_trigger( void p1 );
void SC_THREAD( void func );
void SC_CTHREAD( void func, void clock );
void SC_METHOD( void func );

/// Not really SystemC, but we detect exit and represent natively as Exit so as to 
/// get a way of extracting results from programs.
void exit( void p1 );

// Allow SC_CTOR to parse as a constructor
#define SC_CTOR(X) X( const char *name ) 

// Allow this symbol
void SC_ZERO_TIME;

#else
// Parsed by native compiler
// Reduce the macros we define outside of the SC standard to when SC expects.
// Introdude real SystemC header file
#include <systemc.h>

#define SC_PORT(X) sc_port<X>

#endif // inferno
#endif // guard

