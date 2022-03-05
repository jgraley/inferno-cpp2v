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

TruthTableSolver::TruthTableSolver( shared_ptr<BooleanExpression> equation_ ) :
    equation( equation_ )
{
}


void TruthTableSolver::PreSolve()
{
    string s;
    s += "Presolve equation: " + equation->Render() + "\n\n";
    
    predicates = PredicateAnalysis::GetPredicates( equation );
    // Could de-duplicate? Yes, do but be careful of the forcing - they may not be different (use a std::set for equal ones)
    s += RenderPredicatesAndPredEquation()+"\n";

    truth_table = make_unique<TruthTable>( predicates.size(), true );
    
    PopulateInitial();
    
    s += truth_table->Render( {}, label_var_name, counting_based )+"\n";
    //FTRACE(s);
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
        ASSERT( indices.size() == predicates.size() );
        vector<shared_ptr<BooleanResult>> vr; // must stay in scope across the Evaluate
        
        for( bool b : indices )
            vr.push_back( make_shared<BooleanResult>(ResultInterface::DEFINED, b) );
        
        for( int j=0; j<predicates.size(); j++ )
            for( shared_ptr<PredicateOperator> pred : predicates[j] )
                pred->SetForceResult( vr[j] );       
            
        shared_ptr<BooleanResultInterface> eval_result = equation->Evaluate(kit);
        
        // Rule out any evaluations that come out false
        if( !eval_result->IsDefinedAndTrue() )
            truth_table->Set( indices, false );
    }
}


string TruthTableSolver::PredicateName(int j)
{
    return label_var_name + to_string(j+counting_based);
}


string TruthTableSolver::RenderPredicatesAndPredEquation()
{
    string s;
    vector<shared_ptr<string>> pred_names(predicates.size());
    for( int j=0; j<predicates.size(); j++ )
    {
        string name = PredicateName(j);
        s += name + " := " + (*(predicates[j].begin()))->Render() + "\n";

        pred_names[j] = make_shared<string>(name);
        for( shared_ptr<PredicateOperator> pred : predicates[j] )
            pred->SetForceRender( pred_names[j] ); // want to use later...
    }
    s += "so that we require " + equation->Render() + "\n";
    return s;
}
