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
    explicit TruthTableWithPredicates( vector<EqualPredicateSet> predicates, bool initval );    
    int GetDegree() const;
    shared_ptr<PredicateOperator> GetFrontPredicate( int axis ) const;
    EqualPredicateSet GetPredicateSet( int axis ) const;
    void Extend( vector<EqualPredicateSet> new_predicates );
    TruthTableWithPredicates GetFolded( set<int> fold_axes, bool identity ) const;
    bool PredExists( shared_ptr<PredicateOperator> pred ) const;
    int PredToIndex( shared_ptr<PredicateOperator> pred ) const;
    string Render( set<int> column_axes, string label_var_name, int counting_based ) const; 
    
    vector<EqualPredicateSet> &GetPredicates() { return predicates; }
    TruthTable &GetTruthTable() { return *truth_table; }
    
private:    
    explicit TruthTableWithPredicates( const vector<EqualPredicateSet> &predicates, unique_ptr<TruthTable> truth_table );    
    void UpdatePredToIndex();

    vector<EqualPredicateSet> predicates;
    unique_ptr<TruthTable> truth_table;
    map<shared_ptr<PredicateOperator>, int, Expression::OrderComparer> pred_to_index;
};

};

#endif
