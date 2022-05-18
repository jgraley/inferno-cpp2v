#include "truth_table.hpp"

#include "common/common.hpp"
#include "common/orderable.hpp"

#include <algorithm>

using namespace SYM;

// ------------------------- TruthTable --------------------------

TruthTable::TruthTable( int degree_, CellType initval ) :
    degree( degree_ )
{
    SizeType ncells = (SizeType)1 << degree;
    cells.resize(ncells, initval);
}


TruthTable::TruthTable( const TruthTable &other ) :
    degree( other.degree ),
    cells( other.cells )
{
}


TruthTable &TruthTable::operator=( const TruthTable &other )
{
    degree = other.degree;
    cells = other.cells;
    return *this;
}


void TruthTable::Set( vector<bool> full_indices, CellType new_value )
{
    cells.at( GetCellIndex(full_indices) ) = new_value;
    // TRACE(cells)("\n");
}


void TruthTable::SetSlice( map<int, bool> fixed_map, CellType new_value )
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
    ForPower<bool>( free_axes.size(), index_range_bool, [&](vector<bool> free_indices)
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
    ForPower<bool>( free_axes.size(), index_range_bool, [&](vector<bool> free_indices)
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
    SizeType new_ncells = (SizeType)1 << new_degree;
    cells.resize(new_ncells);

    for( int axis=degree; axis<new_degree; axis++ )
    {
        SizeType ncells = (SizeType)1 << axis;
        copy( cells.begin(),
              cells.begin()+ncells,
              cells.begin()+ncells );
    }
    
    degree = new_degree;
}


int TruthTable::GetDegree() const
{
    return degree;
}


TruthTable::CellType TruthTable::Get( vector<bool> full_indices ) const
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
    TruthTable dest( dest_axes.size(), CellType::FALSE );

    // Capture the fixed axes and indices into our vector
    vector<bool> full_indices(degree);
    ScatterInto( full_indices, fixed_map );

    // For all values of (bool)^(dest axis count)
    ForPower<bool>( dest_axes.size(), index_range_bool, [&](vector<bool> dest_indices)
    {
        // Capture the free axes and indices into our vector
        map<int, bool> dest_map = ZipToMap( dest_axes, dest_indices );
        ScatterInto( full_indices, dest_map );

        // Copy cell to destination
        dest.Set( dest_indices, Get( full_indices ) );
    } );

    return dest;
}


TruthTable TruthTable::GetFolded( set<int> fold_axes ) const
{
    ASSERT( fold_axes.size() <= degree );

    // Determine what the destination axes must be
    vector<int> dest_axes;
    for( int i=0; i<degree; i++ )
        if( fold_axes.count(i) == 0 )
            dest_axes.push_back(i);
    ASSERT( dest_axes.size() + fold_axes.size() == degree );
    TruthTable dest( dest_axes.size(), CellType::FALSE );

    // For all values of (bool)^(dest axis count)
    ForPower<bool>( dest_axes.size(), index_range_bool, [&](vector<bool> dest_indices)
    {
        // Capture the dest axes and indices into our vector
        map<int, bool> dest_map = ZipToMap( dest_axes, dest_indices );
        vector<bool> full_indices(degree);
        ScatterInto( full_indices, dest_map );

        set<CellType> cells_to_fold;

        // For all values of (bool)^(fold axis count)
        ForPower<bool>( fold_axes.size(), index_range_bool, [&](vector<bool> fold_indices)
        {
            // Capture the free axes and indices into our vector
            map<int, bool> fold_map = ZipToMap( fold_axes, fold_indices );
            ScatterInto( full_indices, fold_map );

            cells_to_fold.insert( cells.at( GetCellIndex(full_indices) ) );
        } );
        
        CellType folded_cell = *max_element(cells_to_fold.begin(), cells_to_fold.end());
        dest.Set( dest_indices, folded_cell );
    } );

    return dest;
}


set<vector<bool>> TruthTable::GetIndicesOfValue( CellType value ) const
{
    set<vector<bool>> indices_set;
    
    ForPower<bool>( degree, index_range_bool, [&](vector<bool> indices)
    {
        if( Get(indices) == value )
            indices_set.insert( indices );
    } );
    return indices_set;
}


int TruthTable::CountInSlice( map<int, bool> fixed_map, CellType target_value ) const
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
    int count = 0;
    ForPower<bool>( free_axes.size(), index_range_bool, [&](vector<bool> free_indices)
    {
        // Capture the free axes and indices into our vector
        map<int, bool> free_map = ZipToMap( free_axes, free_indices );
        ScatterInto( full_indices, free_map );

        // Apply the change
        if( Get(full_indices) == target_value )
            count++;
    } );

    return count;
}


