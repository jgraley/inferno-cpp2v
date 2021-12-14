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
    consistency_expression( expression_ )
{
    DetermineVariables( relevent_variables );       
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
    for( VariableId v : my_required_variables )
        variables.push_back( v );     
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
    SYM::BooleanResult r = plan.consistency_expression->Evaluate( kit );
    if( r.matched == SYM::BooleanResult::TRUE || r.matched == SYM::BooleanResult::UNKNOWN )
        return make_tuple(true, Assignment()); // Successful

    if( !current_var )
        return make_tuple(false, Assignment()); // Failed and we don't want a hint
    
    SYM::Solver sym_solver(plan.consistency_expression);
    auto solve_for = make_shared<SYM::SymbolVariable>(current_var);
    shared_ptr<SYM::SymbolExpression> solution = sym_solver.TrySolveForSymbol(solve_for);
    if( !solution ) 
        return make_tuple(false, Assignment()); // Failed and could not solve equation
     
    SYM::SymbolResult solution_result = solution->Evaluate( kit );
    if( !solution_result.xlink )
        return make_tuple(false, Assignment()); // Failed and could not evaluate solved equation
        
    return make_tuple(false, SR::LocatedLink( current_var, solution_result.xlink ));
}


void SymbolicConstraint::Dump() const
{
    TRACE("Degree %d\n", plan.variables.size());
    TRACEC("Consistency expression: ")(plan.consistency_expression->Render())("\n");
    TRACEC("Variables: ")(plan.variables)("\n");
}      
