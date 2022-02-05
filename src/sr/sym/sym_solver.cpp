#include "sym_solver.hpp"

#include "boolean_operators.hpp"
#include "comparison_operators.hpp"
#include "symbol_operators.hpp"
#include "primary_expressions.hpp"
#include "result.hpp"

using namespace SYM;

// ------------------------- SymSolver --------------------------

SymSolver::SymSolver( shared_ptr<SymbolExpression> target_ ) :
    target( target_ )
{
}


shared_ptr<SymbolExpression> SymSolver::TrySolve( shared_ptr<BooleanExpression> equation ) const
{
    shared_ptr<Expression> solution = equation->TrySolveForToEqual( target,
                                                                    make_shared<BooleanConstant>(true) );
    if( !solution )
        return nullptr;
    
    auto sym_solution = dynamic_pointer_cast<SymbolExpression>(solution);
    ASSERT( sym_solution )(solution->Render()); // target is shared_ptr<SymbolicExpression> so should get symbol as solution
    return sym_solution;
}