shared_ptr<map<int, bool>> TruthTable::TryFindBestKarnaughSlice( CellType target_value, bool preferred_index, const TruthTable &so_far ) const
{
    enum class KarnaughClass
    {
        FREE,
        FALSE,
        TRUE
    };

    CellType avoid_value = (target_value==CellType::TRUE) ? CellType::FALSE : CellType::TRUE;
    KarnaughClass preferred_class = preferred_index ? KarnaughClass::TRUE : KarnaughClass::FALSE;

    // Put FREE first so that we try the biggest slices first. Next is the preferred index
    const vector<KarnaughClass> index_range_kc = {KarnaughClass::FREE, KarnaughClass::TRUE, KarnaughClass::FALSE};
    shared_ptr<map<int, bool>> best_slice;
    map<KarnaughClass, int> best_counts;
    best_counts[KarnaughClass::FREE] = -1;
    best_counts[KarnaughClass::FALSE] = -1;
    best_counts[KarnaughClass::TRUE] = -1;
    int best_new_count = -1;
    
    // We'll try 3^degree possibilities for Karnaugh slices
    ForPower<KarnaughClass>( degree, index_range_kc, [&](vector<KarnaughClass> k_classes )
    {     
        auto candidate_slice = make_shared<map<int, bool>>();
        map<KarnaughClass, int> candidate_counts;
        for( int i=0; i<degree; i++ )
        {
            // Make slice across the FREE axes, located by the other ones
            KarnaughClass c = k_classes.at(i);
            switch( c )
            {
            case KarnaughClass::FREE:
                break;
            case KarnaughClass::FALSE:
                (*candidate_slice)[i] = false;
                break;
            case KarnaughClass::TRUE:
                (*candidate_slice)[i] = true;
                break;            
            }
            candidate_counts[c]++;
        }

        int slice_size = 1<<(degree-candidate_slice->size());
        int candidate_new_count = so_far.CountInSlice( *candidate_slice, target_value );

        // NECCESSARY conditions
        if( CountInSlice( *candidate_slice, avoid_value ) == 0 && // Slice should not include "crosses" in original truth table
            candidate_new_count > 0 ) // Slice must improve upon so-far solution by bringing at least one target cell in
        {
            // DESIRABLE conditions
            if( candidate_new_count > best_new_count || // 1. biggest improvement on so-far solution by count of "new ticks covered"
                (candidate_new_count == best_new_count && candidate_counts[KarnaughClass::FREE] > best_counts[KarnaughClass::FREE]) || // higher dimension (fewer clauses)
                (candidate_new_count == best_new_count && candidate_counts[KarnaughClass::FREE] == best_counts[KarnaughClass::FREE] && candidate_counts[preferred_class] > best_counts[preferred_class]) ) // more ticks (fewer negations)
            {
                best_slice = candidate_slice;
                best_new_count = candidate_new_count;
                best_counts = candidate_counts;
            }
        } 
    } );
    
    return best_slice; 
}


bool TruthTable::operator==( const TruthTable &other ) const
{
    // Equality
    return cells == other.cells;
}


bool TruthTable::operator<( const TruthTable &other ) const
{
    // This gets us a lexigographical compare of the underlying data
    return cells < other.cells;
}


