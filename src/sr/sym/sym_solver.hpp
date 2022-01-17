#ifndef SYM_SOLVER_HPP
#define SYM_SOLVER_HPP

#include "expression.hpp"

namespace SYM
{         

class SymbolVariable;

// ------------------------- SymSolver --------------------------

class SymSolver
{
public:
    explicit SymSolver( shared_ptr<SymbolExpression> target );
    shared_ptr<SymbolExpression> TrySolve( shared_ptr<BooleanExpression> equation ) const;

private:
    const shared_ptr<SymbolExpression> target;
};

};

#endif
