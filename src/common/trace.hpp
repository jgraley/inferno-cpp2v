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
#include <unordered_set>
#include <unordered_map>
#include <exception>
	
#define CONTAINER_SEP ",\n"    
    
////////////////////////// Trace() free functions //////////////////////////
    
string Trace(const Traceable &t); 
string Trace(string s); 
string Trace(bool b); 
string Trace(int i); 
string Trace(size_t i); 
string Trace(const exception &e); 
string Trace(const void *p); 
string Trace(const Progress &progress); 

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
    
    
template<typename TF, typename TS>
string Trace(const pair<TF, TS> &p) 
{
    list<string> elts = { Trace(p.first), Trace(p.second) };
    return Join(elts, "(", ", ", ")");
}


template<typename T>
string Trace(const vector<T> &l) 
{
    list<string> elts;
    for( auto x : l )
        elts.push_back( Trace(x) );
    return Join( elts, "[", CONTAINER_SEP, "]" );
}


template<typename T>
string Trace(const list<T> &l) 
{
    list<string> elts;
    for( auto x : l )
        elts.push_back( Trace(x) );
    return Join( elts, "[", CONTAINER_SEP, "]" );
}


template<typename T>
string Trace(const set<T> &s) 
{
    list<string> elts;
    for( auto x : s )
        elts.push_back( Trace(x) );
    return Join( elts, "{", CONTAINER_SEP, "}" );
}


template<typename TK, typename TV>
string Trace(const map<TK, TV> &m) 
{
    list<string> elts;
    for( auto p : m )
        elts.push_back( Trace(p.first) + ": " + Trace(p.second) );
    return Join( elts, "{", CONTAINER_SEP, "}" );
}


template<typename T>
string Trace(const multiset<T> &s) 
{
    list<string> elts;
    for( auto x : s )
        elts.push_back( Trace(x) );
    return Join( elts, "{", CONTAINER_SEP, "}" );
}


template<typename TK, typename TV>
string Trace(const multimap<TK, TV> &m) 
{
    list<string> elts;
    for( auto p : m )
        elts.push_back( Trace(p.first) + ": " + Trace(p.second) );
    return Join( elts, "{", CONTAINER_SEP, "}" );
}


template<typename T>
string Trace(const unordered_set<T> &s) 
{
    list<string> elts;
    for( auto x : s )
        elts.push_back( Trace(x) );
    return Join( elts, "{", CONTAINER_SEP, "}" );
}


template<typename TK, typename TV>
string Trace(const unordered_map<TK, TV> &m) 
{
    list<string> elts;
    for( auto p : m )
        elts.push_back( Trace(p.first) + ": " + Trace(p.second) );
    return Join( elts, "{", CONTAINER_SEP, "}" );
}


template<typename T>
string Trace(const unordered_multiset<T> &s) 
{
    list<string> elts;
    for( auto x : s )
        elts.push_back( Trace(x) );
    return Join( elts, "{", CONTAINER_SEP, "}" );
}


template<typename TK, typename TV>
string Trace(const unordered_multimap<TK, TV> &m) 
{
    list<string> elts;
    for( auto p : m )
        elts.push_back( Trace(p.first) + ": " + Trace(p.second) );
    return Join( elts, "{", CONTAINER_SEP, "}" );
}

////////////////////////// NewtonsCradle //////////////////////////

/// Interface for objects that can be repeat-called using eg ob(a)(b)(c)...
/// It looks like a Newton's Cradle, right?
class NewtonsCradle
{
public:    
    // The actual work should be done in here
    virtual NewtonsCradle &operator()(const string &s) = 0; 

    // And optionally in here if the defualt behaviour is no good
    virtual NewtonsCradle &operator()();    
    NewtonsCradle &operator()(const char *fmt, ...);    
    
    // But not here because you can't override a template function
    template<typename T>
    NewtonsCradle &operator()(const T &x)
    {
        return operator()( Trace(x) );
    }    
};

////////////////////////// Tracer //////////////////////////

class Tracer : public NewtonsCradle
{
public:
    using NewtonsCradle::operator();
    enum Flags
    {
        FORCE = 1,   // Generate the output even when not enabled
        DISABLE = 2, // Do nothing
        ABORT = 4    // Crash out in destructor
    };
    Tracer( const char *f, int l, const char *fu, Flags fl=(Flags)0, const char *cond=0 );
    Tracer( Flags fl=(Flags)0, const char *c=0 );
    ~Tracer();
    virtual Tracer &operator()();
    virtual Tracer &operator()(const string &s); 

    static void Enable( bool e ); ///< enable/disable tracing, only for top level function to call, overridden by flags
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
};

////////////////////////// TraceTo //////////////////////////

class TraceTo : public NewtonsCradle
{
public:
    using NewtonsCradle::operator();

    TraceTo( string &str );
    TraceTo( ostream &osm );
    
    virtual TraceTo &operator()(const string &s); 

private:
    string * const p_str;
    ostream * const p_osm;
};

////////////////////////// Macro layer //////////////////////////

//
// Any time code wants to talk "out of band" to the user, go through
// a macro here, adding one if necessary so that
// (a) you can ensure you don't waste CPU on string processing except
//     where needed (trace enables/assert condition failed), and
// (b) you get decent file and line info so can locate the source
//     without resorting to gdb.
//
// Note: all of these except ASSERTFAIL use Newton's Cradle style args
// Note: else used to avoid ambiguating when used inside an if 
//

#define INFERNO_CURRENT_FUNCTION __func__
// can be BOOST_CURRENT_FUNCTION if you want full signature but I find
// it can get in the way

// Plain tracing...
#define INDENT(P) Tracer::Descend indent_(P); HITP(Tracer::GetPrefix());
#define TRACE if(!Tracer::IsEnabled()) {} else Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION )
#define FTRACE Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, Tracer::FORCE )
#define TRACEC if(!Tracer::IsEnabled()) {} else Tracer()
#define FTRACEC Tracer(Tracer::FORCE)

// Asserts and such...
#define ASSERT(CONDITION) if(CONDITION) {} else Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, (Tracer::Flags)(Tracer::ABORT|Tracer::FORCE), #CONDITION )

// This one does an abort() in-line so you don't get "missing return" warning (which
// we make an error). You can supply a message but no printf() formatting or arguments or std::string.
#define ASSERTFAIL(MESSAGE) do { Tracer( __FILE__, __LINE__, INFERNO_CURRENT_FUNCTION, (Tracer::Flags)(Tracer::ABORT|Tracer::FORCE), #MESSAGE ); abort(); } while(0);

#define STACK_BASE 0x7f0000000000ULL
#define ON_STACK(POINTER) (((uint64_t)(POINTER) & STACK_BASE) == STACK_BASE)
#define ASSERT_NOT_ON_STACK(POINTER) ASSERT( !ON_STACK(POINTER) )("\nObject at %p seems like it's probably on the stack, usually a bad sign\n", POINTER)

// Tracing onto a string...
#define TRACE_TO(DEST) if(!Tracer::IsEnabled()) {} else (TraceTo(DEST))
#define FTRACE_TO(DEST) (TraceTo(DEST))
#endif