string TruthTable::Render( set<int> column_axes, vector<string> pred_labels, int render_cell_size ) const
{
    ASSERT( column_axes.size() <= degree );
    ASSERT( render_cell_size >= 2 );
    for( int i : column_axes )
        ASSERT( i>=0 && i<degree )("axis out of range\n")(column_axes)("\ndegree=%d\n", degree);
    
    // Determine what the row axes must be
    vector<int> row_axes;
    for( int i=0; i<degree; i++ )
        if( column_axes.count(i) == 0 )
            row_axes.push_back(i);
    ASSERT( row_axes.size() + column_axes.size() == degree )
          ("row_axes=\n")(row_axes)("\ncolumn_axes=\n")(column_axes)("\ndegree=%d\n", degree);

    vector<vector<string>> render_table_cells;

    // For all values of (bool)^(row axis count)
    ForPower<bool>( row_axes.size(), index_range_bool, [&](vector<bool> row_indices)
    {
        // Capture the dest axes and indices into our vector
        map<int, bool> row_map = ZipToMap( row_axes, row_indices );
        vector<bool> full_indices(degree);
        ScatterInto( full_indices, row_map );
        
        vector<string> render_row_cells;

        // For all values of (bool)^(column axis count)
        ForPower<bool>( column_axes.size(), index_range_bool, [&](vector<bool> column_indices)
        {
            // Capture the free axes and indices into our vector
            map<int, bool> column_map = ZipToMap( column_axes, column_indices );
            ScatterInto( full_indices, column_map );

            CellType cell = cells.at( GetCellIndex(full_indices) );
            string str_cell = " "; // Align with the "P" in labels
            switch( cell )
            {
            case CellType::TRUE:
                str_cell += "✔"; // Warning: .size() won't be what it looks like!!!!!
                break;
            case CellType::FALSE:
                str_cell += "✘"; // And here!!!
                break;
            case CellType::DONT_CARE:
                str_cell += "-"; 
                break;
            }
            str_cell += string(render_cell_size-2, ' ');
            render_row_cells.push_back( str_cell );
        } );
        
        render_table_cells.push_back(render_row_cells);
    } );
    
    // TRACE(render_table_cells)("\n");
    
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
            for( int i=0; i<len; i++ )
            {
                string str_label;
                if( (i & (1<<layer))==0 ) 
                    str_label += "¬";           
                str_label += pred_labels.at( layers.at(layer) );
                if( str_label.size() < render_cell_size )
                    str_label = string(render_cell_size-str_label.size(), ' ') + str_label;
                render_layer_labels.push_back(str_label);
            }
            render_my_labels.push_back(render_layer_labels);
        }
        return render_my_labels;
    };    
    vector<vector<string>> render_row_labels = labels_lambda( row_axes );
    vector<vector<string>> render_column_labels = labels_lambda( ToVector(column_axes) );
    
    // Cross-check everything fits together and then determine size of render
    int rows_size = 1 << row_axes.size();
    int columns_size = 1 << column_axes.size();
    ASSERT( rows_size >= 1 );
    ASSERT( columns_size >= 1 );
    ASSERT( render_table_cells.size() == rows_size );
    ASSERT( render_table_cells.at(0).size() == columns_size );
    int num_rows = render_column_labels.size() + rows_size;
    int num_columns = render_row_labels.size() + columns_size;
    
    // Join the labels to the table padding dead space with empty strings
    vector<vector<string>> render_figure;
    for( int row=0; row<render_column_labels.size(); row++ )
    {
        vector<string> render_row;
        for( int col=0; col<render_row_labels.size(); col++ )
            render_row.push_back( string(render_cell_size, ' ') );
        for( int col=0; col<columns_size; col++ )
            render_row.push_back(render_column_labels.at(row).at(col));      
        render_figure.push_back(render_row);
    }
    for( int row=0; row<rows_size; row++ )
    {
        vector<string> render_row;
        for( int col=0; col<render_row_labels.size(); col++ )
            render_row.push_back(render_row_labels.at(col).at(row));
        for( int col=0; col<columns_size; col++ )
            render_row.push_back(render_table_cells.at(row).at(col));            
        render_figure.push_back(render_row);
    }
    
    // Combine into a single string with spacing and newlines
    string str_figure;
    for( int row=0; row<render_figure.size(); row++ )
    {
        for( int col=0; col<render_figure.at(row).size(); col++ )    
            str_figure += render_figure.at(row).at(col) + " ";    
        str_figure += "\n";
    }
    
    return str_figure;
}



TruthTable::SizeType TruthTable::GetCellIndex( vector<bool> full_indices ) const
{
    ASSERT( full_indices.size() == degree );
    SizeType cindex = 0;
    for( int j=0; j<full_indices.size(); j++ )
        cindex |= (SizeType)(full_indices.at(j)?1:0) << j;
    return cindex;
}

// ------------------------- unit tests --------------------------

