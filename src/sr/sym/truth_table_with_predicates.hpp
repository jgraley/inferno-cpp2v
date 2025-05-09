#ifndef TRUTH_TABLE_WITH_PREDICATES_HPP
#define TRUTH_TABLE_WITH_PREDICATES_HPP

#include "common/common.hpp"
#include "common/orderable.hpp"
#include "predicate_operators.hpp"
#include "truth_table.hpp"

#include <vector>
#include <map>
#include <set>
#include <functional>

using namespace std;

namespace SYM
{
                          
// ------------------------- TruthTableWithPredicates --------------------------

class TruthTableWithPredicates
{
public:
    typedef set<shared_ptr<PredicateOperator>> EqualPredicateSet; // all should compare equal
    
    explicit TruthTableWithPredicates( vector<EqualPredicateSet> predicates, TruthTable::CellType initval, string label_var_name, int counting_based );    
    TruthTableWithPredicates( const TruthTableWithPredicates &other );    
    TruthTableWithPredicates &operator=( const TruthTableWithPredicates &other ); 
    
    unsigned GetDegree() const;
    shared_ptr<PredicateOperator> GetFrontPredicate( int axis ) const;
    EqualPredicateSet GetPredicateSet( int axis ) const;
    void Extend( vector<EqualPredicateSet> new_predicates );
    TruthTableWithPredicates GetSlice( TruthTable::SliceSpec slice ) const; 
    TruthTableWithPredicates GetFolded( set<int> fold_axes ) const;
    bool PredExists( shared_ptr<PredicateOperator> pred ) const;
    int PredToIndex( shared_ptr<PredicateOperator> pred ) const;
    string Render( set<int> column_axes, bool give_preds=true ) const; 
    
    vector<EqualPredicateSet> &GetPredicates() { return predicates; }
    TruthTable &GetTruthTable() { return *truth_table; }
    
private:    
    explicit TruthTableWithPredicates( string label_var_name, 
                                       int render_cell_size,
                                       string label_fmt,
                                       const vector<EqualPredicateSet> &predicates, 
                                       shared_ptr<TruthTable> truth_table, 
                                       vector<string> pred_labels, 
                                       int next_pred_num );    
    void UpdatePredToIndex();

    const int render_index_size = 2; // 2 digit
    const string label_var_name;
    const int render_cell_size;
    const string label_fmt;
    vector<EqualPredicateSet> predicates;
    shared_ptr<TruthTable> truth_table;
    map<shared_ptr<PredicateOperator>, int, Expression::Relation> pred_to_index;
    vector<string> pred_labels;
    int next_pred_num;
};

};

#endif
