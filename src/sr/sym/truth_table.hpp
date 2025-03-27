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
    enum class CellType
    {
        // Folding prioritises higher values
        DONT_CARE = -1,
        FALSE = 0,
        TRUE = 1
    };
    
    // Specifies a slice by giving a value to every fixed index;
    // free ones are the remaining ones.
    typedef map<int, bool> SliceSpec;

    explicit TruthTable( unsigned degree, CellType initval );
    TruthTable( const TruthTable &other );
    TruthTable &operator=( const TruthTable &other );

    // Set a single cell value given a full vector of indices    
    void Set( vector<bool> full_indices, CellType new_value );

    // Set all cells satisfying the supplied indices. 
    void SetSlice( SliceSpec slice, CellType new_value );

    // Set all cells satisfying the supplied indices. new_values
    // must have all remaining axes.   
    void SetSlice( SliceSpec slice, const TruthTable &new_values );

    // Increase the degree by additional_degree, replicating
    // existing cell values. In-place.
    void Extend( unsigned new_degree );

    unsigned GetDegree() const;

    // Get a single cell value given a full fector of indices
    CellType Get( vector<bool> full_indices ) const;

    // Get a slice of a truth table in which axes indicated
    // by fixed_map's keys have been reduced away as specified by 
    // fixed_map's values.
    TruthTable GetSlice( SliceSpec slice ) const; 

    // Get a folded down truth table in which the fold_axes have
    // been removed and cells are assigned the maximum value of
    // the values across the fold_axes axes, i.e. higher takes priority.
    TruthTable GetFolded( set<int> fold_axes ) const; 

    // Find values matching given value and return their indices.
    set<vector<bool>> GetIndicesOfValue( CellType value ) const;

    // Do all the cells that correspond to the given fixed axes
    // have the given target value.
    int CountInSlice( SliceSpec slice, CellType target_value ) const; 

    // Find the biggest slice wherein every element matches the
    // given target value. If there are no such slices, nullptr is returned.
    shared_ptr<SliceSpec> TryFindBestKarnaughSlice( CellType target_value, bool preferred_index, const TruthTable &so_far ) const;

    // Orderered
    bool operator==( const TruthTable &other ) const;
    bool operator<( const TruthTable &other ) const;
     
    // Draw a nice picture. column_axes along the top, 
    // others down the side
    string Render( set<int> column_axes, vector<string> pred_labels, unsigned render_cell_size ) const; 
     
private:
    typedef vector<bool>::size_type SizeType;
    SizeType GetCellIndex( vector<bool> full_indices ) const;

    unsigned degree;
    vector<CellType> cells;
};

// ------------------------- unit tests --------------------------

void TestTruthTable();

};

#endif