static void TestTruthTableBase()
{
    vector<string> pred_labels = {"p1", "p2", "p3", "p4"};
    int render_cell_size = 3;
    
    // First test checks for consistency between simple Set()
    // and the renders, and looks for various kinds of swaps.
    TruthTable t( 4, TruthTable::CellType::TRUE );
    t.Set( {true, true, false, false}, TruthTable::CellType::FALSE );
    string r = t.Render({0, 2}, pred_labels, render_cell_size);    
    TRACE("{true, true, false, false} to false\n")( "\n"+r );
    ASSERT( r=="        ¬p1  p1 ¬p1  p1 \n"
               "        ¬p3 ¬p3  p3  p3 \n"
               "¬p2 ¬p4  ✔   ✔   ✔   ✔  \n"
               " p2 ¬p4  ✔   ✘   ✔   ✔  \n"
               "¬p2  p4  ✔   ✔   ✔   ✔  \n"
               " p2  p4  ✔   ✔   ✔   ✔  \n" );
    t.Set( {true, true, false, true}, TruthTable::CellType::FALSE );
    r = t.Render({0, 2}, pred_labels, render_cell_size);
    TRACE("{true, true, false, true} to false\n")( "\n"+r );
    ASSERT( r=="        ¬p1  p1 ¬p1  p1 \n"
               "        ¬p3 ¬p3  p3  p3 \n"
               "¬p2 ¬p4  ✔   ✔   ✔   ✔  \n"
               " p2 ¬p4  ✔   ✘   ✔   ✔  \n"
               "¬p2  p4  ✔   ✔   ✔   ✔  \n"
               " p2  p4  ✔   ✘   ✔   ✔  \n" );
    t.Set( {true, true, true, false}, TruthTable::CellType::FALSE );
    r = t.Render({0, 2}, pred_labels, render_cell_size);
    TRACE("{true, true, true, false} to false\n")( "\n"+r );
    ASSERT( r=="        ¬p1  p1 ¬p1  p1 \n"
               "        ¬p3 ¬p3  p3  p3 \n"
               "¬p2 ¬p4  ✔   ✔   ✔   ✔  \n"
               " p2 ¬p4  ✔   ✘   ✔   ✘  \n"
               "¬p2  p4  ✔   ✔   ✔   ✔  \n"
               " p2  p4  ✔   ✘   ✔   ✔  \n" );
    t.Set( {false, false, false, false}, TruthTable::CellType::FALSE );
    r = t.Render({0, 2}, pred_labels, render_cell_size);
    TRACE("{false, false, false, false} to false\n")( "\n"+r );
    ASSERT( r=="        ¬p1  p1 ¬p1  p1 \n"
               "        ¬p3 ¬p3  p3  p3 \n"
               "¬p2 ¬p4  ✘   ✔   ✔   ✔  \n"
               " p2 ¬p4  ✔   ✘   ✔   ✘  \n"
               "¬p2  p4  ✔   ✔   ✔   ✔  \n"
               " p2  p4  ✔   ✘   ✔   ✔  \n" );
    t.Set( {false, true, false, true}, TruthTable::CellType::FALSE );
    r = t.Render({0, 2}, pred_labels, render_cell_size);
    TRACE("{false, true, false, true} to false\n")( "\n"+r );
    ASSERT( r=="        ¬p1  p1 ¬p1  p1 \n"
               "        ¬p3 ¬p3  p3  p3 \n"
               "¬p2 ¬p4  ✘   ✔   ✔   ✔  \n"
               " p2 ¬p4  ✔   ✘   ✔   ✘  \n"
               "¬p2  p4  ✔   ✔   ✔   ✔  \n"
               " p2  p4  ✘   ✘   ✔   ✔  \n" );
    // now undo the first one
    t.Set( {true, true, false, false}, TruthTable::CellType::TRUE );
    r = t.Render({0, 2}, pred_labels, render_cell_size);    
    TRACE("{true, true, false, false} to true\n")( "\n"+r );
    ASSERT( r=="        ¬p1  p1 ¬p1  p1 \n"
               "        ¬p3 ¬p3  p3  p3 \n"
               "¬p2 ¬p4  ✘   ✔   ✔   ✔  \n"
               " p2 ¬p4  ✔   ✔   ✔   ✘  \n"
               "¬p2  p4  ✔   ✔   ✔   ✔  \n"
               " p2  p4  ✘   ✘   ✔   ✔  \n" );
}


