#ifndef TRUTH_TABLE_WITH_PREDICATES_HPP
#define TRUTH_TABLE_WITH_PREDICATES_HPP

#include "common/common.hpp"
#include "common/orderable.hpp"
#include "predicate_operators.hpp"

#include <vector>
#include <map>
#include <set>
#include <functional>

using namespace std;

namespace SYM
{
             
class TruthTable;             
             
// ------------------------- TruthTableWithPredicates --------------------------

class TruthTableWithPredicates
{
public:
    typedef set<shared_ptr<PredicateOperator>> EqualPredicateSet; // all should compare equal
    
    explicit TruthTableWithPredicates( vector<EqualPredicateSet> predicates, bool initval, string label_var_name, int counting_based );    
    TruthTableWithPredicates( const TruthTableWithPredicates &other );    
    TruthTableWithPredicates &operator=( const TruthTableWithPredicates &other ); 
    
    int GetDegree() const;
    shared_ptr<PredicateOperator> GetFrontPredicate( int axis ) const;
    EqualPredicateSet GetPredicateSet( int axis ) const;
    void Extend( vector<EqualPredicateSet> new_predicates );
    TruthTableWithPredicates GetFolded( set<int> fold_axes, bool identity ) const;
    bool PredExists( shared_ptr<PredicateOperator> pred ) const;
    int PredToIndex( shared_ptr<PredicateOperator> pred ) const;
    string Render( set<int> column_axes ) const; 
    
    vector<EqualPredicateSet> &GetPredicates() { return predicates; }
    TruthTable &GetTruthTable() { return *truth_table; }
    
private:    
    explicit TruthTableWithPredicates( const vector<EqualPredicateSet> &predicates, 
                                       shared_ptr<TruthTable> truth_table, 
                                       string label_var_name, 
                                       int render_cell_size,
                                       string label_fmt,
                                       vector<string> pred_labels, 
                                       int next_pred_num );    
    void UpdatePredToIndex();

    const int render_index_size = 1; // 1 digit
    const string label_var_name;
    const int render_cell_size;
    const string label_fmt;
    vector<EqualPredicateSet> predicates;
    shared_ptr<TruthTable> truth_table;
    map<shared_ptr<PredicateOperator>, int, Expression::OrderComparer> pred_to_index;
    vector<string> pred_labels;
    int next_pred_num;

};

};

#endif
