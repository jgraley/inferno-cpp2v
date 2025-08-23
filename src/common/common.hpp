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


// Returns an interator to that which was inserted
template< typename S >
inline typename S::iterator InsertSolo( S &s, const typename S::value_type &p )
{
    auto pr = s.insert( p );
    ASSERT( pr.second )("Already in container");
    return pr.first;
}


template< typename S >
inline void EraseSolo( S &s, const typename S::key_type &p )
{
    size_t n = s.erase( p );
    ASSERT( n==1 )("Unexpectedly erased %u elements matching ", n)(p)(" from ")(s);
}


// Union two sets, without the pain of iterators
template< typename S1, typename S2 >
inline S1 UnionOf( const S1 &s1, const S2 &s2 )
{
    S1 result( s1.key_comp() );
    set_union( s1.begin(), s1.end(), s2.begin(), s2.end(),
               std::inserter(result, result.begin()),
               s1.key_comp() );    
    return result; 
}


// Union two maps. Second overrules first!!
template< typename K, typename V, typename C >
inline map<K, V, C> UnionOf( const map<K, V, C> &m1, const map<K, V, C> &m2 )
{
    typename std::map<K, V, C>::key_compare kc1( m1.key_comp() );
    typename std::map<K, V, C> result( kc1 );
    auto kc2 = [&](const std::pair<K, V> &p1, const std::pair<K, V> &p2)
    { 
        return kc1(p1.first, p2.first); 
    };
    set_union( m1.begin(), m1.end(), m2.begin(), m2.end(),
               std::inserter(result, result.begin()),
               kc2 );    
    return result; 
}


// Union disjoint sets.
template< typename S1, typename S2 >
inline S1 UnionOfSolo( const S1 &s1, const S2 &s2 )
{
    S1 result( s1.key_comp() );
    set_union( s1.begin(), s1.end(), s2.begin(), s2.end(),
               std::inserter(result, result.begin()),
               s1.key_comp() );    
    ASSERT( result.size() == s1.size() + s2.size() )("Solo rule");
    return result; 
}


// Union any two disjoint maps.
template< typename K, typename V, typename C >
inline map<K, V, C> UnionOfSolo( const map<K, V, C> &m1, const map<K, V, C> &m2 )
{
    typename std::map<K, V, C>::key_compare kc1( m1.key_comp() );
    typename std::map<K, V, C> result( kc1 );
    auto kc2 = [&](const std::pair<K, V> &p1, const std::pair<K, V> &p2)
    { 
        return kc1(p1.first, p2.first); 
    };
    set_union( m1.begin(), m1.end(), m2.begin(), m2.end(),
               std::inserter(result, result.begin()),
               kc2 );    
    ASSERT( result.size() == m1.size() + m2.size() )("Solo rule");
    return result; 
}


// Intersect sets.
template< typename S1, typename S2 >
inline S1 IntersectionOf( const S1 &s1, const S2 &s2 )
{
    S1 result( s1.key_comp() );
    set_intersection( s1.begin(), s1.end(), s2.begin(), s2.end(),
                      std::inserter(result, result.begin()),
                      s1.key_comp() );    
    return result; 
}


// Intersect map with complement.
template< typename K, typename V, typename C >
inline map<K, V, C> IntersectionOf( const map<K, V, C> &m1, const map<K, V, C> &m2 )
{
    typename std::map<K, V, C>::key_compare kc1( m1.key_comp() );
    typename std::map<K, V, C> result( kc1 );
    auto kc2 = [&](const std::pair<K, V> &p1, const std::pair<K, V> &p2)
    { 
        return kc1(p1.first, p2.first); 
    };
    set_intersection( m1.begin(), m1.end(), m2.begin(), m2.end(),
                      std::inserter(result, result.begin()),
                      kc2 );    
    return result; 
}


// Intersect set with complement.
template< typename S1, typename S2 >
inline S1 DifferenceOf( const S1 &s1, const S2 &s2 )
{
    S1 result( s1.key_comp() );
    set_difference( s1.begin(), s1.end(), s2.begin(), s2.end(),
                    std::inserter(result, result.begin()),
                    s1.key_comp() );    
    return result; 
}    


// Intersect unordered_set with complement.
template< typename S >
inline unordered_set<S> DifferenceOf( const unordered_set<S> &s1, const unordered_set<S> &s2 )
{
    unordered_set<S> result;
    
    for( const S &s : s1 )
        if( s2.count(s)==0 )
            result.insert(s);
       
    return result; 
}    


