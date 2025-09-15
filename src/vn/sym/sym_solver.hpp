#ifndef SYM_SOLVER_HPP
#define SYM_SOLVER_HPP

#include "expression.hpp"
#include "truth_table.hpp"
#include "truth_table_with_predicates.hpp"

namespace SYM
{         

class SymbolVariable;
class PredicateOperator;

// -------------------------- TruthTableSolver ----------------------------    

class TruthTableSolver
{
public:
    typedef set<VN::PatternLink> GivenSymbolSet;

    TruthTableSolver( const Expression::SolveKit &kit,
                      shared_ptr<BooleanExpression> initial_expression );
    
    /** 
     * Perform analysis on the initial expression that will assist with
     * subsequent solves.
     *
     * @note Solve methods are named per rule #527
     */
    void PreSolve();
    
    /** 
     * Attempt to solve our initial expression for a target variable, expecting 
     * certain given variables to be assumed evaluatable when evaluating the solution.
     * 
     * @param target The expression to solve for, probably needs to be a SymbolVariable
     * @param givens A set of PatternLinks indicating which variables are given
     *
     * @note Solve methods are named per rule #527
     * 
     * @return solution expression or NULL
     */
    shared_ptr<SymbolExpression> TrySolveForGiven( shared_ptr<SymbolVariable> target,
                                                   const GivenSymbolSet &givens ) const;
    shared_ptr<BooleanExpression> GetExpressionViaKarnaughMap( TruthTableWithPredicates initial_ttwp ) const;
    shared_ptr<BooleanExpression> GetAltExpressionForTesting() const;
    
private:
    void ConstrainByEvaluating();
    void ConstrainUsingDerived();

    Relationship TryDeriveRelationship( shared_ptr<PredicateOperator> pi, 
                                        shared_ptr<PredicateOperator> pj ) const;
    shared_ptr<PredicateOperator> TryDerivePredicate( shared_ptr<PredicateOperator> pi, 
                                                      shared_ptr<PredicateOperator> pj ) const;
    shared_ptr<PredicateOperator> Substitute( shared_ptr<PredicateOperator> pred,
                                              shared_ptr<SymbolExpression> over,
                                              shared_ptr<SymbolExpression> with ) const;
    // Logging
    string PredicateName(int i);
    string RenderInitialExpressionInTermsOfPredNames();

    const string label_var_name = "p";
    const int counting_based = 0;
    const shared_ptr<BooleanExpression> initial_expression;
    const Expression::SolveKit &kit;                                            
    unique_ptr<TruthTableWithPredicates> ttwp;
};


};

#endif
