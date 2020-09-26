#ifndef TRACE_HPP
#define TRACE_HPP

#include <string>
#include <typeinfo> 
#include "standard.hpp" 
#include "hit_count.hpp" 
using namespace std;

/*
 * How it works:
 *
 * TRACE
 * We use a functor class "Tracer" to make TRACE look like a function that works
 * just like printf(). Note that Boost provides a multi-platform "name of 
 * current function" macro, which we use.
 *
 * ASSERT
 * Boost asserts due to shared_ptr errors happen quite a lot. We compile with
 * BOOST_ENABLE_ASSERT_HANDLER defined which makes Boost call 
 * boost::assertion_failed() when its own BOOST_ASSERT() fails. We fill this
 * in to print a message (using our Tracer class), and then crash in a way
 * that assures a stack dump. We define our own ASSERT() to use BOOST_ASSERT().
 */

#include <list>
#include <set>
#include <map>

class Tracer
{
public:
    enum Flags
    {
        FORCE = 1,   // Generate the output even when not enabled
        DISABLE = 2, // Do nothing
        ABORT = 4    // Crash out in destructor
    };
    Tracer( const char *f, int l, const char *fu, Flags fl=(Flags)0, const char *c=0 );
    Tracer( Flags fl=(Flags)0, const char *c=0 );
    ~Tracer();
    Tracer &operator()();
    Tracer &operator()(const char *fmt, ...);
    Tracer &operator()(const string &s); // not a printf because of risk of accidental format specifiers
    Tracer &operator()(const Traceable &s); 
    Tracer &operator()(bool b); 

    template<typename T>
    Tracer &operator()(const list<T> &l) 
    {
        operator()("[");
        bool first = true;
        for( auto x : l )
        {
            if( !first )
                operator()(", ");
            operator()(x);
            first = false;
        }
        return operator()("]");
    }

    template<typename T>
    Tracer &operator()(const set<T> &s) 
    {
        operator()("{");
        bool first = true;
        for( auto x : s )
        {
            if( !first )
                operator()(", ");
            operator()(x);
            first = false;            
        }
        return operator()("}");
    }

    template<typename TK, typename TV>
    Tracer &operator()(const map<TK, TV> &m) 
    {
        operator()("{");
        bool first = true;
        for( auto p : m )
        {
            if( !first )
                operator()(", ");
            operator()(p.first);
            operator()(": ");
            operator()(p.second);
            first = false;            
        }
        return operator()("}");
    }

    template<typename T>
    Tracer &operator()(const T *p) 
    {
        operator()("&");
        return operator()(*p);        
    }

    static void EndContinuation();
    static void Enable( bool e ); ///< enable/disable tracing, only for top level funciton to call, overridden by flags
    inline static bool IsEnabled() { return enable; }
    static string GetPrefix() { return Descend::pre; }
    
    class Descend
    {
    public:
    	inline Descend( string s=" " ) : 
    	    os(pre.size()) 
    	{ 
            pre += s; 
            Tracer::EndContinuation(); 
        } 
    	inline ~Descend() 
    	{ 
            pre = pre.substr(0, os); 
            if( pre.size() < leftmost_pre.size() )
                leftmost_pre = pre;
        }
    	static void Indent();
    private:
        static string pre;
        static string last_traced_pre, leftmost_pre;
        const int os;
        friend class Tracer;
    };

    class RAIIEnable
    {
    public:
    	inline RAIIEnable( bool enable_ ) : 
    	    old_enable(enable) 
    	{ 
            enable = enable_; 
        } 
    	inline ~RAIIEnable() 
    	{ 
            enable = old_enable; 
        } 
    private:
        const bool old_enable;
    };

private:
    const char * const file;
    const int line;
    const char * const function;
    Flags flags;
    static bool continuation;
    static bool enable;
};

#define INFERNO_CURRENT_FUNCTION __func__
// can be BOOST_CURRENT_FUNCTION if you want full signature but I find
// it can get in the way

#define FTRACE Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, Tracer::FORCE )
#define TRACE if(Tracer::IsEnabled()) Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION )
#define TRACEC if(Tracer::IsEnabled()) Tracer()

// New assert uses functor. Can be used as ASSERT(cond); or ASSERT(cond)(printf args);
#define ASSERT(CONDITION) if(!(CONDITION)) Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, (Tracer::Flags)(Tracer::ABORT|Tracer::FORCE), #CONDITION )

// This one does an abort() in-line so you don't get "missing return" warning (which
// we make an error). You can supply a message but no printf() formatting or arguments or std::string.
#define ASSERTFAIL(MESSAGE) do { Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, (Tracer::Flags)(Tracer::ABORT|Tracer::FORCE), #MESSAGE ); abort(); } while(0);

#define INDENT(P) Tracer::Descend indent_(P); HITP(Tracer::GetPrefix());

#endif