// Intersect map with complement.
template< typename K, typename V, typename C >
inline map<K, V, C> DifferenceOf( const map<K, V, C> &m1, const map<K, V, C> &m2 )
{
    typename std::map<K, V, C>::key_compare kc1( m1.key_comp() );
    typename std::map<K, V, C> result( kc1 );
    auto kc2 = [&](const std::pair<K, V> &p1, const std::pair<K, V> &p2)
    { 
        return kc1(p1.first, p2.first); 
    };
    set_difference( m1.begin(), m1.end(), m2.begin(), m2.end(),
                    std::inserter(result, result.begin()),
                    kc2 );    
    return result; 
}    


// Intersect set with complement. s2 must be subset of s1.
template< typename S1, typename S2 >
inline S1 DifferenceOfSolo( const S1 &s1, const S2 &s2 )
{
    S1 result( s1.key_comp() );
    set_difference( s1.begin(), s1.end(), s2.begin(), s2.end(),
                    std::inserter(result, result.begin()),
                    s1.key_comp() );    
    ASSERT( result.size() == s1.size() - s2.size() )("Solo rule");
    return result; 
}    


// Intersect map with complement. s2 must be subset of s1.
template< typename K, typename V, typename C >
inline map<K, V, C> DifferenceOfSolo( const map<K, V, C> &m1, const map<K, V, C> &m2 )
{
    typename std::map<K, V, C>::key_compare kc1( m1.key_comp() );
    typename std::map<K, V, C> result( kc1 );
    auto kc2 = [&](const std::pair<K, V> &p1, const std::pair<K, V> &p2)
    { 
        return kc1(p1.first, p2.first); 
    };
    set_difference( m1.begin(), m1.end(), m2.begin(), m2.end(),
                    std::inserter(result, result.begin()),
                    kc2 );    
    ASSERT( result.size() == m1.size() - m2.size() )("Solo rule");
    return result; 
}    


// Difference between sets.
template< typename S1, typename S2 >
inline S1 SymmetricDifferenceOf( const S1 &s1, const S2 &s2 )
{
    S1 result( s1.key_comp() );
    set_symmetric_difference( s1.begin(), s1.end(), s2.begin(), s2.end(),
                              std::inserter(result, result.begin()),
                              s1.key_comp() );    
    return result; 
}    


// Difference between maps
template< typename K, typename V, typename C >
inline map<K, V, C> SymmetricDifferenceOf( const map<K, V, C> &m1, const map<K, V, C> &m2 )
{
    typename std::map<K, V, C>::key_compare kc1( m1.key_comp() );
    typename std::map<K, V, C> result( kc1 );
    auto kc2 = [&](const std::pair<K, V> &p1, const std::pair<K, V> &p2)
    { 
        return kc1(p1.first, p2.first); 
    };
    set_symmetric_difference( m1.begin(), m1.end(), m2.begin(), m2.end(),
                              std::inserter(result, result.begin()),
                              kc2 );    
    return result; 
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


// Is c2 a subset of c1?
template< typename C1, typename C2 >
inline bool IsIncludes( const C1 &c1, const C2 &c2 )
{
    return includes( c1.begin(), c1.end(), c2.begin(), c2.end(), c1.key_comp() );
}


// Is c1 equivalent to c2? NOT THE SAME AS c1==c2!!!
//  - IsEquivalent() uses the relation set for c1
//  - == requires all the elements to compare equal with ==
template< typename C1, typename C2 >
inline bool IsEquivalent( const C1 &c1, const C2 &c2 )
{
    // Prefer includes twice over set_symmetric_difference once
    // because includes only returns a bool and isn't building things.
    return includes( c1.begin(), c1.end(), c2.begin(), c2.end(), c1.key_comp() ) && 
           includes( c2.begin(), c2.end(), c1.begin(), c1.end(), c1.key_comp() );
}


// Get the keys of a map into a set
template<typename KEY, typename VALUE, typename KEY_COMP>
set<KEY, KEY_COMP> KeysToSet( const map<KEY, VALUE, KEY_COMP> &m )
{
    set<KEY, KEY_COMP> s( m.key_comp() );
    for( auto p : m )
        s.insert(p.first);
    return s;
}

// Can call on set, does nothing
template<typename KEY, typename KEY_COMP>
set<KEY, KEY_COMP> KeysToSet( const set<KEY, KEY_COMP> &s )
{
    return s;
}



// Is c1 a submap of c2?
template< typename K, typename V, typename C >
inline bool IsIncludes( const map<K, V, C> &m1, const map<K, V, C> &m2 )
{
    typename std::map<K, V, C>::key_compare kc1( m1.key_comp() );
    auto kc2 = [&](const std::pair<K, V> &p1, const std::pair<K, V> &p2)
    { 
        return kc1(p1.first, p2.first); 
    };
    return includes( m2.begin(), m2.end(), m1.begin(), m1.end(), kc2 );
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
pair<typename T::value_type, typename T::value_type> ToPair( const T&c )
{
    ASSERT( c.size() == 2 );
    typename T::const_iterator it = c.begin();
    typename T::const_iterator it_first = it;
    it++;
    typename T::const_iterator it_second = it;
    return make_pair( *it_first, *it_second );
}


template<typename T>
const typename T::value_type &SoloElementOf( const T &c )
{
    ASSERT( c.size()==1 )("Size is %d:\n", c.size())(c)("\n");
    return *(c.begin());
}


template<typename T>
typename T::value_type &SoloElementOf( T &c )
{
    ASSERT( c.size()==1 )("Size is %d:\n", c.size())(c)("\n");
    return *(c.begin());
}


template<typename E>
const typename set<E>::value_type &SoloElementOf( set<E> &c )
{
    ASSERT( c.size()==1 )("Size is %d:\n", c.size())(c)("\n");
    return *(c.begin());
}


template<typename T>
typename T::value_type &&SoloElementOf( T&&c )
{
    ASSERT( c.size()==1 );
    return move(*(c.begin()));
}


// Provide something like front() that works on sets and maps 
// as well as lists and vectors
template<typename T>
const typename T::value_type &FrontOf( const T&c )
{
    ASSERT( !c.empty() );
    return *(c.begin());
}


// Provide something like back() that works on sets and maps
template<typename T>
const typename T::value_type &BackOf( const T&c )
{
    ASSERT( !c.empty() );
    typename T::const_iterator it = c.end();
    --it;
    return *it;
}

// ----------------------- make_unique<>() -----------------------
// Built in at C++14 onward
#if __cplusplus < 201402L
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
#endif

// Get a "that" pointer, which is a pointer of the same type as "this"
// for multi-methods. NULL not allowed, wrong type not allowed.
#define GET_THAT_POINTER(BT) ({ \
    auto bt( BT ); \
    ASSERT( bt ); \
    auto tp = dynamic_cast<decltype(this)>(bt); \
    ASSERT( tp ); \
    tp; \
})    

