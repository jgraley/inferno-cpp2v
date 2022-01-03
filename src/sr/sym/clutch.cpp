#include "clutch.hpp"

#include "boolean_operators.hpp"
#include "comparison_operators.hpp"
#include "symbol_operators.hpp"
#include "primary_expressions.hpp"

#define BOOL_EQUALITY_METHOD

using namespace SYM;

// ------------------------- ClutchRewriter --------------------------

ClutchRewriter::ClutchRewriter( shared_ptr<SymbolExpression> disengager_expr_ ) : 
    disengager_expr( disengager_expr_ )
{
}


shared_ptr<BooleanExpression> ClutchRewriter::ApplyUnified(shared_ptr<BooleanExpression> original_expr) const
{
    // Implement strict clutch policy: ALL or NONE of the required vars 
    // must be disengager. If some but not all are disengager we don't get a match.
    auto all_disengaged_expr = MakeLazy<BooleanConstant>(true);
    auto all_engaged_expr = MakeLazy<BooleanConstant>(true);
    list< shared_ptr<BooleanExpression> > all_disengaged_list;
    shared_ptr<BooleanExpression> first_engage;
    for( SR::PatternLink plink : original_expr->GetRequiredVariables() )
    {
        auto x = MakeLazy<SymbolVariable>(plink);
        all_disengaged_expr &= ( x == disengager_expr );
        all_engaged_expr &= ( x != disengager_expr );
        all_disengaged_list.push_back( x == disengager_expr );
        if( !first_engage )
            first_engage = ( x != disengager_expr );
    }
#ifdef BOOL_EQUALITY_METHOD
    if( !first_engage ) // TODO should be disallowed
        return original_expr;  
        
    auto original_when_engaged = MakeLazy<ImplicationOperator>( first_engage, original_expr );            
    if( all_disengaged_list.size()==1 )
        return original_when_engaged;

    auto all_disengage_equal = MakeLazy<BoolEqualOperator>( all_disengaged_list ); 
    return all_disengage_equal & original_when_engaged;
#else // and/or method
    return (original_expr & all_engaged_expr) | all_disengaged_expr;
#endif
}    
    

shared_ptr<BooleanExpression> ClutchRewriter::ApplyDistributed(shared_ptr<BooleanExpression> original_expr) const
{
    if( auto and_expr = dynamic_pointer_cast<AndOperator>(original_expr) )
    {
        list<shared_ptr<BooleanExpression>> new_clauses;
        for( shared_ptr<BooleanExpression> clause : and_expr->GetBooleanOperands() )                
            new_clauses.push_back( ApplyDistributed( clause ) ); // recurse in case of nested AndOperator        
        return make_shared<AndOperator>( new_clauses );
    }   
    else
    {
        return ApplyUnified( original_expr );
    }
}
