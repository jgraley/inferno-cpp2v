#include "symbolic_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"
#include "../sym/comparison_operators.hpp"
#include "../sym/primary_expressions.hpp"
#include "../sym/rewriters.hpp"

using namespace CSP;


SymbolicConstraint::SymbolicConstraint( shared_ptr<SYM::BooleanExpression> expression,
                                        set<SR::PatternLink> relevent_plinks ) :
    plan( this, expression, relevent_plinks )
{
}


SymbolicConstraint::Plan::Plan( SymbolicConstraint *algo_,
                                shared_ptr<SYM::BooleanExpression> expression_,
                                set<SR::PatternLink> relevent_plinks ) :
    algo( algo_ ),
    expression( expression_ )
{
    DetermineVariables( relevent_plinks );       
}


void SymbolicConstraint::Plan::DetermineVariables( set<SR::PatternLink> relevent_plinks )
{ 
    // The keyer
    set<SR::PatternLink> required_plinks = expression->GetRequiredPatternLinks();
    
    // Filter down to variables relevent to the current solver
    set<SR::PatternLink> my_required_plinks = IntersectionOf( required_plinks, relevent_plinks );
    for( VariableId plink : my_required_plinks )
        variables.push_back( plink );     
}


string SymbolicConstraint::Plan::GetTrace() const 
{
    return algo->GetName() + ".plan";
}


const list<VariableId> &SymbolicConstraint::GetVariables() const
{ 
    return plan.variables;
}


void SymbolicConstraint::Start( const Assignments &forces_map_, 
                                const SR::TheKnowledge *knowledge_ )
{
    forces_map = forces_map_;
    knowledge = knowledge_;
    ASSERT( knowledge );
}   


tuple<bool, Assignment> SymbolicConstraint::Test( Assignments frees_map,
                                                  VariableId current_var )
{   
    INDENT("T");

    // Merge incoming values with the forces to get a full set of 
    // values that must tally up with the links required by the operator.
    SR::SolutionMap full_map;
    full_map = UnionOfSolo(forces_map, frees_map);
    SYM::Expression::EvalKit kit { &full_map, knowledge };
    
    //Tracer::RAIIDisable silencer(); // make queries be quiet

    ASSERT(plan.expression);
    SYM::BooleanResult r = plan.expression->Evaluate( kit );
    if( r.matched == SYM::BooleanResult::TRUE || r.matched == SYM::BooleanResult::UNKNOWN )
        return make_tuple(true, Assignment()); // Successful

    if( !current_var )
        return make_tuple(false, Assignment()); // Failed and we don't want a hint
    
    SYM::Solver sym_solver(plan.expression);
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
    TRACEC("Expression: ")(plan.expression->Render())("\n");
    TRACEC("Variables: ")(plan.variables)("\n");
}      