// Note: GET_THAT_REFERENCE has problems I think with the statement
// expression ({ ... }) forcing by-value result.
#define GET_THAT_REFERENCE(BT) (*GET_THAT_POINTER(&(BT)))


// To use with set, map and algorithms like min_element when the
// container/range actually contains pointers (or shared or unique pointers)
// to the "real objects" which possess the operator<() that we require.
// You will probably need to explicitly instantiate the template, and this is
// done by providing the pointer type (not the pointed-to type).
template<typename POINTER_TYPE>
bool DereferencingIsLess( const POINTER_TYPE &a, 
                          const POINTER_TYPE &b )
{
    return *a < *b;
}                 


template<typename POINTER_TYPE>
struct DereferencingEqual
{
	bool operator()( const POINTER_TYPE &a, 
                     const POINTER_TYPE &b ) const noexcept
	{
		return *a == *b;
	}
};


template<typename POINTER_TYPE, typename POINTEE_TYPE>
struct DereferencingHash
{
	size_t operator()( const POINTER_TYPE &a ) const noexcept
	{
		return hash<POINTEE_TYPE>()(*a);
	}
};


struct PairHash {
	template <class T1, class T2>
	size_t operator () (const pair<T1,T2> &p) const {
		auto h1 = hash<T1>{}(p.first);
		auto h2 = hash<T2>{}(p.second);
		return h1 ^ h2;  
	}
};


