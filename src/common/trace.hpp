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
#include <exception>
	
#define CONTAINER_SEP ",\n"    
    
    
string Trace(const Traceable &t); 
string Trace(bool b); 
string Trace(int i); 
string Trace(size_t i); 
string Trace(const exception &e); 

template<typename T>
string Trace(const list<T> &l) 
{
    string str = "[";
    bool first = true;
    for( auto x : l )
    {
        if( !first )
            str += CONTAINER_SEP;
        str += Trace(x);
        first = false;
    }
    return str + "]";
}


template<typename T>
string Trace(const vector<T> &l) 
{
    string str = "[";
    bool first = true;
    for( auto x : l )
    {
        if( !first )
            str += CONTAINER_SEP;
        str += Trace(x);
        first = false;
    }
    return str + "]";
}


template<typename T>
string Trace(const set<T> &s) 
{
    string str = "{";
    bool first = true;
    for( auto x : s )
    {
        if( !first )
            str += CONTAINER_SEP;
        str += Trace(x);
        first = false;            
    }
    return str + "}";
}


template<typename T>
string Trace(const multiset<T> &s) 
{
    string str = "{";
    bool first = true;
    for( auto x : s )
    {
        if( !first )
            str += CONTAINER_SEP;
        str += Trace(x);
        first = false;            
    }
    return str + "}";
}


template<typename TK, typename TV>
string Trace(const map<TK, TV> &m) 
{
    string str = "{";
    bool first = true;
    for( auto p : m )
    {
        if( !first )
            str += CONTAINER_SEP;
        str += Trace(p.first);
        str += ": ";
        str += Trace(p.second);
        first = false;            
    }
    return str + "}";
}


template<typename TF, typename TS>
string Trace(const pair<TF, TS> &p) 
{
    string str = "(";
    str += Trace(p.first);
    str += ", ";
    str += Trace(p.second);
    return str + ")";
}


template<typename T>
string Trace(const T *p) 
{
    if( p )
    {
        return string("&") + Trace(*p);        
    }
    else
    {
        return string("NULL");
    }
}


template<typename T>
string Trace(shared_ptr<T> p) 
{
    return Trace(p.get());        
}
    
    
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
    Tracer &operator()(const string &s); // not a printf because of risk of accidental format specifiers
    Tracer &operator()(const char *fmt, ...);

    template<typename T>
    Tracer &operator()(const T &x)
    {
        return operator()( Trace(x) );
    }
    
    static void Enable( bool e ); ///< enable/disable tracing, only for top level funciton to call, overridden by flags
    inline static bool IsEnabled() { return enable; }
    static string GetPrefix() { return Descend::pre; }
    
    class Descend
    {
    public:
    	Descend( string s="." );
    	~Descend();
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

    static void MaybePrintEndl();

    static void SetStep( int s );

private:    
    void PrintPrefix();
    void MaybePrintBanner();

    const char * const file;
    const int line;
    const char * const function;
    Flags flags;
    static bool require_endl;
    static bool require_banner;
    static bool enable;
    static int current_step;
};


#define INFERNO_CURRENT_FUNCTION __func__
// can be BOOST_CURRENT_FUNCTION if you want full signature but I find
// it can get in the way

#define TRACE if(Tracer::IsEnabled()) Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION )
#define FTRACE Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, Tracer::FORCE )
#define TRACEC if(Tracer::IsEnabled()) Tracer()
#define FTRACEC Tracer(Tracer::FORCE)

// New assert uses functor. Can be used as ASSERT(cond); or ASSERT(cond)(printf args);
#define ASSERT(CONDITION) if(!(CONDITION)) Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, (Tracer::Flags)(Tracer::ABORT|Tracer::FORCE), #CONDITION )

// This one does an abort() in-line so you don't get "missing return" warning (which
// we make an error). You can supply a message but no printf() formatting or arguments or std::string.
#define ASSERTFAIL(MESSAGE) do { Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, (Tracer::Flags)(Tracer::ABORT|Tracer::FORCE), #MESSAGE ); abort(); } while(0);

#define INDENT(P) Tracer::Descend indent_(P); HITP(Tracer::GetPrefix());

#endif

