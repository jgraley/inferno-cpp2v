#ifndef COMMON_HPP
#define COMMON_HPP

#include "standard.hpp"
#include "trace.hpp"

#include <string>
#include <functional>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

string GetInnermostTemplateParam( string s );
string RemoveAllTemplateParam( string s );
string RemoveOneOuterScope( string s );
void RemoveCommonPrefix( string &s1, string &s2 );
string RemoveOuterTemplate( string s );

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
    ASSERT( pr.second )("Already in container");
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
template< typename T1, typename T2 >
inline set<T1> DifferenceOf( const set<T1> &s1, const set<T2> &s2 )
{
    set<T1> result;
    result = s1;
    for( auto x : s2 )
        result.erase(x);                 
    return result; // There, much nicer!
}    


// Intersect set with complement, any compatible associative 
// containers, ordered or otherwise.
template< typename T1, typename T2 >
inline map<T1, T2> DifferenceOf( const map<T1, T2> &s1, const map<T1, T2> &s2 )
{
    map<T1, T2> result;
    result = s1;
    for( auto x : s2 )
        result.erase(x.first); // this .first is why we need separate version for map     
    return result; // There, much nicer!
}    


// Intersect set with complement, any compatible associative 
// containers, ordered or otherwise. s2 must be subset of s1.
template< typename T1, typename T2 >
inline set<T1> DifferenceOfSolo( const set<T1> &s1, const set<T2> &s2 )
{
    set<T1> result;
    result = s1;
    for( auto x : s2 )
    {
        int n = result.erase(x);                 
        ASSERT( n > 0 );
    }
    return result; // There, much nicer!
}    


// Intersect set with complement, any compatible associative 
// containers, ordered or otherwise. s2 must be subset of s1.
template< typename T1, typename T2 >
inline map<T1, T2> DifferenceOfSolo( const map<T1, T2> &s1, const map<T1, T2> &s2 )
{
    map<T1, T2> result;
    result = s1;
    for( auto x : s2 )
    {
        int n = result.erase(x.first); // this .first is why we need separate version for map                      
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


template< typename C1, typename C2 >
inline bool IsSubset( const C1 &c1, const C2 &c2 )
{
    return DifferenceOf( c1, c2 ).empty();
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
    l.splice( l.end(), r );
    return l;
}


template< typename T >
vector<T> operator+( vector<T> l, vector<T> r ) // Just like in Python!
{
    l.insert(l.end(), r.begin(), r.end());
    return l;
}


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


// Acting on a container such as [1, 2, 3, 4], will call func with (1, 2), 
// then (2, 3), then (3, 4). No iteratons if size() is 0 or 1.
// Note: using on set/map with arbitrary ordering can amplify reproducibility
// problems because the set of pairs actually changes membership, not just
// the order seen.
template<typename T>
void ForOverlappingAdjacentPairs( const T &container, 
                                  function<void(const typename T::value_type &first, 
                                                const typename T::value_type &second)> func) 
{
    const typename T::value_type *prev_x = nullptr;
	for( const auto &x : container )
    {
        if( prev_x )
            func( *prev_x, x );
 		prev_x = &x;
	}
}


// Acting on a container such as [1, 2, 3, 4], will call func with 
// (2, 1), (3, 1), (4, 1), (3, 2), (4, 2), (4, 3), i.e. n(n-1)/2 iterations.
// No iterations if size() is 0 or 1.
template<typename T>
void ForAllCommutativeDistinctPairs( const T &container, 
                                     function<void(const typename T::value_type &first, 
                                                   const typename T::value_type &second)> func ) 
{
	for( typename T::const_iterator oit=container.begin(); oit != container.end(); ++oit )
    {
        // Starting at oit gets us "Commutative"
        for( typename T::const_iterator iit=oit; iit != container.end(); ++iit )
        {
            if( oit != iit ) // Gets us "Distinct"
                func( *iit, *oit );
 		}
	}
}


// Acting on a container such as [1, 2, 3], will call func with 
// (1, 2), (1, 3), (2, 1), (2, 3), (3, 1), (3, 2), i.e. n(n-1) iterations.
// No iterations if size() is 0 or 1.
template<typename T>
void ForAllDistinctPairs( const T &container, 
                          function<void(const typename T::value_type &first, 
                                        const typename T::value_type &second)> func ) 
{
	for( typename T::const_iterator oit=container.begin(); oit != container.end(); ++oit )
    {
        // Starting at oit gets us "Commutative"
        for( typename T::const_iterator iit=container.begin(); iit != container.end(); ++iit )
        {
            if( oit != iit ) // Gets us "Distinct"
                func( *iit, *oit );
 		}
	}
}


template<typename T, class COMPARE = less<typename T::value_type>>
set<typename T::value_type, COMPARE> ToSet( const T&c )
{
    set<typename T::value_type, COMPARE> s;
    for( typename T::value_type x : c )
        s.insert(x);  // later takes priority
    return s;
}


template<typename T, class COMPARE = less<typename T::value_type>>
set<typename T::value_type, COMPARE> ToSetSolo( const T&c )
{
    set<typename T::value_type, COMPARE> s;
    for( typename T::value_type x : c )
        InsertSolo(s, x);
    return s;
}


template<typename T>
list<typename T::value_type> ToList( const T&c )
{
    list<typename T::value_type> l;
    for( typename T::value_type x : c )
        l.push_back(x);  // freezes the set ordering
    return l;
}


template<typename T>
vector<typename T::value_type> ToVector( const T&c )
{
    vector<typename T::value_type> v;
    for( typename T::value_type x : c )
        v.push_back(x);  // freezes the set ordering
    return v;
}


template<typename T>
const typename T::value_type &OnlyElementOf( const T&c )
{
    ASSERT( c.size()==1 );
    return *(c.begin());
}


// Provide somehting like front() that works on sets and maps
template<typename T>
const typename T::value_type &FrontOf( const T&c )
{
    ASSERT( !c.empty() );
    return *(c.begin());
}

// ----------------------- make_unique<>() -----------------------

template<class T> 
struct _Unique_if 
{
    typedef unique_ptr<T> _Single_object;
};


template<class T> 
struct _Unique_if<T[]> 
{
    typedef unique_ptr<T[]> _Unknown_bound;
};


template<class T, size_t N> 
struct _Unique_if<T[N]> {
    typedef void _Known_bound;
};


template<class T, class... Args>
typename _Unique_if<T>::_Single_object make_unique(Args&&... args) 
{
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
}


template<class T>
typename _Unique_if<T>::_Unknown_bound make_unique(size_t n) 
{
    typedef typename remove_extent<T>::type U;
    return unique_ptr<T>(new U[n]());
}


template<class T, class... Args>
typename _Unique_if<T>::_Known_bound make_unique(Args&&...) = delete;


// Get a "that" pointer, which is a pointer of the same type as "this"
// for multi-methods. NULL not allowed, wrong type not allowed.
#define GET_THAT_POINTER(BT) ({ \
    auto bt( BT ); \
    ASSERT( bt ); \
    auto tp = dynamic_cast<decltype(this)>(bt); \
    ASSERT( tp ); \
    tp; \
})    


