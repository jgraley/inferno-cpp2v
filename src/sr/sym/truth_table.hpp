#ifndef TRUTH_TABLE_HPP
#define TRUTH_TABLE_HPP

#include "common/common.hpp"
#include "common/orderable.hpp"

#include <vector>
#include <map>
#include <set>
#include <functional>

using namespace std;

namespace SYM
{

// Convert a pair of equal-length vectors to a map. Keys must be unique.
template<typename KEYS_TYPE, typename VALUES_TYPE>
map<typename KEYS_TYPE::value_type, typename VALUES_TYPE::value_type> MakeMap( const KEYS_TYPE &keys, const VALUES_TYPE &values )
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


// Overwrite some values of a vector as dictated by a map. Key has to be size_type
// since that's how you index a vector. dest_vec must already be big enough.
template<typename VALUE_TYPE>
void ScatterInto( vector<VALUE_TYPE> &dest_vec, const map<int, VALUE_TYPE> &my_map )
{
    for( auto p : my_map )
        dest_vec.at(p.first) = p.second;
}


// Lexicographical increment over a vector of INDEX_TYPE. INDEX_TYPE has to
// be incrementable, comparable, have a zero and be able to overflow by 1   
bool IncrementIndices( vector<int> &indices, int index_range )
{
    for( typename vector<int>::reverse_iterator it = indices.rbegin(); 
         it != indices.rend();
         ++it )
    {
        *it++;
        if( *it==index_range )
            *it = 0; // need to borrow so iterate again
        else
            return false; // OK - did not wrap
    }
    return true; // wrapped
}


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
    while(!IncrementIndices( int_indices, index_range.size() ) );
}


const vector<bool> index_range_bool = {false, true};
             
// ------------------------- TruthTable --------------------------

class TruthTable
{
public:
    explicit TruthTable( int degree, bool initval=false );

    bool Get( vector<bool> full_indices );
    void Set( vector<bool> full_indices, bool value );
    void Set( map<int, bool> fixed_map, bool value );

    // identity=true will OR; =false will AND
    TruthTable GetFold( set<int> fold_axes, bool identity ) const; 

private:
    int GetCellIndex( vector<bool> full_indices ) const;

    int degree;
    vector<bool> cells;
};

};

#endif
