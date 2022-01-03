#include "symbolic_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"
#include "../sym/comparison_operators.hpp"
#include "../sym/primary_expressions.hpp"
#include "../sym/rewriters.hpp"

//#define CHECK_ASSIGNMENTS_INLCUDES_REQUIRED_VARS
//#define CHECK_HINTS

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
    shared_ptr<SYM::BooleanResult> r = plan.consistency_expression->Evaluate( kit );
    if( r->value == SYM::BooleanResult::TRUE )
        return make_tuple(true, Assignment()); // Successful

    if( !current_var || plan.hint_expressions.count(current_var)==0 )
        return make_tuple(false, Assignment()); // We don't want a hint or don't have expression for one in the plan
     
    shared_ptr<SYM::SymbolExpression> hint_expression = plan.hint_expressions.at(current_var);
    shared_ptr<SYM::SymbolResult> hint_result = hint_expression->Evaluate( kit );
    if( hint_result->xlink == SR::XLink::OffEndLink )
        return make_tuple(false, Assignment()); // effectively a failure to evaluate
        
    SR::LocatedLink hint( current_var, hint_result->xlink );
    
#ifdef CHECK_HINTS
    Assignments hinted_assignments = assignments;
    hinted_assignments[current_var] = hint_result->xlink;
    SYM::Expression::EvalKit hinted_kit { &hinted_assignments, knowledge };
    shared_ptr<SYM::BooleanResult> hinted_r = plan.consistency_expression->Evaluate( hinted_kit );
    ASSERT( hinted_r->value == SYM::BooleanResult::TRUE )("Hint failed check\n")
          ("Consistentcy expression:\n")(plan.consistency_expression)
          ("\nCurrent variable: ")(current_var)
          ("\nHint expression for current variable:\n")(hint_expression)
          ("\nOriginal assignments:\n")(assignments)
          ("\nHint: ")(hint)
          ("\nHinted assignments:\n")(hinted_assignments);          
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
