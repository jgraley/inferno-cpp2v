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
             
// ------------------------- TruthTable --------------------------

class TruthTable
{
public:
    explicit TruthTable( int degree, bool initval=false );

    // Set a single cell value given a full fector of indices    
    void Set( vector<bool> full_indices, bool new_value );

    // Set all cells satisfying the supplied indices. 
    void SetSlice( map<int, bool> fixed_map, bool new_value );

    // Set all cells satisfying the supplied indices. new_values
    // must have all remaining axes.   
    void SetSlice( map<int, bool> fixed_map, const TruthTable &new_values );

    // Increase the degree by additional_degree, replicating
    // existing cell values. In-place.
    void Extend( int new_degree );

    // Get a single cell value given a full fector of indices
    bool Get( vector<bool> full_indices ) const;

    // Get a slice of a truth table in which the fold_axes have
    TruthTable GetSlice( map<int, bool> fixed_map ) const; 

    // Get a folded down truth table in which the fold_axes have
    // been removed and the values combined together based on
    // identity=false will OR; =true will AND
    TruthTable GetFolded( set<int> fold_axes, bool identity ) const; 

    // Find values matching given value and return their indices
    set<vector<bool>> GetIndicesOfValue( bool value ) const;

    // Ordering
    bool operator==( const TruthTable &other ) const;
    bool operator<( const TruthTable &other ) const;
     
    // Draw a nice picture. column_axes along the top, 
    // others down the side
    string Render( set<int> column_axes, string label_var_name, int counting_based ) const; 
     
private:
    int GetCellIndex( vector<bool> full_indices ) const;

    int degree;
    vector<bool> cells;
};

// ------------------------- unit tests --------------------------

void TestTruthTable();

};

#endif
