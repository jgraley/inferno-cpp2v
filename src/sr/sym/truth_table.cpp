#include "common/common.hpp"
#include "common/orderable.hpp"

#include "truth_table.hpp"

using namespace SYM;

// ------------------------- TruthTable --------------------------

TruthTable::TruthTable( int degree_, bool initval ) :
    degree( degree_ )
{
    int ncells = 1 << degree;
    cells.resize(ncells, initval);
}


void TruthTable::Set( vector<bool> full_indices, bool new_value )
{
    cells.at( GetCellIndex(full_indices) ) = new_value;
}


void TruthTable::SetSlice( map<int, bool> fixed_map, bool new_value )
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
        map<int, bool> free_map = ZipToMap( free_axes, free_indices );
        ScatterInto( full_indices, free_map );

        // Apply the change
        Set( full_indices, new_value );
    } );
}


void TruthTable::SetSlice( map<int, bool> fixed_map, const TruthTable &new_values )
{
    ASSERT( fixed_map.size() + new_values.degree == degree );
    
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
        map<int, bool> free_map = ZipToMap( free_axes, free_indices );
        ScatterInto( full_indices, free_map );

        // Apply the change
        Set( full_indices, new_values.Get( free_indices ) );
    } );
}


void TruthTable::Extend( int additional_degree )
{
    int new_degree = degree + additional_degree;
    int new_ncells = 1 << new_degree;
    cells.resize(new_ncells);

    for( int i=0; i<additional_degree; i++ )
    {
        int ncells = 1 << degree;
        copy( cells.begin(),
              cells.begin()+ncells,
              cells.begin()+ncells );
        degree++;                  
    }
}


bool TruthTable::Get( vector<bool> full_indices ) const
{
    return cells.at( GetCellIndex(full_indices) );
}


TruthTable TruthTable::GetSlice( map<int, bool> fixed_map ) const
{
    ASSERT( fixed_map.size() <= degree );
    
    // Determine what the free axes must be
    vector<int> dest_axes;
    for( int i=0; i<degree; i++ )
        if( fixed_map.count(i) == 0 )
            dest_axes.push_back(i);
    ASSERT( dest_axes.size() + fixed_map.size() == degree );
    TruthTable dest( dest_axes.size() );

    // Capture the fixed axes and indices into our vector
    vector<bool> full_indices(degree);
    ScatterInto( full_indices, fixed_map );

    // For all values of (bool)^(free axis count)
    ForPower( dest_axes.size(), index_range_bool, (function<void(vector<bool>)>)[&](vector<bool> dest_indices)
    {
        // Capture the free axes and indices into our vector
        map<int, bool> dest_map = ZipToMap( dest_axes, dest_indices );
        ScatterInto( full_indices, dest_map );

        // Apply the change
        dest.Set( dest_indices, Get( full_indices ) );
    } );

    return dest;
}


TruthTable TruthTable::GetFolded( set<int> fold_axes, bool identity ) const
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
        map<int, bool> dest_map = ZipToMap( dest_axes, dest_indices );
        ScatterInto( full_indices, dest_map );

        bool cell_total = identity;

        // For all values of (bool)^(fold axis count)
        ForPower( fold_axes.size(), index_range_bool, (function<void(vector<bool>)>)[&](vector<bool> fold_indices)
        {
            // Capture the free axes and indices into our vector
            map<int, bool> fold_map = ZipToMap( fold_axes, fold_indices );
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


bool TruthTable::operator<( const TruthTable &other ) const
{
    // This gets us a lexigographical compare of the underlying data
    return cells < other.cells;
}


int TruthTable::GetCellIndex( vector<bool> full_indices ) const
{
    ASSERT( full_indices.size() == degree );
    int cindex = 0;
    for( int j=0; j<full_indices.size(); j++ )
        cindex |= (full_indices.at(j)?1:0) << j;
    return cindex;
}



