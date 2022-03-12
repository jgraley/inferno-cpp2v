#ifndef SYM_SOLVER_HPP
#define SYM_SOLVER_HPP

#include "expression.hpp"
#include "truth_table.hpp"

namespace SYM
{         

class SymbolVariable;
class PredicateOperator;

// ------------------------- SymSolver --------------------------

class SymSolver
{
public:
    explicit SymSolver( shared_ptr<BooleanExpression> equation );
    shared_ptr<SymbolExpression> TrySolve(shared_ptr<SymbolExpression> target) const;

private:
    const shared_ptr<BooleanExpression> equation;
};

// -------------------------- TruthTableSolver ----------------------------    

class TruthTableSolver
{
public:
    TruthTableSolver( shared_ptr<BooleanExpression> equation );
    void PreSolve();
    
private:
    void PopulateInitial();
    void ConstrainUsingDerived();

    // Logging
    string PredicateName(int i);
    string RenderPredicatesAndPredEquation();

    const string label_var_name = "p";
    const int counting_based = 1;
    const shared_ptr<BooleanExpression> equation;
    vector<set<shared_ptr<PredicateOperator>>> predicates;
    unique_ptr<TruthTable> truth_table;
};


};

#endif
