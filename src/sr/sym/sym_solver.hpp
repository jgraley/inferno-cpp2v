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
    typedef set<SR::PatternLink> GivenSymbolSet;

    TruthTableSolver( shared_ptr<BooleanExpression> equation );
    
    void PreSolve();
    
    /** 
     * Attempt to solve our equation in a specified way, according to args.
     * 
     * @param target The expression to solve for, probably needs to be a SymbolVariable
     * @param givens A set of PatternLinks indicating which variables should be assumed evaluatable in the solution
     * 
     * @return solution expression or NULL
     */
    shared_ptr<SymbolExpression> TrySolveFor( shared_ptr<SymbolExpression> target,
                                              const GivenSymbolSet &givens ) const;
    shared_ptr<BooleanExpression> GetAltEquationForTesting() const;
    
private:
    void ConstrainByEvaluating();
    void ConstrainUsingDerived();
    static shared_ptr<PredicateOperator> TryDerive( shared_ptr<PredicateOperator> pi, 
                                                        shared_ptr<PredicateOperator> pj );

    // Logging
    string PredicateName(int i);
    string RenderEquationInTermsOfPredNames();

    const string label_var_name = "p";
    const int counting_based = 1;
    const shared_ptr<BooleanExpression> equation;
    unique_ptr<TruthTableWithPredicates> ttwp;
};


};

#endif