static void TestTruthTableKarnaugh()
{
    vector<string> pred_labels = {"p1", "p2", "p3", "p4"};
    int render_cell_size = 3;
    
    // Repeat TestTruthTableBase to get same truth table but
    // without so many render checks.
    TruthTable t( 4, TruthTable::CellType::TRUE );
    t.Set( {true, true, false, false}, TruthTable::CellType::FALSE );
    t.Set( {true, true, false, true}, TruthTable::CellType::FALSE );
    t.Set( {true, true, true, false}, TruthTable::CellType::FALSE );
    t.Set( {false, false, false, false}, TruthTable::CellType::FALSE );
    t.Set( {false, true, false, true}, TruthTable::CellType::FALSE );
    t.Set( {true, true, false, false}, TruthTable::CellType::TRUE );
    string r = t.Render({0, 2}, pred_labels, render_cell_size);    
    TRACE("{true, true, false, false} to true\n")( "\n"+r );
    ASSERT( r=="        ¬p1  p1 ¬p1  p1 \n"
               "        ¬p3 ¬p3  p3  p3 \n"
               "¬p2 ¬p4  ✘   ✔   ✔   ✔  \n"
               " p2 ¬p4  ✔   ✔   ✔   ✘  \n"
               "¬p2  p4  ✔   ✔   ✔   ✔  \n"
               " p2  p4  ✘   ✘   ✔   ✔  \n" );

    // Now look for some karnaugh slices - success cases
    ASSERT( t.CountInSlice( {{2, true}, {3, true}}, TruthTable::CellType::TRUE )==4 );
    ASSERT( t.CountInSlice( {{1, false}, {3, true}}, TruthTable::CellType::TRUE )==4 );
    ASSERT( t.CountInSlice( {{0, false}, {2, true}}, TruthTable::CellType::TRUE )==4 );
    ASSERT( t.CountInSlice( {{1, true}, {2, false}, {3, false}}, TruthTable::CellType::TRUE )==2 );
    ASSERT( t.CountInSlice( {{1, true}, {2, false}, {3, true}}, TruthTable::CellType::FALSE )==2 );
               
    // Look for some more karnaugh slices - unsucessful cases
    ASSERT( t.CountInSlice( {{2, true}, {3, true}}, TruthTable::CellType::FALSE )<4 );
    ASSERT( t.CountInSlice( {{1, true}, {2, false}, {3, true}}, TruthTable::CellType::TRUE )<2 );
    ASSERT( t.CountInSlice( {{1, false}, {2, false}, {3, false}}, TruthTable::CellType::TRUE )<2 );
    ASSERT( t.CountInSlice( {{1, false}, {3, false}}, TruthTable::CellType::TRUE )<4 );
    ASSERT( t.CountInSlice( {{2, true}, {3, false}}, TruthTable::CellType::TRUE )<4 );
    ASSERT( t.CountInSlice( {{3, false}}, TruthTable::CellType::TRUE )<8 );

    // Whole thing
    ASSERT( t.CountInSlice( {}, TruthTable::CellType::TRUE )==12 );
    ASSERT( t.CountInSlice( {}, TruthTable::CellType::FALSE )==4 );
    TruthTable t_all( 4, TruthTable::CellType::TRUE );
    ASSERT( t_all.CountInSlice( {}, TruthTable::CellType::TRUE )==16 );
    ASSERT( t_all.CountInSlice( {}, TruthTable::CellType::FALSE )==0 );
    TruthTable t_none( 4, TruthTable::CellType::FALSE );
    ASSERT( t_none.CountInSlice( {}, TruthTable::CellType::FALSE )==16 );
    ASSERT( t_none.CountInSlice( {}, TruthTable::CellType::TRUE )==0 );

    // Single cell
    ASSERT( t.CountInSlice( {{0, true}, {1, false}, {2, false}, {3, false}}, TruthTable::CellType::TRUE )==1 );
    ASSERT( t.CountInSlice( {{0, true}, {1, false}, {2, false}, {3, false}}, TruthTable::CellType::FALSE )==0 );
    ASSERT( t.CountInSlice( {{0, false}, {1, false}, {2, false}, {3, false}}, TruthTable::CellType::FALSE )==1 );
    ASSERT( t.CountInSlice( {{0, false}, {1, false}, {2, false}, {3, false}}, TruthTable::CellType::TRUE )==0 );

    // -------------------------------- DISJOINT KARNAUGH --------------------------------------

    // Test TryFindBestKarnaughSlice() by walking through a map generation process on our "t".
    shared_ptr<map<int, bool>> p;
    p = t.TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, t );
    ASSERT( p );
    TRACE(*p)("\n");
    ASSERT( *p == (map<int, bool>({{2, true}, {3, true}})) )(*p);
    t.SetSlice(*p, TruthTable::CellType::FALSE);
    
    p = t.TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, t  );
    ASSERT( p );
    TRACE(*p)("\n");
    ASSERT( p->size()==3 )(*p);
    t.SetSlice(*p, TruthTable::CellType::FALSE);
    
    p = t.TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, t  );
    ASSERT( p );
    TRACE(*p)("\n");
    ASSERT( p->size()==3 )(*p);
    t.SetSlice(*p, TruthTable::CellType::FALSE);
    
    p = t.TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, t  );
    ASSERT( p );
    TRACE(*p)("\n");
    ASSERT( p->size()==3 )(*p);
    t.SetSlice(*p, TruthTable::CellType::FALSE);
    
    p = t.TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, t  );
    ASSERT( p );
    TRACE(*p)("\n");
    ASSERT( p->size()==4 )(*p);
    t.SetSlice(*p, TruthTable::CellType::FALSE);
    
    p = t.TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, t  );
    ASSERT( p );
    TRACE(*p)("\n");
    ASSERT( p->size()==4 )(*p);
    t.SetSlice(*p, TruthTable::CellType::FALSE);
    
    p = t.TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, t  );
    ASSERT( !p );

    r = t.Render({0, 2}, pred_labels, render_cell_size);    
    TRACE( "\n"+r );    

    // all and none cases
    p = t_all.TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, t_all );
    ASSERT( p );
    TRACE(*p)("\n");
    ASSERT( p->size()==0 )(*p);
    
    p = t_all.TryFindBestKarnaughSlice( TruthTable::CellType::FALSE, true, t_all );
    ASSERT( !p );

    p = t_none.TryFindBestKarnaughSlice( TruthTable::CellType::FALSE, true, t_none );
    ASSERT( p );
    TRACE(*p)("\n");
    ASSERT( p->size()==0 )(*p);
    
    p = t_none.TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, t_none );
    ASSERT( !p );

    // -------------------------------- STANDARD KARNAUGH --------------------------------------
    // Repeat TestTruthTableBase to get same truth table but
    // without so many render checks.
    t.SetSlice( {}, TruthTable::CellType::TRUE );
    t.Set( {true, true, false, false}, TruthTable::CellType::FALSE );
    t.Set( {true, true, false, true}, TruthTable::CellType::FALSE );
    t.Set( {true, true, true, false}, TruthTable::CellType::FALSE );
    t.Set( {false, false, false, false}, TruthTable::CellType::FALSE );
    t.Set( {false, true, false, true}, TruthTable::CellType::FALSE );
    t.Set( {true, true, false, false}, TruthTable::CellType::TRUE );
    r = t.Render({0, 2}, pred_labels, render_cell_size);    
    TRACE("{true, true, false, false} to true\n")( "\n"+r );
    ASSERT( r=="        ¬p1  p1 ¬p1  p1 \n"
               "        ¬p3 ¬p3  p3  p3 \n"
               "¬p2 ¬p4  ✘   ✔   ✔   ✔  \n"
               " p2 ¬p4  ✔   ✔   ✔   ✘  \n"
               "¬p2  p4  ✔   ✔   ✔   ✔  \n"
               " p2  p4  ✘   ✘   ✔   ✔  \n" );

    TruthTable so_far = t;
    p = t.TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, so_far );
    ASSERT( p );
    TRACE(*p)("\n");
    ASSERT( *p == (map<int, bool>({{2, true}, {3, true}})) )(*p);
    so_far.SetSlice(*p, TruthTable::CellType::FALSE);
    r = so_far.Render({0, 2}, pred_labels, render_cell_size);    
    TRACE( "\n"+r );
        
    p = t.TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, so_far );
    ASSERT( p );
    TRACE(*p)("\n");
    ASSERT( p->size()==2 )(*p);
    so_far.SetSlice(*p, TruthTable::CellType::FALSE);
    r = so_far.Render({0, 2}, pred_labels, render_cell_size);    
    TRACE( "\n"+r );
        
    p = t.TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, so_far );
    ASSERT( p );
    TRACE(*p)("\n");
    ASSERT( p->size()==2 )(*p);
    so_far.SetSlice(*p, TruthTable::CellType::FALSE);
    r = so_far.Render({0, 2}, pred_labels, render_cell_size);    
    TRACE( "\n"+r );
        
    p = t.TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, so_far );
    ASSERT( p );
    TRACE(*p)("\n");
    ASSERT( p->size()==3 )(*p);
    so_far.SetSlice(*p, TruthTable::CellType::FALSE);
    r = so_far.Render({0, 2}, pred_labels, render_cell_size);    
    TRACE( "\n"+r );
    
    p = t.TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, so_far );
    ASSERT( p );
    TRACE(*p)("\n");
    ASSERT( p->size()==2 )(*p);
    so_far.SetSlice(*p, TruthTable::CellType::FALSE);
    r = so_far.Render({0, 2}, pred_labels, render_cell_size);    
    TRACE( "\n"+r );
    
    p = t.TryFindBestKarnaughSlice( TruthTable::CellType::TRUE, true, so_far );
    ASSERT( !p );
}


