#include "sym_solver.hpp"

#include "boolean_operators.hpp"
#include "comparison_operators.hpp"
#include "symbol_operators.hpp"
#include "primary_expressions.hpp"

//#define FTRACE_SUCCESSES_AND_FAILURES

using namespace SYM;

// ------------------------- SymSolver --------------------------

SymSolver::SymSolver( shared_ptr<SymbolExpression> target_ ) :
    target( target_ )
{
}


shared_ptr<SymbolExpression> SymSolver::TrySolve( shared_ptr<BooleanExpression> equation ) const
{
#ifdef FTRACE_SUCCESSES_AND_FAILURES
    FTRACEC("\n----------------------------Trying to solve:\n")(equation->Render())
           ("\nwith respect to: ")(target->Render())("\n");
#endif

    shared_ptr<Expression> solution = equation->TrySolveFor( target );

#ifdef FTRACE_SUCCESSES_AND_FAILURES
    if( solution )
        FTRACEC("---------------------------and got:\n")(solution->Render())("\n\n");
    else
        FTRACEC("---------------------------but FAILED\n\n");
#endif

    if( !solution )
        return nullptr;
    
    auto sym_solution = dynamic_pointer_cast<SymbolExpression>(solution);
    ASSERT( sym_solution )(solution->Render()); // target is shared_ptr<SymbolicExpression> so should get symbol as solution
    return sym_solution;
}
