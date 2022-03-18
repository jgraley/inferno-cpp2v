#include "common/common.hpp"
#include "common/orderable.hpp"

#include "truth_table.hpp"

using namespace SYM;

// ------------------------- TruthTable --------------------------

TruthTable::TruthTable( int degree_, bool initval ) :
    degree( degree_ )
{
    SizeType ncells = (SizeType)1 << degree;
    cells.resize(ncells, initval);
}


TruthTable &TruthTable::operator=( const TruthTable &other )
{
    degree = other.degree;
    cells = other.cells;
    return *this;
}


void TruthTable::Set( vector<bool> full_indices, bool new_value )
{
    cells.at( GetCellIndex(full_indices) ) = new_value;
    // TRACE(cells)("\n");
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


string TruthTable::Render( set<int> column_axes, string label_var_name, int counting_based ) const
{
    ASSERT( column_axes.size() <= degree );
    
    int render_index_size = 1; // 1 digit
    int render_cell_size = render_index_size+label_var_name.size()+1;  // +1 for the ¬
    string label_fmt = SSPrintf("%s%%0%dd", label_var_name.c_str(), render_cell_size-2); 
    
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
                else
                    str_label += " ";                
                str_label += SSPrintf(label_fmt.c_str(), layers.at(layer)+counting_based);
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
    // First test checks for consistency between simple Set()
    // and the renders, and looks for various kinds of swaps.
    TruthTable t( 4, true );
    t.Set( {true, true, false, false}, false );
    string r = t.Render({0, 2}, "p", 1);    
    TRACE("{true, true, false, false} to false\n")( "\n"+r );
    ASSERT( r=="        ¬p1  p1 ¬p1  p1 \n"
               "        ¬p3 ¬p3  p3  p3 \n"
               "¬p2 ¬p4  ✔   ✔   ✔   ✔  \n"
               " p2 ¬p4  ✔   ✘   ✔   ✔  \n"
               "¬p2  p4  ✔   ✔   ✔   ✔  \n"
               " p2  p4  ✔   ✔   ✔   ✔  \n" );
    t.Set( {true, true, false, true}, false );
    r = t.Render({0, 2}, "p", 1);
    TRACE("{true, true, false, true} to false\n")( "\n"+r );
    ASSERT( r=="        ¬p1  p1 ¬p1  p1 \n"
               "        ¬p3 ¬p3  p3  p3 \n"
               "¬p2 ¬p4  ✔   ✔   ✔   ✔  \n"
               " p2 ¬p4  ✔   ✘   ✔   ✔  \n"
               "¬p2  p4  ✔   ✔   ✔   ✔  \n"
               " p2  p4  ✔   ✘   ✔   ✔  \n" );
    t.Set( {true, true, true, false}, false );
    r = t.Render({0, 2}, "p", 1);
    TRACE("{true, true, true, false} to false\n")( "\n"+r );
    ASSERT( r=="        ¬p1  p1 ¬p1  p1 \n"
               "        ¬p3 ¬p3  p3  p3 \n"
               "¬p2 ¬p4  ✔   ✔   ✔   ✔  \n"
               " p2 ¬p4  ✔   ✘   ✔   ✘  \n"
               "¬p2  p4  ✔   ✔   ✔   ✔  \n"
               " p2  p4  ✔   ✘   ✔   ✔  \n" );
    t.Set( {false, false, false, false}, false );
    r = t.Render({0, 2}, "p", 1);
    TRACE("{false, false, false, false} to false\n")( "\n"+r );
    ASSERT( r=="        ¬p1  p1 ¬p1  p1 \n"
               "        ¬p3 ¬p3  p3  p3 \n"
               "¬p2 ¬p4  ✘   ✔   ✔   ✔  \n"
               " p2 ¬p4  ✔   ✘   ✔   ✘  \n"
               "¬p2  p4  ✔   ✔   ✔   ✔  \n"
               " p2  p4  ✔   ✘   ✔   ✔  \n" );
    t.Set( {false, true, false, true}, false );
    r = t.Render({0, 2}, "p", 1);
    TRACE("{false, true, false, true} to false\n")( "\n"+r );
    ASSERT( r=="        ¬p1  p1 ¬p1  p1 \n"
               "        ¬p3 ¬p3  p3  p3 \n"
               "¬p2 ¬p4  ✘   ✔   ✔   ✔  \n"
               " p2 ¬p4  ✔   ✘   ✔   ✘  \n"
               "¬p2  p4  ✔   ✔   ✔   ✔  \n"
               " p2  p4  ✘   ✘   ✔   ✔  \n" );
    // now undo the first one
    t.Set( {true, true, false, false}, true );
    r = t.Render({0, 2}, "p", 1);    
    TRACE("{true, true, false, false} to true\n")( "\n"+r );
    ASSERT( r=="        ¬p1  p1 ¬p1  p1 \n"
               "        ¬p3 ¬p3  p3  p3 \n"
               "¬p2 ¬p4  ✘   ✔   ✔   ✔  \n"
               " p2 ¬p4  ✔   ✔   ✔   ✘  \n"
               "¬p2  p4  ✔   ✔   ✔   ✔  \n"
               " p2  p4  ✘   ✘   ✔   ✔  \n" );
}


static void TestTruthTableDefaultMMAX()
{
    // This test tries out the MMAX logic for a default MMAX agent (at the time of writing) (harder case)
    // Sat expression is ((x0==M) iff (x1==M)) && (x1!=M => x1==f(x0)).
    // Setting predicates p1:x0==M, p2:x1==M, p3:x1=f(x0),
    // we get predicate equation (p1 iff p2) && (!p2 => p3).
    // Each clause gives an equation that we can rule out
    // (but this last step is not necessary; we can get the info we need from repeated evaluation)
    
    // 3-D truth table incorporates predicates p1 to p3
    TruthTable t_analytic( 3, true );
    TRACE("Constructed truth table\n")( "\n"+t_analytic.Render({1, 2}, "p", 1) );

    // Filled in by analysis of predicate expression
    t_analytic.SetSlice( {{0, false}, {1, true}}, false ); // (p1 iff p2)
    t_analytic.SetSlice( {{0, true}, {1, false}}, false ); // (p1 iff p2) other way
    t_analytic.SetSlice( {{1, false}, {2, false}}, false ); // !p2 => p3 aka p2 || p3
    TRACE("Initial truth table (after clauses)\n")( "\n"+t_analytic.Render({1, 2}, "p", 1) );
    
    // Filled in by brute force (repeated evaluation of the predicate equation)
    TruthTable t_brute( 3, true );
    t_brute.Set( {false, false, false}, false ); 
    t_brute.Set( {false, false, true}, true ); 
    t_brute.Set( {false, true, false}, false ); 
    t_brute.Set( {false, true, true}, false ); 
    t_brute.Set( {true, false, false}, false ); 
    t_brute.Set( {true, false, true}, false ); 
    t_brute.Set( {true, true, false}, true ); 
    t_brute.Set( {true, true, true}, true ); 
    TRACE("Initial truth table (after brute force)\n")( "\n"+t_brute.Render({1, 2}, "p", 1) );
    
    ASSERT( t_analytic == t_brute );
    
    // Extract remaining possibles 
    set<vector<bool>> iv = t_brute.GetIndicesOfValue( true );
    TRACE("Possible {p1, p2, p3}\n")(iv)("\n");
    ASSERT( iv == set<vector<bool>>({{false, false, true}, // not submerged MMAX case, f() applies
                                     {true, true, true},   // submerged MMAX case, don't care f()                           
                                     {true, true, false} }) )(iv); // submerged MMAX case, still don't care f()
}


static void TestTruthTableCoupling()
{
    // This test tries out the MMAX logic for a 2-coupling (at the time of writing)
    // Sat expression is ((x0==M) || (x1==M)) || (x0~~x1)).
    // Setting predicates p1:x0==M, p2:x1==M, p3:x0~~x1,
    // we get predicate equation (p1 || p2 || p3).
    
    // 3-D truth table incorporates predicates p1 to p3
    TruthTable t( 3, true );
    TRACE("Constructed truth table\n")( "\n"+t.Render({1, 2}, "p", 1) );

    // Filled in by brute force (repeated evaluation of the predicate equation)
    t.Set( {false, false, false}, false ); 
    t.Set( {false, false, true}, true ); 
    t.Set( {false, true, false}, true ); 
    t.Set( {false, true, true}, true ); 
    t.Set( {true, false, false}, true ); 
    t.Set( {true, false, true}, true ); 
    t.Set( {true, true, false}, true ); 
    t.Set( {true, true, true}, true ); 
    TRACE("Initial truth table (after brute force)\n")( "\n"+t.Render({1, 2}, "p", 1) );

    // TODO imagine a property like x~~M iff x==M ("M is singleton under equivalence")
    // and works for both x0 and x1 (say p4 and p5)
    // Then transitive on ~~ giving p4 && p5 => p3 and rotations.
    // p5 && p3 => p4 would rule out spurious {true, true} result below
        
    // Let's try solving for the case where x0 != M. Thus, p1 is false
    TruthTable ts = t.GetSlice( {{0, false}} );    

    // Extract remaining possibles 
    set<vector<bool>> iv = ts.GetIndicesOfValue( true );
    TRACE("Possible {p2, p3}\n")(iv)("\n");
    
    ASSERT( iv == set<vector<bool>>({{false, true}, // x1 not MMAX; x1 equivalent to x0
                                     {true, false}, // x1 is MMAX                           
                                     {true, true} }) )(iv); // ax is MMAX and equivalent to x0 (will become empty set) 
}


static void TestTruthTableDisjunction()
{
    // This test tries out the logic for a binary Disjunction agent (at the time of writing)
    // Sat expression is (x1==x0 && x2==M) || (x1==M && x2=x0).
    // Setting predicates p1:=x1==x0, p2:=x2==M, p3:=x1==M, p4:x2==x0
    // we get predicate equation (p1 && p2) || (p3 && p4).
    // And this implies equations (1) to (8) of the form eg !p1 => p3 and similar
    // (but this last step is not necessary; we can get the info we need from repeated evaluation)
    
    // 4-D truth table incorporates predicates p1 to p4
    TruthTable t( 4, true );
    TRACE("Constructed truth table\n")( "\n"+t.Render({0, 2}, "p", 1) );

    // Filled in by analysis but could be repeated evaluation of the predicate equation
    t.SetSlice( {{0, false}, {2, false}}, false ); // (1) !p1 => p3 aka p1 || p3
    t.SetSlice( {{1, false}, {2, false}}, false ); // (3) !p2 => p3 aka p2 || p3
    t.SetSlice( {{1, false}, {3, false}}, false ); // (4) !p2 => p4 aka p2 || p4
    t.SetSlice( {{3, false}, {0, false}}, false ); // (7) !p4 => p1 aka p4 || p1
    TRACE("Initial truth table (after (1) (3) (4) (7))\n")( "\n"+t.Render({0, 2}, "p", 1) );
    
    // (Too lazy to code up the brute force version here)

    // Extend to include extrapolated predicate p5, which is x0==M
    // (we would find this by searching for transitives or substitutions among p1 to p4)
    t.Extend(5);

    TRACE("Extended truth table\n")( "\n"+t.Render({0, 2}, "p", 1) );

    // Filled in by analysis including extrapolated predicate, searching for transitives/substitutions
    t.SetSlice( {{0, true}, {2, true}, {4, false}}, false ); // (T1a) Transitive Equal p1 && p3 => p5 aka !p1 || !p3 || p5
    t.SetSlice( {{0, true}, {2, false}, {4, true}}, false ); // (T1b) Transitive Equal p5 && p1 => p3 aka !p5 || !p1 || p3
    t.SetSlice( {{0, false}, {2, true}, {4, true}}, false ); // (T1c) Transitive Equal p3 && p5 => p1 aka !p3 || !p5 || p1
    t.SetSlice( {{1, true}, {3, true}, {4, false}}, false ); // (T2a) Transitive Equal p2 && p4 => p5 aka !p2 || !p4 || p5
    t.SetSlice( {{1, true}, {3, false}, {4, true}}, false ); // (T2b) Transitive Equal p5 && p2 => p4 aka !p5 || !p2 || p4
    t.SetSlice( {{1, false}, {3, true}, {4, true}}, false ); // (T2c) Transitive Equal p4 && p5 => p2 aka !p4 || !p5 || p2

    TRACE("Extrapolate-restricted (after (T1) (T2))\n")( "\n"+t.Render({0, 2}, "p", 1) );

    // Fold out extrapolated literal with OR-rule (because we don't care if it's true OR false)
    TruthTable t2 = t.GetFolded({4}, false); 

    TRACE("Folded back\n")( "\n"+t2.Render({0, 2}, "p", 1) );

    // Extract remaining possibles 
    set<vector<bool>> iv = t2.GetIndicesOfValue( true );
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
}
