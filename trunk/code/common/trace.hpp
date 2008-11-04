#ifndef TRACE_HPP
#define TRACE_HPP

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
        FORCE = 1 // Generate the output even when -t is not specfied on the command line
    };
    Tracer( const char *f, int l, const char *fu, Flags fl=(Flags)0 );
    void operator()();
    void operator()(const char *fmt, ...);
    // TODO ostream support?
    
private:
    const char * const file;
    const int line;
    const char * const function;
    Flags flags;
};

#define INFERNO_CURRENT_FUNCTION __func__
// can be BOOST_CURRENT_FUNCTION if you want full signature but I find
// it can get in the way

#define TRACE Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION )
#define ASSERT BOOST_ASSERT

#endif
