#include "truth_table_with_predicates.hpp"

#include "common/common.hpp"
#include "common/orderable.hpp"
#include "predicate_operators.hpp"
#include "truth_table.hpp"

#include <vector>
#include <map>
#include <set>
#include <functional>

using namespace SYM;          
             
// ------------------------- TruthTableWithPredicates --------------------------

TruthTableWithPredicates::TruthTableWithPredicates( vector<EqualPredicateSet> predicates_, 
                                                    TruthTable::CellType initval, 
                                                    string label_var_name_, 
                                                    int counting_based ) :
    label_var_name( label_var_name_ ),
    render_cell_size( render_index_size+label_var_name.size()+1 ),  // +1 for the ¬ 
    label_fmt( SSPrintf("%s%%0%dd", label_var_name.c_str(), render_cell_size-2) ),
    predicates( predicates_ ),
    truth_table( make_shared<TruthTable>( predicates.size(), initval ) ),
    next_pred_num( counting_based )
{    
    UpdatePredToIndex();

    for( unsigned i=0; i<GetDegree(); i++ )
        pred_labels.push_back( SSPrintf(label_fmt.c_str(), next_pred_num++) );    
}
 
 
TruthTableWithPredicates::TruthTableWithPredicates( const TruthTableWithPredicates &other ) :
    label_var_name( other.label_var_name ),
    render_cell_size( other.render_cell_size ),
    label_fmt( other.label_fmt ),
    predicates( other.predicates ),
    truth_table( make_shared<TruthTable>(*other.truth_table) ),
    pred_to_index( other.pred_to_index ),
    pred_labels( other.pred_labels ),
    next_pred_num( other.next_pred_num )
{
}


TruthTableWithPredicates &TruthTableWithPredicates::operator=( const TruthTableWithPredicates &other )
{
    predicates = other.predicates;
    truth_table = make_shared<TruthTable>(*other.truth_table);
    pred_to_index = other.pred_to_index;
    pred_labels = other.pred_labels;
    next_pred_num = other.next_pred_num;
    return *this;
}


unsigned TruthTableWithPredicates::GetDegree() const
{
    ASSERT( truth_table->GetDegree() == predicates.size() );
    return truth_table->GetDegree();
}
    
    
shared_ptr<PredicateOperator> TruthTableWithPredicates::GetFrontPredicate( int axis ) const
{
    return FrontOf( predicates.at(axis) );
}
    
    
TruthTableWithPredicates::EqualPredicateSet TruthTableWithPredicates::GetPredicateSet( int axis ) const
{
    return predicates.at(axis);
}
    
    
void TruthTableWithPredicates::Extend( vector<EqualPredicateSet> new_predicates )
{
    TRACE("Extending truth table from %d by %d\n", GetDegree(), new_predicates.size());
    truth_table->Extend( GetDegree() + new_predicates.size() ); 
    predicates = predicates + new_predicates;
    UpdatePredToIndex();
    for( unsigned i=0; i<new_predicates.size(); i++ )
        pred_labels.push_back( SSPrintf(label_fmt.c_str(), next_pred_num++) );    
}


TruthTableWithPredicates TruthTableWithPredicates::GetSlice( TruthTable::SliceSpec slice ) const
{
    vector<EqualPredicateSet> new_predicates;
    vector<string> new_pred_labels;
    for( unsigned axis=0; axis<GetDegree(); axis++ )
    {  
        if( slice.count(axis) == 0 ) // this is NOT one of the fixed axes
        {
            new_predicates.push_back( predicates.at(axis) );
            new_pred_labels.push_back( pred_labels.at(axis) );
        }
    }

    auto new_tt = make_shared<TruthTable>( truth_table->GetSlice( slice ) );

    return TruthTableWithPredicates( label_var_name, render_cell_size, label_fmt, 
                                     new_predicates, new_tt, 
                                     new_pred_labels, next_pred_num );
}


TruthTableWithPredicates TruthTableWithPredicates::GetFolded( set<int> fold_axes ) const
{
    vector<EqualPredicateSet> new_predicates;
    vector<string> new_pred_labels;
    for( unsigned axis=0; axis<GetDegree(); axis++ )
    {  
        if( fold_axes.count(axis) == 0 ) // this is NOT one of the fold axes
        {
            new_predicates.push_back( predicates.at(axis) );
            new_pred_labels.push_back( pred_labels.at(axis) );
        }
    }
    
    auto new_tt = make_shared<TruthTable>( truth_table->GetFolded( fold_axes ) );

    return TruthTableWithPredicates( label_var_name, render_cell_size, label_fmt, 
                                     new_predicates, new_tt, 
                                     new_pred_labels, next_pred_num );
}

    
bool TruthTableWithPredicates::PredExists( shared_ptr<PredicateOperator> pred ) const
{
    return pred_to_index.count( pred ) == 1;
}


int TruthTableWithPredicates::PredToIndex( shared_ptr<PredicateOperator> pred ) const
{
    ASSERT( PredExists(pred) );
    return pred_to_index.at( pred );
}


string TruthTableWithPredicates::Render( set<int> column_axes, bool give_preds ) const
{    
    string s = "\n"; // Work around trace prefix
    if( give_preds )
        for( unsigned axis=0; axis<GetDegree(); axis++ )
            s += pred_labels.at(axis) + " := " + GetFrontPredicate(axis)->Render() + "\n";
    s += truth_table->Render( column_axes, pred_labels, render_cell_size );
    return s;
}


TruthTableWithPredicates::TruthTableWithPredicates( string label_var_name_, 
                                                    int render_cell_size_,
                                                    string label_fmt_,
                                                    const vector<EqualPredicateSet> &predicates_, 
                                                    shared_ptr<TruthTable> truth_table_, 
                                                    vector<string> pred_labels_, 
                                                    int next_pred_num_ ) :
    label_var_name( label_var_name_ ),
    render_cell_size( render_cell_size_ ),
    label_fmt( label_fmt_ ),
    predicates( predicates_ ),
    truth_table( truth_table_ ),
    pred_labels( pred_labels_ ),
    next_pred_num( next_pred_num_ )
{
    UpdatePredToIndex();
}


void TruthTableWithPredicates::UpdatePredToIndex()
{
    pred_to_index.clear();
    for( unsigned i=0; i<GetDegree(); i++ )
        pred_to_index[GetFrontPredicate(i)] = i;
}
