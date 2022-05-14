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
    void SetSlice( map<int, bool> fixed_map, bool new_value );
    TruthTableWithPredicates GetSlice( map<int, bool> fixed_map ) const; 
    TruthTableWithPredicates GetFolded( set<int> fold_axes, bool identity ) const;
    bool PredExists( shared_ptr<PredicateOperator> pred ) const;
    int PredToIndex( shared_ptr<PredicateOperator> pred ) const;
    int CountInSlice( map<int, bool> fixed_map, bool target_value ) const; 
    shared_ptr<map<int, bool>> TryFindBestKarnaughSlice( bool target_value, bool preferred_index, const TruthTableWithPredicates &so_far ) const;
    string Render( set<int> column_axes ) const; 
    
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
    map<shared_ptr<PredicateOperator>, int, Expression::OrderComparer> pred_to_index;
    vector<string> pred_labels;
    int next_pred_num;
};

};

#endif
