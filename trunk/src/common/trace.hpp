#ifndef TRACE_HPP
#define TRACE_HPP

#include <string>
#include <typeinfo> 
#include "read_args.hpp"
#include "hit_count.hpp" 
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

    static void EndContinuation();
    
    class Descend
    {
    public:
    	inline Descend() { depth++; }
    	inline ~Descend() { depth--; }
    	static void Indent();
    private:
        static int depth;
    };

private:
    const char * const file;
    const int line;
    const char * const function;
    Flags flags;
    static bool continuation;
};

class Traceable
{
public:
	static string CPPFilt( string s );
	virtual operator string() const;
};

#define INFERNO_CURRENT_FUNCTION __func__
// can be BOOST_CURRENT_FUNCTION if you want full signature but I find
// it can get in the way

#define TRACE if(ReadArgs::trace) Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION )

// New assert uses functor. Can be used as ASSERT(cond); or ASSERT(cond)(printf args);
#define ASSERT(CONDITION) if(!(CONDITION)) Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, (Tracer::Flags)(Tracer::ABORT|Tracer::FORCE), #CONDITION )

// This one does an abort() in-line so you don't get "missing return" warning (which
// we make an error). You can supply a message but no printf() formatting or arguments or std::string.
#define ASSERTFAIL(MESSAGE) do { Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, (Tracer::Flags)(Tracer::ABORT|Tracer::FORCE), #MESSAGE ); abort(); } while(0);

#define INDENT Tracer::Descend indent_;

#endif

