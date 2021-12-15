#include "symbolic_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"
#include "../sym/comparison_operators.hpp"
#include "../sym/primary_expressions.hpp"
#include "../sym/rewriters.hpp"

using namespace CSP;


SymbolicConstraint::SymbolicConstraint( shared_ptr<SYM::BooleanExpression> expression,
                                        set<VariableId> relevent_variables ) :
    plan( this, expression, relevent_variables )
{
}


SymbolicConstraint::Plan::Plan( SymbolicConstraint *algo_,
                                shared_ptr<SYM::BooleanExpression> expression_,
                                set<VariableId> relevent_variables ) :
    algo( algo_ ),
    consistency_expression( expression_ ),
    sym_solver( consistency_expression )
{
    DetermineVariables( relevent_variables );   
    DetermineHintExpressions();    
}


void SymbolicConstraint::Plan::DetermineVariables( set<VariableId> relevent_variables )
{ 
    // Which variables are required in order to check consistency
    set<VariableId> required_variables = consistency_expression->GetRequiredVariables();
    
    // Filter down to variables relevent to the current solver
    // This means we will permit partial queries when some required variables are 
    // irrelevent. This only happens with coupling keyer/residuals
    set<VariableId> my_required_variables = IntersectionOf( required_variables, 
                                                            relevent_variables );
    variables = ToList(my_required_variables);
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

    SYM::Expression::EvalKit kit { &assignments, knowledge };
    
    //Tracer::RAIIDisable silencer(); // make queries be quiet

    ASSERT(plan.consistency_expression);
    unique_ptr<SYM::BooleanResult> r = plan.consistency_expression->Evaluate( kit );
    if( r->matched == SYM::BooleanResult::TRUE || r->matched == SYM::BooleanResult::UNKNOWN )
        return make_tuple(true, Assignment()); // Successful

    if( !current_var || plan.hint_expressions.count(current_var)==0 )
        return make_tuple(false, Assignment()); // We don't want a hint or don't have expression for one
     
    unique_ptr<SYM::SymbolResult> hint_result = plan.hint_expressions.at(current_var)->Evaluate( kit );
    if( !hint_result->xlink )
        return make_tuple(false, Assignment()); // Could not evaluate expression (eg due partial assignment)
        
    return make_tuple(false, SR::LocatedLink( current_var, hint_result->xlink ));
}


void SymbolicConstraint::Dump() const
{
    TRACE("Degree %d\n", plan.variables.size());
    TRACEC("Consistency expression: ")(plan.consistency_expression->Render())("\n");
    for( auto p : plan.hint_expressions )
        TRACEC("Hint expression for ")(p.first)(" is ")(p.second->Render())("\n");
    TRACEC("Variables: ")(plan.variables)("\n");
}      
