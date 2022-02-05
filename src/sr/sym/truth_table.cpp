#include "common/common.hpp"
#include "common/orderable.hpp"

#include "truth_table.hpp"

using namespace SYM;

// ------------------------- TruthTable --------------------------

TruthTable::TruthTable( int degree_, bool initval ) :
    degree( degree_ )
{
    int ncells = 1 << degree;

    // Table creation is reductive; we rule out possibilities
    cells.resize(ncells, initval);
}


bool TruthTable::Get( vector<bool> full_indices )
{
    return cells.at( GetCellIndex(full_indices) );
}


void TruthTable::Set( vector<bool> full_indices, bool value )
{
    cells.at( GetCellIndex(full_indices) ) = value;
}


void TruthTable::Set( map<int, bool> fixed_map, bool value )
{
    ASSERT( fixed_map.size() <= degree );
    
    // Determine what the free axes must be
    vector<int> free_axes;
    for( int i=0; i<degree; i++ )
        if( fixed_map.count(i) == 0 )
            free_axes.push_back(i);
    ASSERT( free_axes.size() + fixed_map.size() == degree );

    // Capture the fixed axes and indices into our vector
    vector<bool> full_indices(degree);
    ScatterInto( full_indices, fixed_map );

    // For all values of (bool)^(free axis count)
    ForPower( free_axes.size(), index_range_bool, (function<void(vector<bool>)>)[&](vector<bool> free_indices)
    {
        // Capture the free axes and indices into our vector
        map<int, bool> free_map = MakeMap( free_axes, free_indices );
        ScatterInto( full_indices, free_map );

        // Apply the change
        Set( full_indices, value );
    } );
}


TruthTable TruthTable::GetFold( set<int> fold_axes, bool identity ) const
{
    ASSERT( fold_axes.size() <= degree );

    // Determine what the destination axes must be
    vector<int> dest_axes;
    for( int i=0; i<degree; i++ )
        if( fold_axes.count(i) == 0 )
            dest_axes.push_back(i);
    ASSERT( dest_axes.size() + fold_axes.size() == degree );
    TruthTable dest( dest_axes.size() );

    vector<bool> full_indices(degree);

    // For all values of (bool)^(dest axis count)
    ForPower( dest_axes.size(), index_range_bool, (function<void(vector<bool>)>)[&](vector<bool> dest_indices)
    {
        // Capture the dest axes and indices into our vector
        map<int, bool> dest_map = MakeMap( dest_axes, dest_indices );
        ScatterInto( full_indices, dest_map );

        bool cell_total = identity;

        // For all values of (bool)^(fold axis count)
        ForPower( fold_axes.size(), index_range_bool, (function<void(vector<bool>)>)[&](vector<bool> fold_indices)
        {
            // Capture the free axes and indices into our vector
            map<int, bool> fold_map = MakeMap( fold_axes, fold_indices );
            ScatterInto( full_indices, fold_map );

            bool cell = cells.at( GetCellIndex(full_indices) );
            if( identity )
                cell_total = cell_total || cell;
            else
                cell_total = cell_total && cell;            
        } );

        dest.Set( dest_indices, cell_total );
    } );

    return dest;
}


int TruthTable::GetCellIndex( vector<bool> full_indices ) const
{
    ASSERT( full_indices.size() == degree );
    int cindex = 0;
    for( int j=0; j<full_indices.size(); j++ )
        cindex |= (full_indices.at(j)?1:0) << j;
    return cindex;
}


