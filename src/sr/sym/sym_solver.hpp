#ifndef SYM_SOLVER_HPP
#define SYM_SOLVER_HPP

#include "expression.hpp"

namespace SR
{ 
    class Agent;
};

namespace SYM
{         

class SymbolVariable;

// Bake in the idea that an "equation" is really just a boolean expression
// which is required to evaluate to TRUE i.e. the "== TRUE` is assumed.
typedef BooleanExpression Equation;

// ------------------------- SymSolver --------------------------

class SymSolver
{
public:
    explicit SymSolver( shared_ptr<SymbolVariable> target );
    shared_ptr<SymbolExpression> TrySolve( shared_ptr<Equation> equation ) const;
    bool IsIndependent( shared_ptr<Expression> expr ) const;

private:
    const shared_ptr<SymbolVariable> target;
};

};

#endif