// 2 differences from std::lexicographical_compare():
// - You don't need all those .begin() .end()
// - You get a full comparison result: -ve for less, 0 for equal, +ve for greater
template<typename C0, typename C1, class COMPARE>
int LexicographicalCompare( const C0 &c0, const C1 &c1, const COMPARE &compare )
{
    bool less_01 = lexicographical_compare( c0.begin(), c0.end(), 
                                            c1.begin(), c1.end(),
                                            compare );
    bool less_10 = lexicographical_compare( c1.begin(), c1.end(), 
                                            c0.begin(), c0.end(),
                                            compare );
    return (int)less_01 - (int)less_10;
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


// Over-write some values of a vector as dictated by a map. dest_vec must already be big enough.
template<typename VALUE_TYPE>
void ScatterInto( vector<VALUE_TYPE> &dest_vec, const map<int, VALUE_TYPE> &my_map )
{
    for( auto p : my_map )
        dest_vec.at(p.first) = p.second;
}


// dynamic_pointer_cast<>() extended to unique_ptr<>. This is based on an
// example from Stack Overflow but I want to support the same if-else-if chain
// style that I can with shared_ptr. 
// So: 
// - if src is NULL, so will the return be
// - if the cast fails, NULL is returned and src is still valid
// - if the cast succeeds, converted unique_ptr is returned and src is NULL
// In effect, the "move" takes place iff the conversion succeeds. We need
// to make src an lvalue for this to be possible, which rules out applying
// the dynamic cast to a true rvalue. 
// https://stackoverflow.com/questions/11002641/dynamic-casting-for-unique-ptr
template <typename T_DEST, typename T_SRC>
std::unique_ptr<T_DEST> dynamic_pointer_cast(std::unique_ptr<T_SRC>&& src)
{
    // When nullptr, just return nullptr
    if (!src) 
        return std::unique_ptr<T_DEST>(nullptr);

    // Perform dynamic_cast, throws std::bad_cast() if this doesn't work out
    T_DEST* dest_ptr = dynamic_cast<T_DEST*>(src.get());
    
    // Do return nullptr on bad_cast
    if (!dest_ptr) 
        return std::unique_ptr<T_DEST>(nullptr);

    // Move into new unique_ptr
    std::unique_ptr<T_DEST> dest_temp(dest_ptr);
    src.release();

    return dest_temp;
}


template<typename C>
string DiffTrace( const C &c0, const C &c1 )
{
    string s;
    s += SSPrintf("Size: %u -> %u elements; diff begins\n", c0.size(), c1.size());
    C ins = DifferenceOf( c1, c0 );
    for( auto e : ins )
        s += "+ " + Trace(e) + "\n";
    C rem = DifferenceOf( c0, c1 );
    for( auto e : rem )
        s += "- " + Trace(e) + "\n";
    s += "diff ends\n";
    return s;
}


// 3-way compare for pre C++20 STL containers like set, map etc that implement == and <
template <typename STL_TYPE>
Orderable::Diff STLCompare3Way( const STL_TYPE &l, const STL_TYPE &r )
{
    if( l == r )
        return 0;
    else if( l < r )
        return -1;
    else
        return 1;
}

// Unique ptr with full value semantics plus nullability. Clones on copy/assign.
// Makes you happy by permitting default copy constructor etc for containing class.
// Assuming polymorphic, VALUE_TYPE must have 
// virtual VALUE_TYPE *Clone() const;
template<typename VALUE_TYPE>
class ValuePtr : public unique_ptr<VALUE_TYPE>
{    
public:
    // Expose all of unique_ptr's constructors
    using unique_ptr<VALUE_TYPE>::unique_ptr;

    // Un-delete copy constructor and copy-assignment. Our versions will
    // call Clone() (the only way to prevent slicing) on non-NULL other.
    // template<typename OTHER_VALUE_TYPE>. Note: Clone() always produces the
    // final type, even though we may be templated on an intermediate.
    ValuePtr(const ValuePtr<VALUE_TYPE> &other) :
        unique_ptr<VALUE_TYPE>( other ? 
                                static_cast<VALUE_TYPE *>(other->Clone()) : 
                                nullptr )
    {        
    }
    
    //template<typename OTHER_VALUE_TYPE>
    ValuePtr &operator=(const ValuePtr<VALUE_TYPE> &other)
    {
        unique_ptr<VALUE_TYPE>::operator=( other ? 
                                           unique_ptr<VALUE_TYPE>(other->Clone()) : 
                                           nullptr );
        return *this;
    }    

    // Create from unique_ptr by rvalue/move (respecting unique_ptr semantics)
    ValuePtr(unique_ptr<VALUE_TYPE> &&uptr) :
        unique_ptr<VALUE_TYPE>( move(uptr) )
    {        
    }

    // Convert to unique ptr by cloning (our semantics)
    operator unique_ptr<VALUE_TYPE>() 
    {
        return *this ? 
               unique_ptr<VALUE_TYPE>((*this)->Clone()) : 
               nullptr;
    }

    // Instead of make_unique<X>() do ValuePtr<X>::Make()
    template<typename ... CP>
    static ValuePtr<VALUE_TYPE> Make(const CP &...cp) 
    {
        return ValuePtr<VALUE_TYPE>( new VALUE_TYPE(cp...) );
    }
    
    // Dynamic cast: has my preferred semantics (move if and only if cast successful)
    template<typename OTHER_VALUE_TYPE>
    static ValuePtr<VALUE_TYPE> DynamicCast(ValuePtr<OTHER_VALUE_TYPE> &&other) 
    {
        // Using our extension of dynamic_pointer_cast to unique_ptr
        return ValuePtr<VALUE_TYPE>( dynamic_pointer_cast<VALUE_TYPE>(move(other)) );
    }
    
    // TODO could add a dyn-cast on const ref, eg try the raw pointer dyncast and if
    // successful, Clone() for a return value.
};

// The hashing algo for pointers MAY just be a simple cast. So, shift out the alignment
// bits from the address for better hashing.
#define HASHING_POINTERS_ALIGNMENT_BITS 3 

#endif
