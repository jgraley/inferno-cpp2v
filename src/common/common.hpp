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


// Union two maps. Second overrules first!!
template< typename KEY, typename T >
inline map<KEY, T> MapUnion( const map<KEY, T> &m1, const map<KEY, T> &m2 )
{
	typedef pair<KEY, T> Pair;
    map<KEY, T> result = m1;
    for( Pair p : m2 )
    {
		result[p.first] = p.second;
    }
    return result; 
}


template< typename KEY, typename T >
inline void InsertSolo( map<KEY, T> &m, const typename map<KEY, T>::value_type &p )
{
    ASSERT( m.count(p.first) == 0 )(p)(" already in map ")(m);
    m.insert( p );
}


// Union two sets, without the pain of iterators
template< typename KEY >
inline set<KEY> SetUnion( const set<KEY> &s1, const set<KEY> &s2 )
{
    set<KEY> result;
    std::set_union( s1.begin(), s1.end(), 
                    s2.begin(), s2.end(),
                    std::inserter(result, result.end()) );
    return result; 
}


// Intersect two sets, without the pain of iterators
template< typename KEY >
inline set<KEY> SetIntersection( const set<KEY> &s1, const set<KEY> &s2 )
{
    set<KEY> result;
    std::set_intersection( s1.begin(), s1.end(), 
                            s2.begin(), s2.end(),
                            std::inserter(result, result.end()) );
    return result; 
}


// Intersect set with complement, without the pain of iterators
template< typename KEY >
inline set<KEY> SetDifference( const set<KEY> &s1, const set<KEY> &s2 )
{
    set<KEY> result;
    std::set_difference( s1.begin(), s1.end(), 
                         s2.begin(), s2.end(),
                         std::inserter(result, result.end()) );
    return result; // There, much nicer!
}    


template< typename KEY >
inline void InsertSolo( set<KEY> &s, const typename set<KEY>::value_type &x )
{
    ASSERT( s.count(x) == 0 )(x)(" already in set ")(s);
    s.insert( x );
}

#endif
