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

TruthTableWithPredicates::TruthTableWithPredicates( vector<EqualPredicateSet> predicates_, bool initval ) :
    predicates( predicates_ ),
    truth_table( make_unique<TruthTable>( predicates.size(), initval ) )
{    
    UpdatePredToIndex();
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
}


TruthTableWithPredicates TruthTableWithPredicates::GetFolded( set<int> fold_axes, bool identity ) const
{
    vector<EqualPredicateSet> new_predicates;
    for( int axis=0; axis<GetDegree(); axis++ )    
        if( fold_axes.count(axis) == 0 ) // this is NOT one of the fold axes
            new_predicates.push_back( predicates.at(axis) );

    auto new_tt = make_unique<TruthTable>( truth_table->GetFolded( fold_axes, identity ) );

    return TruthTableWithPredicates( new_predicates, move(new_tt) );
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


string TruthTableWithPredicates::Render( set<int> column_axes, string label_var_name, int counting_based ) const
{
    string s;
    for( int axis=0; axis<GetDegree(); axis++ )
    {
        string name = label_var_name + to_string(axis+counting_based);
        s += name + " := " + GetFrontPredicate(axis)->Render() + "\n";
    }
    s += truth_table->Render( column_axes, label_var_name, counting_based );
    return s;
}


TruthTableWithPredicates::TruthTableWithPredicates( const vector<EqualPredicateSet> &predicates_, 
                                                    unique_ptr<TruthTable> truth_table_ ) :
    predicates( predicates_ ),
    truth_table( move(truth_table_) )                                                        
{
    UpdatePredToIndex();
}


void TruthTableWithPredicates::UpdatePredToIndex()
{
    pred_to_index.clear();
    for( int i=0; i<GetDegree(); i++ )
        pred_to_index[GetFrontPredicate(i)] = i;
}

  
    
    