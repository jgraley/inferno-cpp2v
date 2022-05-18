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
    /*enum CellType
    {
        FALSE,
        TRUE,
        DONT_CARE
    };*/
    typedef bool CellType;

    explicit TruthTable( int degree, CellType initval );
    TruthTable( const TruthTable &other );
    TruthTable &operator=( const TruthTable &other );

    // Set a single cell value given a full vector of indices    
    void Set( vector<bool> full_indices, CellType new_value );

    // Set all cells satisfying the supplied indices. 
    void SetSlice( map<int, bool> fixed_map, CellType new_value );

    // Set all cells satisfying the supplied indices. new_values
    // must have all remaining axes.   
    void SetSlice( map<int, bool> fixed_map, const TruthTable &new_values );

    // Increase the degree by additional_degree, replicating
    // existing cell values. In-place.
    void Extend( int new_degree );

    int GetDegree() const;

    // Get a single cell value given a full fector of indices
    CellType Get( vector<bool> full_indices ) const;

    // Get a slice of a truth table in which axes indicated
    // by fixed_map's keys have been reduced away as specified by 
    // fixed_map's values.
    TruthTable GetSlice( map<int, bool> fixed_map ) const; 

    // Get a folded down truth table in which the fold_axes have
    // been removed and cells are assigned the maximum value of
    // the values across the fold_axes axes, i.e. higher takes priority.
    TruthTable GetFolded( set<int> fold_axes ) const; 

    // Find values matching given value and return their indices.
    set<vector<bool>> GetIndicesOfValue( CellType value ) const;

    // Do all the cells that correspond to the given fixed axes
    // have the given target value.
    int CountInSlice( map<int, bool> fixed_map, bool target_value ) const; 

    // Find the biggest slice wherein every element matches the
    // given target value. If there are no such slices, nullptr is returned.
    shared_ptr<map<int, bool>> TryFindBestKarnaughSlice( bool target_value, bool preferred_index, const TruthTable &so_far ) const;

    // Ordering
    bool operator==( const TruthTable &other ) const;
    bool operator<( const TruthTable &other ) const;
     
    // Draw a nice picture. column_axes along the top, 
    // others down the side
    string Render( set<int> column_axes, vector<string> pred_labels, int render_cell_size ) const; 
     
private:
    typedef vector<bool>::size_type SizeType;
    SizeType GetCellIndex( vector<bool> full_indices ) const;

    int degree;
    vector<CellType> cells;
};

// ------------------------- unit tests --------------------------

void TestTruthTable();

};

#endif
