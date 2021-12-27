#include "symbolic_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"
#include "../sym/comparison_operators.hpp"
#include "../sym/primary_expressions.hpp"
#include "../sym/rewriters.hpp"

//#define CHECK_ASSIGNMENTS_INLCUDES_REQUIRED_VARS

using namespace CSP;

SymbolicConstraint::SymbolicConstraint( shared_ptr<SYM::BooleanExpression> expression ) :
    plan( this, expression )
{
}


SymbolicConstraint::Plan::Plan( SymbolicConstraint *algo_,
                                shared_ptr<SYM::BooleanExpression> expression_ ) :
    algo( algo_ ),
    consistency_expression( expression_ ),
    sym_solver( consistency_expression )
{
    DetermineVariables();   
    DetermineHintExpressions();    
}


void SymbolicConstraint::Plan::DetermineVariables()
{ 
    // Which variables are required in order to check consistency
    set<VariableId> required_variables = consistency_expression->GetRequiredVariables();
    variables = ToList(required_variables);
}


void SymbolicConstraint::Plan::DetermineHintExpressions()
{
    for( VariableId v : variables )
    {
        auto v_expr = make_shared<SYM::SymbolVariable>(v);
        shared_ptr<SYM::SymbolExpression> he = sym_solver.TrySolveForSymbol(v_expr);
        if( he )
            hint_expressions[v] = he; // only store good ones in the map
    }
}


string SymbolicConstraint::Plan::GetTrace() const 
{
    return algo->GetName() + ".plan";
}


const list<VariableId> &SymbolicConstraint::GetVariables() const
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

#ifdef CHECK_ASSIGNMENTS_INLCUDES_REQUIRED_VARS
    for( VariableId v : plan.variables )
        ASSERT( assignments.count(v)==1 );
#endif        

    SYM::Expression::EvalKit kit { &assignments, knowledge };
    
    ASSERT(plan.consistency_expression);
    shared_ptr<SYM::BooleanResult> r = plan.consistency_expression->Evaluate( kit );
    if( r->value == SYM::BooleanResult::TRUE )
        return make_tuple(true, Assignment()); // Successful

    if( !current_var || plan.hint_expressions.count(current_var)==0 )
        return make_tuple(false, Assignment()); // We don't want a hint or don't have expression for one in the plan
     
    shared_ptr<SYM::SymbolResult> hint_result = plan.hint_expressions.at(current_var)->Evaluate( kit );
    if( !hint_result->xlink )
        return make_tuple(false, Assignment()); // Could not evaluate expression (eg due partial assignment)
    return make_tuple(false, SR::LocatedLink( current_var, hint_result->xlink ));
}


void SymbolicConstraint::Dump() const
{
    TRACE("Degree: ")(plan.variables.size())("\n"); // keep this - useful for quickly checking the degrees
    TRACEC("Variables: ")(plan.variables)("\n");
    TRACEC("Consistency expression: ")(plan.consistency_expression->Render())("\n");
    for( auto p : plan.hint_expressions )
        TRACEC("Hint expression for ")(p.first)(" is ")(p.second->Render())("\n");
}      
