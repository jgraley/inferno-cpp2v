#ifndef LAMBDA_LOOPS_HPP
#define LAMBDA_LOOPS_HPP

#include "standard.hpp"
#include "trace.hpp"
#include "common.hpp"

#include <string>
#include <functional>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

class BreakException : exception
{
};


// Break out of one of these loops
void Break();
// For "continue", just return out of the lambda

// Acting on a container such as [1, 2, 3, 4], will call func with (1, 2), 
// then (2, 3), then (3, 4). No iteratons if size() is 0 or 1.
// Note: using on set/map with arbitrary ordering can amplify reproducibility
// problems because the set of pairs actually changes membership, not just
// the order seen.
template<typename T>
void ForOverlappingAdjacentPairs( const T &container, 
                                  function<void(const typename T::value_type &first, 
                                                const typename T::value_type &second)> func) try
{
    const typename T::value_type *prev_x = nullptr;
	for( const auto &x : container )
    {
        if( prev_x )
            func( *prev_x, x );
 		prev_x = &x;
	}
}
catch( BreakException )
{
}

// Acting on a container such as [1, 2, 3, 4], will call func with 
// (2, 1), (3, 1), (4, 1), (3, 2), (4, 2), (4, 3), i.e. n(n-1)/2 iterations.
// No iterations if size() is 0 or 1.
template<typename T>
void ForAllCommutativeDistinctPairs( const T &container, 
                                     function<void(const typename T::value_type &first, 
                                                   const typename T::value_type &second)> func ) try
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
catch( BreakException )
{
}

// Acting on a container such as [1, 2, 3], will call func with 
// (1, 2), (1, 3), (2, 1), (2, 3), (3, 1), (3, 2), i.e. n(n-1) iterations.
// No iterations if size() is 0 or 1.
template<typename T>
void ForAllDistinctPairs( const T &container, 
                          function<void(const typename T::value_type &first, 
                                        const typename T::value_type &second)> func ) try
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
catch( BreakException )
{
}


// Lexicographical increment over a vector of int.
bool IncrementIndices( vector<int> &indices, int index_range );


// Lambda powered loop loops over some space raised to the power of degree. index_range
// contains all the values in the space, and their actual type is templated for your convenience.
template<typename AXIS_TYPE>
void ForPower( int degree, vector<AXIS_TYPE> index_range, function<void(vector<AXIS_TYPE>)> body ) try
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
catch( BreakException )
{
}

// The value range of a bool, to be used with ForPower
extern const vector<bool> index_range_bool;

#endif