static void TestTruthTableDefaultMMAX()
{
    vector<string> pred_labels = {"p1", "p2", "p3", "p4"};
    int render_cell_size = 3;

    // This test tries out the MMAX logic for a default MMAX agent (at the time of writing) (harder case)
    // Sat expression is ((x0==M) iff (x1==M)) && (x1!=M => x1==f(x0)).
    // Setting predicates p1:x0==M, p2:x1==M, p3:x1=f(x0),
    // we get predicate form (p1 iff p2) && (!p2 => p3).
    // Each clause gives an expression that we can rule out
    // (but this last step is not necessary; we can get the info we need from repeated evaluation)
    
    // 3-D truth table incorporates predicates p1 to p3
    TruthTable t_analytic( 3, TruthTable::CellType::TRUE );
    TRACE("Constructed truth table\n")( "\n"+t_analytic.Render({1, 2}, pred_labels, render_cell_size) );

    // Filled in by analysis of predicate expression
    t_analytic.SetSlice( {{0, false}, {1, true}}, TruthTable::CellType::FALSE ); // (p1 iff p2)
    t_analytic.SetSlice( {{0, true}, {1, false}}, TruthTable::CellType::FALSE ); // (p1 iff p2) other way
    t_analytic.SetSlice( {{1, false}, {2, false}}, TruthTable::CellType::FALSE ); // !p2 => p3 aka p2 || p3
    TRACE("Initial truth table (after clauses)\n")( "\n"+t_analytic.Render({1, 2}, pred_labels, render_cell_size) );
    
    // Filled in by brute force (repeated evaluation of the predicate form)
    TruthTable t_brute( 3, TruthTable::CellType::TRUE );
    t_brute.Set( {false, false, false}, TruthTable::CellType::FALSE ); 
    t_brute.Set( {false, false, true}, TruthTable::CellType::TRUE ); 
    t_brute.Set( {false, true, false}, TruthTable::CellType::FALSE ); 
    t_brute.Set( {false, true, true}, TruthTable::CellType::FALSE ); 
    t_brute.Set( {true, false, false}, TruthTable::CellType::FALSE ); 
    t_brute.Set( {true, false, true}, TruthTable::CellType::FALSE ); 
    t_brute.Set( {true, true, false}, TruthTable::CellType::TRUE ); 
    t_brute.Set( {true, true, true}, TruthTable::CellType::TRUE ); 
    TRACE("Initial truth table (after brute force)\n")( "\n"+t_brute.Render({1, 2}, pred_labels, render_cell_size) );
    
    ASSERT( t_analytic == t_brute );
    
    // Extract remaining possibles 
    set<vector<bool>> iv = t_brute.GetIndicesOfValue( TruthTable::CellType::TRUE );
    TRACE("Possible {p1, p2, p3}\n")(iv)("\n");
    ASSERT( iv == set<vector<bool>>({{false, false, true}, // not submerged MMAX case, f() applies
                                     {true, true, true},   // submerged MMAX case, don't care f()                           
                                     {true, true, false} }) )(iv); // submerged MMAX case, still don't care f()
}


