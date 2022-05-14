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

             
class TruthTable;             
             
// ------------------------- TruthTableWithPredicates --------------------------

TruthTableWithPredicates::TruthTableWithPredicates( vector<EqualPredicateSet> predicates_, 
                                                    bool initval, 
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

    for( int i=0; i<GetDegree(); i++ )
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


int TruthTableWithPredicates::GetDegree() const
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
    for( int i=0; i<new_predicates.size(); i++ )
        pred_labels.push_back( SSPrintf(label_fmt.c_str(), next_pred_num++) );    
}


void TruthTableWithPredicates::SetSlice( map<int, bool> fixed_map, bool new_value )
{
    truth_table->SetSlice( fixed_map, new_value );
}


TruthTableWithPredicates TruthTableWithPredicates::GetSlice( map<int, bool> fixed_map ) const
{
    vector<EqualPredicateSet> new_predicates;
    vector<string> new_pred_labels;
    for( int axis=0; axis<GetDegree(); axis++ )
    {  
        if( fixed_map.count(axis) == 0 ) // this is NOT one of the fixed axes
        {
            new_predicates.push_back( predicates.at(axis) );
            new_pred_labels.push_back( pred_labels.at(axis) );
        }
    }

    auto new_tt = make_shared<TruthTable>( truth_table->GetSlice( fixed_map ) );

    return TruthTableWithPredicates( label_var_name, render_cell_size, label_fmt, 
                                     new_predicates, new_tt, 
                                     new_pred_labels, next_pred_num );
}


TruthTableWithPredicates TruthTableWithPredicates::GetFolded( set<int> fold_axes, bool identity ) const
{
    vector<EqualPredicateSet> new_predicates;
    vector<string> new_pred_labels;
    for( int axis=0; axis<GetDegree(); axis++ )
    {  
        if( fold_axes.count(axis) == 0 ) // this is NOT one of the fold axes
        {
            new_predicates.push_back( predicates.at(axis) );
            new_pred_labels.push_back( pred_labels.at(axis) );
        }
    }
    
    auto new_tt = make_shared<TruthTable>( truth_table->GetFolded( fold_axes, identity ) );

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


int TruthTableWithPredicates::CountInSlice( map<int, bool> fixed_map, bool target_value ) const
{
    return truth_table->CountInSlice( fixed_map, target_value );
}


shared_ptr<map<int, bool>> TruthTableWithPredicates::TryFindBestKarnaughSlice( bool target_value, bool preferred_index, const TruthTableWithPredicates &so_far ) const
{
    return truth_table->TryFindBestKarnaughSlice( target_value, preferred_index, *(so_far.truth_table) );
}


string TruthTableWithPredicates::Render( set<int> column_axes ) const
{    
    string s;
    for( int axis=0; axis<GetDegree(); axis++ )
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
    for( int i=0; i<GetDegree(); i++ )
        pred_to_index[GetFrontPredicate(i)] = i;
}
