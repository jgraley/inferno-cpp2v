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
    TruthTableSolver( shared_ptr<BooleanExpression> expr );
    void PreSolve();
    
private:
    void PopulateInitial();

    const shared_ptr<BooleanExpression> expr;
    vector<shared_ptr<PredicateOperator>> predicates;
    unique_ptr<TruthTable> truth_table;
};


};

#endif
