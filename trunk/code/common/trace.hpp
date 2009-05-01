#ifndef TRACE_HPP
#define TRACE_HPP

#include <string>
using namespace std;

/*
 * How it works:
 *
 * TRACE
 * We use a functor class "Tracer" to make TRACE look like a function that works
 * just like printf(). Note that Boost provides a multi-platform "name of 
 * currrent function" macro, which we use.
 *
 * ASSERT
 * Boost asserts due to shared_ptr errors happen quite a lot. We compile with
 * BOOST_ENABLE_ASSERT_HANDLER defined which makes Boost call 
 * boost::assertion_failed() when its own BOOST_ASSERT() fails. We fill this
 * in to print a message (using our Tracer class), and then crash in a way
 * that assures a stack dump. We define our own ASSERT() to use BOOST_ASSERT().
 */

class Tracer
{
public:
    enum Flags
    {
        FORCE = 1,   // Generate the output even when -t is not specfied on the command line
        DISABLE = 2, // Do nothing
        ABORT = 4    // Crash out in destructor
    };
    Tracer( const char *f, int l, const char *fu, Flags fl=(Flags)0, const char *c=0 );
    ~Tracer();
    Tracer &operator()();
    Tracer &operator()(const char *fmt, ...);
    Tracer &operator()(const string &s); // not a printf because of risk of accidental format specifiers
    // TODO ostream support?
    static void EndContinuation();
    
private:
    const char * const file;
    const int line;
    const char * const function;
    Flags flags;
    static bool continuation;
};

#define INFERNO_CURRENT_FUNCTION __func__
// can be BOOST_CURRENT_FUNCTION if you want full signature but I find
// it can get in the way

#define TRACE Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION )

// Use BOOST_ASSERT, but be sure to place an abort() in line, so that we
// don't get "missing return" errors if there's no return after ASSERT(0).
// On some systems, abort() won't cause a stack dump, so we do somehting
// nasty in our boost::assertion_failed() implmentation in trace.cpp
#define ASSERT(CONDITION) do { BOOST_ASSERT(CONDITION); if(!(CONDITION)) abort(); } while(0)


// New assert uses functor. Can be used as ASSERT(cond); or ASSERT(cond)(printf args);
// but ASSERF(0) won't produce an abort() so we'll get return value warnings. Maybe
// do a subclass AlwaysDieTracer whose destructor is inline and abort()s every time?
#define ASSERTF(CONDITION) Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, (CONDITION)?Tracer::DISABLE:(Tracer::Flags)(Tracer::ABORT|Tracer::FORCE), #CONDITION )

#endif
