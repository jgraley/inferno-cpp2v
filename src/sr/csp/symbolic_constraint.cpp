#include "symbolic_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"
#include "../sym/predicate_operators.hpp"
#include "../sym/primary_expressions.hpp"
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


bool SymbolicConstraint::IsConsistent( const Assignments &assignments ) const
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
    
    return result->IsDefinedAndTrue(); 
}


pair<bool, set<Value>> SymbolicConstraint::GetSuggestedValues( const Assignments &assignments,
                                                               const VariableId &var ) const
{                                 
    ASSERT( var );
    SYM::Expression::EvalKit kit { &assignments, knowledge };    
    set<Value> hint_links_tt;
    bool ok_tt = false;
    if( plan.hint_expressions_tt.count(var)>0 )
    {
        shared_ptr<SYM::SymbolExpression> hint_expression_tt = plan.hint_expressions_tt.at(var);
        shared_ptr<SYM::SymbolResultInterface> hint_result_tt = hint_expression_tt->Evaluate( kit );
        ASSERT( hint_result_tt );
        ok_tt = hint_result_tt->TryGetAsSetOfXLinks(hint_links_tt);
    }

    set<Value> hint_links;
    bool ok = false;
    if( plan.hint_expressions.count(var)>0 )
    {
        shared_ptr<SYM::SymbolExpression> hint_expression = plan.hint_expressions.at(var);
        shared_ptr<SYM::SymbolResultInterface> hint_result = hint_expression->Evaluate( kit );
        ASSERT( hint_result );
        ok = hint_result->TryGetAsSetOfXLinks(hint_links);
    }
#ifdef COMPARE_HINTS    
    if( ok )
    {
        ASSERT( ok_tt );
        ASSERT( hint_links_tt.size() <= hint_links.size() );
    }
#endif
    gsv_n++;
    if( !ok_tt )
    {
        gsv_nfail++;
        return make_pair(false, set<Value>()); // effectively a failure to evaluate         
    }
    //ASSERT( !hint_links_tt.empty() )(var)("\n")(plan.hint_expressions_tt.at(var));
    if( hint_links_tt.empty() )
        gsv_nempty++;
    else
        gsv_tot += hint_links_tt.size();
    return make_pair(true, hint_links_tt);
}


void SymbolicConstraint::Dump() const
{
    TRACE("Degree: ")(plan.variables.size())("\n"); // keep this - useful for quickly checking the degrees
    TRACEC("Variables: ")(plan.variables)("\n");
    TRACEC("Consistency expression: ")(plan.consistency_expression->Render())("\n");
    for( auto p : plan.hint_expressions )
        TRACEC("Hint expression for ")(p.first)(" is ")(p.second->Render())("\n");
}      


void SymbolicConstraint::DumpGSV()
{
    FTRACES("GetSuggestedValues dump\n");
    FTRACEC("Failure to extensionalise: %f%%\n", 100.0*gsv_nfail/gsv_n);
    FTRACEC("Empty set: %f%%\n", 100.0*gsv_nempty/gsv_n);
    FTRACEC("Average size of successful, non-empty: %f\n", 1.0*gsv_tot/(gsv_n-gsv_nfail-gsv_nempty));
}


uint64_t SymbolicConstraint::gsv_n = 0;
uint64_t SymbolicConstraint::gsv_nfail = 0;
uint64_t SymbolicConstraint::gsv_nempty = 0;
uint64_t SymbolicConstraint::gsv_tot = 0;
