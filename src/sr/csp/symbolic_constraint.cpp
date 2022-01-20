#include "symbolic_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"
#include "../sym/comparison_operators.hpp"
#include "../sym/primary_expressions.hpp"
#include "../sym/sym_solver.hpp"
#include "../sym/result.hpp"

//#define CHECK_ASSIGNMENTS_INLCUDES_REQUIRED_VARS
//#define CHECK_HINTS
#define REJECT_OFF_END_HINT

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
            
    for( VariableId v : variables )
    {
        auto v_expr = make_shared<SYM::SymbolVariable>(v);
        shared_ptr<SYM::SymbolExpression> he = SYM::SymSolver(v_expr).TrySolve(consistency_expression);
        if( he )
        {
            TRACEC("Solved for variable: ")(v)
                  ("\nSolution:\n")(he->Render())("\n");
            hint_expressions[v] = he; // only store good ones in the map            
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


tuple<bool, Assignment> SymbolicConstraint::Test( const Assignments &assignments,
                                                  const VariableId &current_var )
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
    if( result->IsDefinedAndTrue() )
        return make_tuple(true, Assignment()); // Successful

    if( !current_var || plan.hint_expressions.count(current_var)==0 )
        return make_tuple(false, Assignment()); // We don't want a hint or don't have expression for one in the plan
     
    shared_ptr<SYM::SymbolExpression> hint_expression = plan.hint_expressions.at(current_var);
    shared_ptr<SYM::SymbolResultInterface> hint_result = hint_expression->Evaluate( kit );
    ASSERT( hint_result );
    if( !hint_result->IsDefinedAndUnique() )
        return make_tuple(false, Assignment()); // effectively a failure to evaluate
          
    // Testing hint by evaluating using consistency expression with hint substituted over original value
    SR::XLink *p_current_assignment = const_cast<SR::XLink *>(&(assignments.at(current_var)));
    SR::XLink prev_xlink = *p_current_assignment;
    *p_current_assignment = hint_result->GetAsXLink();
    shared_ptr<SYM::BooleanResult> hint_check_result = plan.consistency_expression->Evaluate( kit );
    ASSERT( hint_check_result );
    *p_current_assignment = prev_xlink; // put it back again    
    if( !hint_check_result->IsDefinedAndTrue() )
        return make_tuple(false, Assignment()); // evaluated false using hint - probably inconsistent in the OTHER variables

    SR::LocatedLink hint( current_var, hint_result->GetAsXLink() );
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
