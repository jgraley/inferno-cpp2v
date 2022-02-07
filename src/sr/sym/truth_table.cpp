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


void TruthTable::Extend( int new_degree )
{
    int new_ncells = 1 << new_degree;
    cells.resize(new_ncells);

    for( int axis=degree; axis<new_degree; axis++ )
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

    // For all values of (bool)^(dest axis count)
    ForPower( dest_axes.size(), index_range_bool, (function<void(vector<bool>)>)[&](vector<bool> dest_indices)
    {
        // Capture the dest axes and indices into our vector
        map<int, bool> dest_map = ZipToMap( dest_axes, dest_indices );
        vector<bool> full_indices(degree);
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
                cell_total = cell_total && cell;
            else
                cell_total = cell_total || cell;            
        } );

        dest.Set( dest_indices, cell_total );
    } );

    return dest;
}


set<vector<bool>> TruthTable::GetIndicesOfValue( bool value ) const
{
    set<vector<bool>> indices_set;
    
    ForPower( degree, index_range_bool, (function<void(vector<bool>)>)[&](vector<bool> indices)
    {
        if( Get(indices) == value )
            indices_set.insert( indices );
    } );
    return indices_set;
}


bool TruthTable::operator<( const TruthTable &other ) const
{
    // This gets us a lexigographical compare of the underlying data
    return cells < other.cells;
}


string Render( set<int> column_axes ) const
{
    ASSERT( column_axes.size() <= degree );
    
    int str_cell_size = 3;
    string label_fmt = SSPrintf("P\%0%dd", str_cell_size-2); // remember the ¬
    
    // Determine what the row axes must be
    vector<int> row_axes;
    for( int i=0; i<degree; i++ )
        if( column_axes.count(i) == 0 )
            row_axes.push_back(i);
    ASSERT( row_axes.size() + column_axes.size() == degree );

    vector<vector<string>> render_table_cells;

    // For all values of (bool)^(dest axis count)
    ForPower( row_axes.size(), index_range_bool, (function<void(vector<bool>)>)[&](vector<bool> row_indices)
    {
        // Capture the dest axes and indices into our vector
        map<int, bool> row_map = ZipToMap( row_axes, row_indices );
        vector<bool> full_indices(degree);
        ScatterInto( full_indices, row_map );
        
        vector<string> render_row_cells;

        // For all values of (bool)^(fold axis count)
        ForPower( column_axes.size(), index_range_bool, (function<void(vector<bool>)>)[&](vector<bool> column_indices)
        {
            // Capture the free axes and indices into our vector
            map<int, bool> column_map = ZipToMap( column_axes, column_indices );
            ScatterInto( full_indices, column_map );

            bool cell = cells.at( GetCellIndex(full_indices) );
            string str_cell = " "; // Align with the "P" in labels
            if( cell )
                str_cell += "✔"; // Warning: .size() won't be what it looks like!!!!!
            else
                str_cell += "✘"; // And here!!!
            str_cell += string(str_cell_size-2);
            render_row_cells.push_back( str_cell );
        } );
        
        render_table_cells.push_back(row);
    } );
    
    // Fill in 2-d blocks of items for the labels (in general, each 
    // of the two rendered axes row, col may have many truth table axes
    // and so multiple layers of labels may be needed
    auto labels_lambda = [&](vector<int> layers)
    {
        vector<vector<string>> render_my_labels;
        for( int layer=0; layer<layers.size(); layer++ )
        {
            vector<string> render_layer_labels;
            int len = 1 << layers.size(); // The size must be 2^num layers to get all combos in
            for( i=0; i<len; i++ )
            {
                string str_label;
                if( (i & (1<<layer))==0 ) 
                    str_label += "¬";
                else
                    str_label += " ";                
                str_label += SSPrintf(label_fmt.c_str(), layers[layer]);
                render_layer_labels.push_back(str_label);
            }
            render_my_labels.push_back(render_layer_labels);
        }
        return render_my_labels;
    };    
    render_row_labels = labels_lambda( row_axes );
    render_column_labels = labels_lambda( ToVector(column_axes) );
    
    // Cross-check everything fits together and then determine size of render
    ASSERT( render_table_cells.size() == render_row_labels[0].size() );
    ASSERT( render_table_cells[0].size() == render_column_labels[0].size() );
    int num_rows = render_column_labels.size() + render_row_labels[0].size();
    int num_columns = render_row_labels.size() + render_column_labels[0].size();
    
    // Join the labels to the table padding dead space with empty strings
    vector<vector<string>> render_table;
    for( int row=0; row<render_column_labels.size(); row++ )
    {
        vector<string> render_row;
        for( int col=0; col<render_row_labels.size(); col++ )
            render_row.push_back("");
        for( int col=0; col<render_column_labels[0].size(); col++ )
            render_row.push_back(render_column_labels[row][col]);      
        render_table.push_back(render_row);
    }
    for( int row=0; row<render_row_labels[0].size(); row++ )
    {
        vector<string> render_row;
        for( int col=0; col<render_row_labels.size(); col++ )
            render_row.push_back(render_row_labels[col][row]);
        for( int col=0; col<render_column_labels[0].size(); col++ )
            render_row.push_back(render_table_cells[row][col]);            
        render_table.push_back(render_row);
    }
    
    // Combine into a single string with spacing and newlines
    string str_table;
    for( int row=0; row<render_table.size(); row++ )
    {
        for( int col=0; col<render_row_labels.size(); col++ )    
            str_table += item + " ";    
        str_table += "\n";
    }
    
    return s;
}



int TruthTable::GetCellIndex( vector<bool> full_indices ) const
{
    ASSERT( full_indices.size() == degree );
    int cindex = 0;
    for( int j=0; j<full_indices.size(); j++ )
        cindex |= (full_indices.at(j)?1:0) << j;
    return cindex;
}

// ------------------------- unit tests --------------------------

void SYM::TestTruthTable()
{
    // 4-D truth table incorporates literals found in CSP equation
    TruthTable t( 4, true );

    // Filled in by analysis but could be repeated evaluation
    t.SetSlice( {{0, false}, {2, false}}, false ); // (1) np1 => p3 aka p1 || p3
    t.SetSlice( {{1, false}, {2, false}}, false ); // (3) np2 => p3 aka p2 || p3
    t.SetSlice( {{1, false}, {3, false}}, false ); // (4) np2 => p4 aka p2 || p4
    t.SetSlice( {{3, false}, {0, false}}, false ); // (7) np4 => p1 aka p4 || p1

    // Extend to include extrapolated literal
    t.Extend(5);

    // Filled in by analysis involving extrapolated literal
    t.SetSlice( {{0, true}, {2, true}, {4, false}}, false ); // (T1) Transitive Equal p1 p3 p5
    t.SetSlice( {{0, true}, {2, false}, {4, true}}, false ); // (T1) Transitive Equal p1 p3 p5
    t.SetSlice( {{0, false}, {2, true}, {4, true}}, false ); // (T1) Transitive Equal p1 p3 p5

    // Fold out extrapolated literal with OR-rule
    TruthTable t2 = t.GetFolded({4}, false); 

    // Extract remaining possibles using OR-rule
    set<vector<bool>> iv = t2.GetIndicesOfValue( false );

    ASSERT( iv == set<vector<bool>>({{true, true, true, true},
                                     {false, false, true, true},
                                     {true, true, false, false} }) )(iv);
}


