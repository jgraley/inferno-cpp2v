#include "symbolic_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"
#include "../sym/predicate_operators.hpp"
#include "../sym/symbol_operators.hpp"
#include "../sym/sym_solver.hpp"
#include "../sym/result.hpp"

#include <inttypes.h> 

//#define CHECK_ASSIGNMENTS_INLCUDES_REQUIRED_VARS
#define COMPARE_HINTS

using namespace CSP;

SymbolicConstraint::SymbolicConstraint( shared_ptr<SYM::BooleanExpression> expression ) :
    plan( this, expression )
{
}


SymbolicConstraint::Plan::Plan( SymbolicConstraint *algo_,
                                shared_ptr<SYM::BooleanExpression> expression_ ) :
    algo( algo_ ),
    consistency_expression( expression_ )
{
    DetermineVariables();   
    DetermineHintExpressions();    
}


void SymbolicConstraint::Plan::DetermineVariables()
{ 
    // Which variables are required in order to check consistency
    variables = consistency_expression->GetRequiredVariables();
}


void SymbolicConstraint::Plan::DetermineHintExpressions()
{
    TRACE("Trying to solve:\n")(consistency_expression->Render())("\n");
   
    // Truth-table solver
    SYM::TruthTableSolver my_solver(consistency_expression);
    my_solver.PreSolve();    
        
    for( VariableId v : variables )
    {        
        auto target = make_shared<SYM::SymbolVariable>(v);
        TRACEC("Solving for variable: ")(v)("\n");
        
        SYM::TruthTableSolver::GivenSymbolSet other_vars = variables;           
        other_vars.erase(v);

        vector<VariableId> other_vars_vec = ToVector( other_vars );
        ForPower<bool>( other_vars_vec.size(), index_range_bool, [&](vector<bool> indices)
        {
            SYM::TruthTableSolver::GivenSymbolSet givens;
            for( int i=0; i<other_vars_vec.size(); i++ )            
                if( indices.at(i) )
                    givens.insert( other_vars_vec[i] );
            
            shared_ptr<SYM::SymbolExpression> he = my_solver.TrySolveFor(target, givens);
            if( he )
            {
                TRACEC("Given:\n")(givens)
                      ("\nSolution:\n")(he->Render())("\n");
                suggestion_expressions[v][givens] = he; // only store good ones in the map            
            }            
        });                       
    }
}


string SymbolicConstraint::Plan::GetTrace() const 
{
    return algo->GetTrace() + ".plan";
}


const set<VariableId> &SymbolicConstraint::GetVariables() const
{ 
    return plan.variables;
}


void SymbolicConstraint::Start( const SR::TheKnowledge *knowledge_ )
{
    knowledge = knowledge_;
    ASSERT( knowledge );
}   


bool SymbolicConstraint::IsConsistent( const Assignments &assignments ) const
{   
    INDENT("T");
    ASSERT(plan.consistency_expression);

#ifdef CHECK_ASSIGNMENTS_INLCUDES_REQUIRED_VARS
    for( VariableId v : plan.variables )
        ASSERT( assignments.count(v)==1 );
#endif        

    SYM::Expression::EvalKit kit { &assignments, knowledge };    
    shared_ptr<SYM::BooleanResult> result = plan.consistency_expression->Evaluate( kit );
    ASSERT( result );
    if( plan.alt_expression_for_testing )
    {
        // Test that the truth table solver's version of the sat equation
        // agrees with the original sat equation
        shared_ptr<SYM::BooleanResult> alt_result = plan.alt_expression_for_testing->Evaluate( kit );
        ASSERT( alt_result );
        ASSERT( *alt_result == *result )(*alt_result)(" != ")(*result);
    }
    
    return result->IsDefinedAndTrue(); 
}


shared_ptr<SYM::SetResult> SymbolicConstraint::GetSuggestedValues( const Assignments &assignments,
                                                                   const VariableId &target_var ) const
{                                 
    ASSERT( target_var );
    SYM::Expression::EvalKit kit { &assignments, knowledge };    

    SYM::TruthTableSolver::GivenSymbolSet givens;
    for( VariableId v : plan.variables )            
        if( v != target_var && assignments.count(v) > 0 )
            givens.insert( v );

    if( plan.suggestion_expressions.count(target_var)==0 ||
        plan.suggestion_expressions.at(target_var).count(givens)==0 )
        return nullptr;
        
    shared_ptr<SYM::SymbolExpression> hint_expression = plan.suggestion_expressions.at(target_var).at(givens);
    shared_ptr<SYM::SymbolResultInterface> hr = hint_expression->Evaluate( kit );
    ASSERT( hr );
    shared_ptr<SYM::SetResult> hint_result = dynamic_pointer_cast<SYM::SetResult>(hr);
    ASSERT( hint_result );
    return hint_result;
}


void SymbolicConstraint::Dump() const
{
    TRACE("Degree: ")(plan.variables.size())("\n"); // keep this - useful for quickly checking the degrees
    TRACEC("Variables: ")(plan.variables)("\n");
    TRACEC("Consistency expression: ")(plan.consistency_expression->Render())("\n");
    for( auto p : plan.suggestion_expressions )
    {
        TRACEC("Suggestion expression for target ")(p.first)(":\n");
        for( auto pp : p.second )
            TRACEC("Given ")(pp.first)(" is ")(pp.second->Render())("\n");
    }
}      


