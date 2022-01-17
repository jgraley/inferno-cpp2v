#include "sym_solver.hpp"

#include "boolean_operators.hpp"
#include "comparison_operators.hpp"
#include "symbol_operators.hpp"
#include "primary_expressions.hpp"

using namespace SYM;

// ------------------------- SymSolver --------------------------

SymSolver::SymSolver( shared_ptr<SymbolVariable> target_ ) :
    target( target_ )
{
}


shared_ptr<SymbolExpression> SymSolver::TrySolve( shared_ptr<BooleanExpression> equation ) const
{
    FTRACE("\nTrying to solve:\n")(equation->Render())
          ("\nwith respect to: ")(target->Render())("\n");
    shared_ptr<SymbolExpression> solution = equation->TrySolveFor( target );
    if( solution )
        FTRACEC("and got:\n")(solution->Render())("\n\n");
    else
        FTRACEC("but FAILED\n\n");
    
    return solution;
}
