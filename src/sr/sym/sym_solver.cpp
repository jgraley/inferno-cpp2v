#include "sym_solver.hpp"

#include "boolean_operators.hpp"
#include "predicate_operators.hpp"
#include "symbol_operators.hpp"
#include "primary_expressions.hpp"
#include "expression_analysis.hpp"
#include "result.hpp"
#include "truth_table.hpp"

using namespace SYM;

// ------------------------- SymSolver --------------------------

SymSolver::SymSolver( shared_ptr<BooleanExpression> equation_ ) :
    equation( equation_ )
{
}


shared_ptr<SymbolExpression> SymSolver::TrySolve( shared_ptr<SymbolExpression> target ) const
{
    shared_ptr<Expression> solution = equation->TrySolveForToEqual( target,
                                                                    make_shared<BooleanConstant>(true) );
    if( !solution )
        return nullptr;
    
    auto sym_solution = dynamic_pointer_cast<SymbolExpression>(solution);
    ASSERT( sym_solution )(solution->Render()); // target is shared_ptr<SymbolicExpression> so should get symbol as solution
    return sym_solution;
}

// -------------------------- TruthTableSolver ----------------------------    

TruthTableSolver::TruthTableSolver( shared_ptr<BooleanExpression> expr_ ) :
    expr( expr_ )
{
}


void TruthTableSolver::PreSolve()
{
    predicates = PredicateAnalysis::GetPredicates( expr );
    // Could de-duplicate?
    
    truth_table = make_unique<TruthTable>( predicates.size(), true );
    
    PopulateInitial();
}


void TruthTableSolver::PopulateInitial()
{
    // Gets them all because they're all true atm
    set<vector<bool>> all_indices = truth_table->GetIndicesOfValue(true);
    const SR::SolutionMap sm{};
    const SR::TheKnowledge tk{};
    Expression::EvalKit kit { &sm, &tk }; 
    
    for( vector<bool> indices : all_indices )
    {
        vector<shared_ptr<BooleanResult>> vr; // must stay in scope across the Evaluate
        
        for( bool b : indices )
            vr.push_back( make_shared<BooleanResult>(ResultInterface::DEFINED, b) );
        
        for( int j=0; j<indices.size(); j++ )
            predicates[j]->SetForceResult( vr[j] );       
            
        shared_ptr<BooleanResultInterface> eval_result = expr->Evaluate(kit);
        
        // Rule out any evaluations that come out false
        if( !eval_result->IsDefinedAndTrue() )
            truth_table->Set( indices, false );
    }
}