static void TestTruthTableCoupling()
{
    vector<string> pred_labels = {"p1", "p2", "p3", "p4"};
    int render_cell_size = 3;

    // This test tries out the MMAX logic for a 2-coupling (at the time of writing)
    // Sat expression is ((x0==M) || (x1==M)) || (x0~~x1)).
    // Setting predicates p1:x0==M, p2:x1==M, p3:x0~~x1,
    // we get predicate form (p1 || p2 || p3).
    
    // 3-D truth table incorporates predicates p1 to p3
    TruthTable t( 3, TruthTable::CellType::TRUE );
    TRACE("Constructed truth table\n")( "\n"+t.Render({1, 2}, pred_labels, render_cell_size) );

    // Filled in by brute force (repeated evaluation of the predicate form)
    t.Set( {false, false, false}, TruthTable::CellType::FALSE ); 
    t.Set( {false, false, true}, TruthTable::CellType::TRUE ); 
    t.Set( {false, true, false}, TruthTable::CellType::TRUE ); 
    t.Set( {false, true, true}, TruthTable::CellType::TRUE ); 
    t.Set( {true, false, false}, TruthTable::CellType::TRUE ); 
    t.Set( {true, false, true}, TruthTable::CellType::TRUE ); 
    t.Set( {true, true, false}, TruthTable::CellType::TRUE ); 
    t.Set( {true, true, true}, TruthTable::CellType::TRUE ); 
    TRACE("Initial truth table (after brute force)\n")( "\n"+t.Render({1, 2}, pred_labels, render_cell_size) );

    // TODO imagine a property like x~~M iff x==M ("M is singleton under equivalence")
    // and works for both x0 and x1 (say p4 and p5)
    // Then transitive on ~~ giving p4 && p5 => p3 and rotations.
    // p5 && p3 => p4 would rule out spurious {true, true} result below
        
    // Let's try solving for the case where x0 != M. Thus, p1 is false
    TruthTable ts = t.GetSlice( {{0, false}} );    

    // Extract remaining possibles 
    set<vector<bool>> iv = ts.GetIndicesOfValue( TruthTable::CellType::TRUE );
    TRACE("Possible {p2, p3}\n")(iv)("\n");
    
    ASSERT( iv == set<vector<bool>>({{false, true}, // x1 not MMAX; x1 equivalent to x0
                                     {true, false}, // x1 is MMAX                           
                                     {true, true} }) )(iv); // ax is MMAX and equivalent to x0 (will become empty set) 
}


