#ifndef COMMON_HPP
#define COMMON_HPP

#include "standard.hpp"
#include "trace.hpp"

// Pushes element t of type T onto stack s, then pops again in destructor
template< typename T >
class AutoPush
{
public:
    AutoPush( stack<T> &s, const T &t ) : st(s)
    {
        st.push(t);
    }    
    ~AutoPush()
    {
        st.pop();
    }
    
private:
    std::stack<T> &st;
};


template< typename S >
inline void InsertSolo( S &s, const typename S::value_type &p )
{
    auto pr = s.insert( p );
    ASSERT( pr.second )(p)(" already in container ")(s);
}


// Union two sets, without the pain of iterators
// Union two maps. Second overrules first!!
template< typename S1, typename S2 >
inline S1 UnionOf( const S1 &s1, const S2 &s2 )
{
    S1 result;
    result = s1;
    for( auto x : s2 )
        result.insert(x);
    return result; 
}


// Union any compatible associative containers, ordered
// or otherwise. Sets must be disjoint.
template< typename S1, typename S2 >
inline S1 UnionOfSolo( const S1 &s1, const S2 &s2 )
{
    S1 result;
    result = s1;
    for( auto x : s2 )
    {
        auto p = result.insert(x);
        ASSERT( p.second );
    }
    return result; 
}


// Intersect any compatible associative containers, ordered
// or otherwise.
template< typename S1, typename S2 >
inline S1 IntersectionOf( const S1 &s1, const S2 &s2 )
{
    S1 result;
    for( auto x : s2 )
        if( s1.count(x) > 0 )
            result.insert(x);                     
    return result; 
}


// Intersect set with complement, any compatible associative 
// containers, ordered or otherwise.
template< typename S1, typename S2 >
inline S1 DifferenceOf( const S1 &s1, const S2 &s2 )
{
    S1 result;
    result = s1;
    for( auto x : s2 )
        result.erase(x);                 
    return result; // There, much nicer!
}    


// Intersect set with complement, any compatible associative 
// containers, ordered or otherwise. s2 must be subset of s1.
template< typename S1, typename S2 >
inline S1 DifferenceOfSolo( const S1 &s1, const S2 &s2 )
{
    S1 result;
    result = s1;
    for( auto x : s2 )
    {
        int n = result.erase(x);                 
        ASSERT( n > 0 );
    }
    return result; // There, much nicer!
}    


template< typename T0, typename T1 >
inline list< pair<typename T0::value_type, typename T1::value_type> > Zip( const T0 &x0, const T1 &x1 )
{
    ASSERT( x0.size() == x1.size() );
    list< pair<typename T0::value_type, typename T1::value_type> > result;
    typename T0::const_iterator it0 = x0.begin();
    typename T1::const_iterator it1 = x1.begin();
    while( it0 != x0.end() )
        result.push_back( make_pair(*it0++, *it1++) );
    return result;
}    


template< typename KEY >
inline void InsertSolo( set<KEY> &s, const typename set<KEY>::value_type &x )
{
    ASSERT( s.count(x) == 0 )(x)(" already in set ")(s);
    s.insert( x );
}


template<class C, class IT>
void AdvanceWithWrap(const C &c, IT& it, int n)
{
    while (n > 0) {
        --n;
        ++it;
        if( it == c.end() )
            it = c.begin();            
    }
    while (n < 0) {
        ++n;
        if( it == c.begin() )
            it = c.end();            
        --it;
    }
}


template< typename T >
list<T> operator+( list<T> l, list<T> r ) // Just like in Python!
{
    // Note: both l and r are modified by this algo
    l.splice( l.begin(), r );
    return l;
}

typedef int CompareResult;
static const CompareResult EQUAL = 0;

template<typename T>
class ScopedAssign
{
public:    
    ScopedAssign( T &var_, const T &newval ) :
        var(var_),
        oldval(var)
    {            
        var = newval;
    }
    
    ~ScopedAssign()
    {
        var = oldval;
    }
    
private:
    T &var;
    const T oldval;
};


#endif
