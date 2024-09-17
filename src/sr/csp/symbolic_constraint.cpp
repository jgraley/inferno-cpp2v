#include "symbolic_constraint.hpp"

#include "query.hpp"
#include "agents/agent.hpp"
#include "link.hpp"
#include "../sym/predicate_operators.hpp"
#include "../sym/symbol_operators.hpp"
#include "../sym/sym_solver.hpp"
#include "../sym/result.hpp"
#include "common/lambda_loops.hpp"

#include <inttypes.h> 

//#define CHECK_ASSIGNMENTS_INLCUDES_REQUIRED_VARS
#define COMPARE_HINTS

using namespace CSP;

SymbolicConstraint::SymbolicConstraint( shared_ptr<SYM::BooleanExpression> expression,
                                        shared_ptr<const SR::Lacing> lacing ) :
    plan( this, expression, lacing )
{
}


SymbolicConstraint::Plan::Plan( SymbolicConstraint *algo_,
                                shared_ptr<SYM::BooleanExpression> expression_,
                                shared_ptr<const SR::Lacing> lacing_ ) :
    algo( algo_ ),
    consistency_expression( expression_ ),
    lacing( lacing_ )
{
    DetermineVariables();   
    DetermineHintExpressions();   
    DetermineXTreeDbRequirement(); 
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
    SYM::Expression::SolveKit kit { lacing.get() };    
    SYM::TruthTableSolver my_solver(kit, consistency_expression);
    my_solver.PreSolve();    
        
    for( VariableId v : variables )
    {        
        auto target = make_shared<SYM::SymbolVariable>(v);
        TRACEC("Solving for variable: ")(v)("\n");
        
        SYM::TruthTableSolver::GivenSymbolSet other_vars = variables;           
        EraseSolo( other_vars, v );

        vector<VariableId> other_vars_vec = ToVector( other_vars );
        ForPower<bool>( other_vars_vec.size(), index_range_bool, [&](vector<bool> indices)
        {
            SYM::TruthTableSolver::GivenSymbolSet givens;
            for( int i=0; i<other_vars_vec.size(); i++ )            
                if( indices.at(i) )
                    givens.insert( other_vars_vec[i] );
            
            shared_ptr<SYM::SymbolExpression> he = my_solver.TrySolveForGiven(target, givens);
            if( he )            
                suggestion_expressions[v][givens] = he; // only store good ones in the map                        
        });                       
    }
}


void SymbolicConstraint::Plan::DetermineXTreeDbRequirement()
{
    required_x_tree_db_level.clear();
    
    consistency_expression->ForDepthFirstWalk( [&](const SYM::Expression *expr)
    {
        required_x_tree_db_level = UnionOf( required_x_tree_db_level, 
                                            expr->GetVariablesRequiringRows() );
    } );
    
    for( const auto &target_suggestions : suggestion_expressions )
    {
        for( auto suggestion : target_suggestions.second )
        {
            suggestion.second->ForDepthFirstWalk( [&](const SYM::Expression *expr)
            {
                required_x_tree_db_level = UnionOf( required_x_tree_db_level, 
                                                    expr->GetVariablesRequiringRows() );            
            } );
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


SYM::Expression::VariablesRequiringRows SymbolicConstraint::GetVariablesRequiringRows() const
{
    return plan.required_x_tree_db_level;
}


void SymbolicConstraint::Start( const SR::XTreeDatabase *x_tree_db_ )
{
    ASSERT( x_tree_db_ );
	x_tree_db = x_tree_db_;
}   


bool SymbolicConstraint::IsSatisfied( const Assignments &assignments ) const
{   
    INDENT("T");
    ASSERT(plan.consistency_expression);

#ifdef CHECK_ASSIGNMENTS_INLCUDES_REQUIRED_VARS
    for( VariableId v : plan.variables )
        ASSERT( assignments.count(v)==1 );
#endif        
    SYM::Expression::EvalKit kit { &assignments, x_tree_db };    
    unique_ptr<SYM::BooleanResult> result = plan.consistency_expression->Evaluate( kit );
    ASSERT( result );
    if( plan.alt_expression_for_testing )
    {
        // Test that the truth table solver's version of the consistency expression
        // agrees with the original one
        unique_ptr<SYM::BooleanResult> alt_result = plan.alt_expression_for_testing->Evaluate( kit );
        ASSERT( alt_result );
        ASSERT( *alt_result == *result )(*alt_result)(" != ")(*result);
    }
    
    return result->IsDefinedAndTrue(); 
}


unique_ptr<SYM::SubsetResult> SymbolicConstraint::GetSuggestedValues( const Assignments &assignments,
                                                                   const VariableId &target_var ) const
{                                 
    ASSERT( target_var );
    SYM::Expression::EvalKit kit { &assignments, x_tree_db };    

    SYM::TruthTableSolver::GivenSymbolSet givens;
    for( VariableId v : plan.variables )            
        if( v != target_var && assignments.count(v) > 0 )
            givens.insert( v );

    if( plan.suggestion_expressions.count(target_var)==0 ||
        plan.suggestion_expressions.at(target_var).count(givens)==0 )
        return nullptr;
        
    shared_ptr<SYM::SymbolExpression> hint_expression = plan.suggestion_expressions.at(target_var).at(givens);
    unique_ptr<SYM::SymbolicResult> hr = hint_expression->Evaluate( kit );
    ASSERT( hr );
    unique_ptr<SYM::SubsetResult> hint_result = dynamic_pointer_cast<SYM::SubsetResult>(hr);
    ASSERT( hint_result );
    return hint_result;
}


string SymbolicConstraint::GetTrace() const
{
    return Constraint::GetTrace() + SSPrintf("[%d]", plan.variables.size());
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


