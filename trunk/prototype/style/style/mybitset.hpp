
#ifndef MYBITSET_HPP
#define MYBITSET_HPP

#include <bitset>

// It's sometimes easier to think in terms of supersets and subsets than
// in terms of unions/intersections/complements. So here, we build subset
// inequalities on top of the other set operations.
//
// Since sets are not fully ordered the usual correspondances between 
// inequalities do not apply. You can have sets A and B such that
// (A>=B) is false and (A<B) is false etc. 
//
// Note that a strict subset must be smaller ie when A is a strict subset
// of B there must exist x such that x is in B but not in A. With non-strict
// subsets, the sets can be the same.

// Returns true when a is non-strict subset of b
template<int LIMIT>
inline bool operator<=( std::bitset<LIMIT> a, std::bitset<LIMIT> b )
{
    // For a subset (non-strict) any elements in a must also be in b.
    // So there must be no x where x is in a but not in b.
    // Such x would be in a and the complement of b.
    // Ie they would be in the intersection of a and the complement of b. 
    return (a & ~b).none();
}

// Returns true when a is strict subset of b
template<int LIMIT>
inline bool operator<( std::bitset<LIMIT> a, std::bitset<LIMIT> b )
{
    return (a <= b) && (a != b);
}

// Returns true when b is non-strict subset of a
template<int LIMIT>
inline bool operator>=( std::bitset<LIMIT> a, std::bitset<LIMIT> b )
{
    return b <= a;
}

// Returns true when b is strict subset of a
template<int LIMIT>
inline bool operator>( std::bitset<LIMIT> a, std::bitset<LIMIT> b )
{
    return b < a;
}

#endif // include guard