static void TestTruthTableDisjunction()
{
    vector<string> pred_labels = {"p1", "p2", "p3", "p4", "p5"};
    int render_cell_size = 3;

    // This test tries out the logic for a binary Disjunction agent (at the time of writing)
    // Sat expression is (x1==x0 && x2==M) || (x1==M && x2=x0).
    // Setting predicates p1:=x1==x0, p2:=x2==M, p3:=x1==M, p4:x2==x0
    // we get predicate form (p1 && p2) || (p3 && p4).
    // And this implies expressions (1) to (8) of the form eg !p1 => p3 and similar
    // (but this last step is not necessary; we can get the info we need from repeated evaluation)
    
    // 4-D truth table incorporates predicates p1 to p4
    TruthTable t( 4, TruthTable::CellType::TRUE );
    TRACE("Constructed truth table\n")( "\n"+t.Render({0, 2}, pred_labels, render_cell_size) );

    // Filled in by analysis but could be repeated evaluation of the predicate form
    t.SetSlice( {{0, false}, {2, false}}, TruthTable::CellType::FALSE ); // (1) !p1 => p3 aka p1 || p3
    t.SetSlice( {{1, false}, {2, false}}, TruthTable::CellType::FALSE ); // (3) !p2 => p3 aka p2 || p3
    t.SetSlice( {{1, false}, {3, false}}, TruthTable::CellType::FALSE ); // (4) !p2 => p4 aka p2 || p4
    t.SetSlice( {{3, false}, {0, false}}, TruthTable::CellType::FALSE ); // (7) !p4 => p1 aka p4 || p1
    TRACE("Initial truth table (after (1) (3) (4) (7))\n")( "\n"+t.Render({0, 2}, pred_labels, render_cell_size) );
    
    // (Too lazy to code up the brute force version here)

    // Extend to include extrapolated predicate p5, which is x0==M
    // (we would find this by searching for transitives or substitutions among p1 to p4)
    t.Extend(5);

    TRACE("Extended truth table\n")( "\n"+t.Render({0, 2}, pred_labels, render_cell_size) );

    // Filled in by analysis including extrapolated predicate, searching for transitives/substitutions
    t.SetSlice( {{0, true}, {2, true}, {4, false}}, TruthTable::CellType::FALSE ); // (T1a) Transitive Equal p1 && p3 => p5 aka !p1 || !p3 || p5
    t.SetSlice( {{0, true}, {2, false}, {4, true}}, TruthTable::CellType::FALSE ); // (T1b) Transitive Equal p5 && p1 => p3 aka !p5 || !p1 || p3
    t.SetSlice( {{0, false}, {2, true}, {4, true}}, TruthTable::CellType::FALSE ); // (T1c) Transitive Equal p3 && p5 => p1 aka !p3 || !p5 || p1
    t.SetSlice( {{1, true}, {3, true}, {4, false}}, TruthTable::CellType::FALSE ); // (T2a) Transitive Equal p2 && p4 => p5 aka !p2 || !p4 || p5
    t.SetSlice( {{1, true}, {3, false}, {4, true}}, TruthTable::CellType::FALSE ); // (T2b) Transitive Equal p5 && p2 => p4 aka !p5 || !p2 || p4
    t.SetSlice( {{1, false}, {3, true}, {4, true}}, TruthTable::CellType::FALSE ); // (T2c) Transitive Equal p4 && p5 => p2 aka !p4 || !p5 || p2

    TRACE("Extrapolate-restricted (after (T1) (T2))\n")( "\n"+t.Render({0, 2}, pred_labels, render_cell_size) );

    // Fold out extrapolated literal with OR-rule (because we don't care if it's true OR false)
    TruthTable t2 = t.GetFolded({4}); 

    TRACE("Folded back\n")( "\n"+t2.Render({0, 2}, pred_labels, render_cell_size) );

    // Extract remaining possibles 
    set<vector<bool>> iv = t2.GetIndicesOfValue( TruthTable::CellType::TRUE );
    TRACE("Possible {p1, p2, p3, p4}\n")(iv)("\n");
    ASSERT( iv == set<vector<bool>>({{true, true, true, true}, // submerged MMAX case 
                                     {false, false, true, true}, // New MMAX on the left and match on right
                                     {true, true, false, false} }) )(iv); // Vice-versa
}


void SYM::TestTruthTable()
{
    TestTruthTableBase();
    TestTruthTableDefaultMMAX();
    TestTruthTableCoupling();
    TestTruthTableDisjunction();
    TestTruthTableKarnaugh();
}