// To use with set, map and algorithms like min_element when the
// container/range actually contains pointers (or shared or unique pointers)
// to the "real objects" which possess the operator<() that we require.
// You will probably need to explicitly instantiate the template, and this is
// done by providing the pointer type (not the pointed-to type).
template<typename POINTER_TYPE>
bool DereferencingCompare( const POINTER_TYPE &a, 
                           const POINTER_TYPE &b )
{
    return *a < *b;
}                                      


// Convert a pair of equal-length vectors to a map. Keys must be unique.
template<typename KEYS_TYPE, typename VALUES_TYPE>
map<typename KEYS_TYPE::value_type, typename VALUES_TYPE::value_type> ZipToMap( const KEYS_TYPE &keys, const VALUES_TYPE &values )
{
    ASSERT( keys.size() == values.size() );
    map<typename KEYS_TYPE::value_type, typename VALUES_TYPE::value_type> my_map;
    typename KEYS_TYPE::const_iterator kit;
    typename VALUES_TYPE::const_iterator vit; 
    for( kit = keys.begin(), vit = values.begin();
         kit != keys.end();
         ++kit, ++vit )
        InsertSolo( my_map, make_pair(*kit, *vit) );
    return my_map;
}


// Overwrite some values of a vector as dictated by a map. dest_vec must already be big enough.
template<typename VALUE_TYPE>
void ScatterInto( vector<VALUE_TYPE> &dest_vec, const map<int, VALUE_TYPE> &my_map )
{
    for( auto p : my_map )
        dest_vec.at(p.first) = p.second;
}


// Lexicographical increment over a vector of int.
bool IncrementIndices( vector<int> &indices, int index_range );


// Lambda powered loop loops over some space raised to the power of degree. index_range
// contains all the values in the space, and their actual type is templated for your convenience.
template<typename AXIS_TYPE>
void ForPower( int degree, vector<AXIS_TYPE> index_range, function<void(vector<AXIS_TYPE>)> body )
{
    vector<int> int_indices( degree, 0 );
    vector<AXIS_TYPE> typed_indices( degree );
    do
    {
        // Copy from the int type that we can increment to the desired AXIS_TYPE
        for( int axis=0; axis<degree; axis++ )
            typed_indices[axis] = index_range.at( int_indices.at(axis) );
        body( typed_indices );
    }
    while( !IncrementIndices( int_indices, index_range.size() ) );
}


// The value range of a bool, to be used with ForPower
extern const vector<bool> index_range_bool;

#endif
