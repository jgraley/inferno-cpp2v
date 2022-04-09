#include "symbolic_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"
#include "../sym/predicate_operators.hpp"
#include "../sym/primary_expressions.hpp"
#include "../sym/sym_solver.hpp"
#include "../sym/result.hpp"

//#define CHECK_ASSIGNMENTS_INLCUDES_REQUIRED_VARS


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
    TRACE("Trying to solve:\n")(consistency_expression->Render())("\n")
         ("For variables:\n")(variables)("\n");
            
    // Old solver
    SYM::SymSolver my_solver(consistency_expression);
    
    // Truth-table solver
    SYM::TruthTableSolver my_tt_solver(consistency_expression);
    my_tt_solver.PreSolve();    
    alt_expression_for_testing = my_tt_solver.GetAltEquationForTesting();
    
    for( VariableId v : variables )
    {        
        auto v_expr = make_shared<SYM::SymbolVariable>(v);
        
        //my_tt_solver.TrySolveFor(v_expr); // Just for the logs, for now
        
        shared_ptr<SYM::SymbolExpression> he = my_solver.TrySolveFor(v_expr);
        shared_ptr<SYM::SymbolExpression> hett = my_tt_solver.TrySolveFor(v_expr);
        if( he )
        {
            TRACEC("Solved old style for variable: ")(v)
                  ("\nSolution:\n")(he->Render())("\n");
            hint_expressions[v] = he; // only store good ones in the map            
        }
        if( hett )
        {
            TRACEC("Solved truth-table style for variable: ")(v)
                  ("\nSolution:\n")(he->Render())("\n");
            hint_expressions_tt[v] = hett; // only store good ones in the map            
        }
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


tuple<bool, Hint> SymbolicConstraint::Test( const Assignments &assignments,
                                            const VariableId &current_var )
{   
    INDENT("T");
    ASSERT(plan.consistency_expression);

#ifdef CHECK_ASSIGNMENTS_INLCUDES_REQUIRED_VARS
    for( VariableId v : plan.variables )
        ASSERT( assignments.count(v)==1 );
#endif        

    SYM::Expression::EvalKit kit { &assignments, knowledge };    
    shared_ptr<SYM::BooleanResultInterface> result = plan.consistency_expression->Evaluate( kit );
    ASSERT( result );
    if( plan.alt_expression_for_testing )
    {
        // Test that the truth table solver's version of the sat equation
        // agrees with the original sat equation
        shared_ptr<SYM::BooleanResultInterface> alt_result = plan.alt_expression_for_testing->Evaluate( kit );
        ASSERT( alt_result );
        ASSERT( *alt_result == *result )(*alt_result)(" != ")(*result);
    }
    
    if( result->IsDefinedAndTrue() )
        return make_tuple(true, Hint()); // Successful
      
#if 0
    if( !current_var || plan.hint_expressions_tt.count(current_var)==0 )
        return make_tuple(false, Hint()); // We don't want a hint or don't have expression for one in the plan

    shared_ptr<SYM::SymbolExpression> hint_expression_tt = plan.hint_expressions_tt.at(current_var);
    shared_ptr<SYM::SymbolResultInterface> hint_result_tt = hint_expression_tt->Evaluate( kit );
    ASSERT( hint_result_tt );
    set<SR::XLink> hint_links_tt;
    bool ok_tt = hint_result_tt->TryGetAsSetOfXLinks(hint_links_tt);
    if( !ok_tt || hint_links_tt.empty() )
        return make_tuple(false, Hint(current_var, {})); // effectively a failure to evaluate
          
    Hint hint( current_var, {hint_links_tt} );
#else
    if( !current_var || plan.hint_expressions.count(current_var)==0 )
        return make_tuple(false, Hint()); // We don't want a hint or don't have expression for one in the plan

    shared_ptr<SYM::SymbolExpression> hint_expression = plan.hint_expressions.at(current_var);
    shared_ptr<SYM::SymbolResultInterface> hint_result = hint_expression->Evaluate( kit );
    ASSERT( hint_result );
    set<SR::XLink> hint_links;
    bool ok = hint_result->TryGetAsSetOfXLinks(hint_links);
    if( !ok || hint_links.empty() )
        return make_tuple(false, Hint(current_var, {})); // effectively a failure to evaluate
          
    Hint hint( current_var, {hint_links} );
#endif
    
    return make_tuple(false, hint);
}


void SymbolicConstraint::Dump() const
{
    TRACE("Degree: ")(plan.variables.size())("\n"); // keep this - useful for quickly checking the degrees
    TRACEC("Variables: ")(plan.variables)("\n");
    TRACEC("Consistency expression: ")(plan.consistency_expression->Render())("\n");
    for( auto p : plan.hint_expressions )
        TRACEC("Hint expression for ")(p.first)(" is ")(p.second->Render())("\n");
}      
