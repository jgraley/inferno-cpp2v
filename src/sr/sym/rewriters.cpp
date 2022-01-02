#include "rewriters.hpp"

#include "boolean_operators.hpp"
#include "comparison_operators.hpp"
#include "primary_expressions.hpp"

using namespace SYM;

BooleanExpressionSet PreprocessForEngine::operator()( BooleanExpressionSet in ) const
{
    BooleanExpressionSet l1;
    for( auto bexpr : in )
    {
        SplitAnds( l1, bexpr );
    }

    BooleanExpressionSet l2;
    for( auto bexpr : l1 )
    {
        if( auto bool_const_expr = dynamic_pointer_cast<BooleanConstant>((shared_ptr<BooleanExpression>)bexpr) )
        {
            shared_ptr<BooleanResult> r = bool_const_expr->GetValue();
            switch(r->value)
            {
            case BooleanResult::TRUE:
                break; // no action required
            case BooleanResult::FALSE:
                ASSERT(false)("Got a FALSE BooleanConstant clause in engine and-rule preprocessing");
                // Of course, there IS a correct thing to do - replace the whole list with a single constant FALSE
                return { MakeLazy<BooleanConstant>(false) }; // ...like this
            }
        }
        else
        {
            l2.insert( bexpr );
        }
    }
    
    return l2;
}


void PreprocessForEngine::SplitAnds( BooleanExpressionSet &split, 
                                     shared_ptr<BooleanExpression> original ) const
{
    if( auto and_expr = dynamic_pointer_cast<AndOperator>(original) )
    {
        for( shared_ptr<BooleanExpression> sub_expr : and_expr->GetBooleanOperands() )
            SplitAnds( split, sub_expr );
    }   
    else
    {
        split.insert( original );
    }
}

// ------------------------- CreateTidiedOperator --------------------------

template<typename OP>
CreateTidiedOperator<OP>::CreateTidiedOperator( bool identity_ ) :
    identity(identity_)
{
}


template<typename OP>
shared_ptr<BooleanExpression> CreateTidiedOperator<OP>::operator()( list< shared_ptr<BooleanExpression> > in ) const
{
    list< shared_ptr<BooleanExpression> > out;
    for( auto bexpr : in )
    {
        // Handle finding our own opeator as an operand - expand one level (non-recursive)
        if( auto op_expr = dynamic_pointer_cast<OP>((shared_ptr<BooleanExpression>)bexpr) )
        {
            for( shared_ptr<Expression> sub_expr : op_expr->GetOperands() )
                out.push_back( dynamic_pointer_cast<BooleanExpression>(sub_expr) );
        }   
        else if( auto bconst_expr = dynamic_pointer_cast<BooleanConstant>((shared_ptr<BooleanExpression>)bexpr) )
        {
            // Handle finding constant booleans relevent to the operator
            if( bconst_expr->GetValue()->value == (identity ? BooleanResult::TRUE : BooleanResult::FALSE) )                
            {
                // drop the clause - has no effect
            }
            else
            {
                // dominates
                out = { bconst_expr };
                break;
            }                    
        }
        else
        {
            out.push_back( bexpr );
        }
    }
    
    // By cases: empty get identity, singlur gets returned, otherwise actually generate the operator
    if( out.empty() )
        return MakeLazy<BooleanConstant>(identity);
    else if( out.size()==1 )
        return OnlyElementOf(out);
    else
        return make_shared<OP>( out );
}

// Explicit template instantiations
template class CreateTidiedOperator<AndOperator>;
template class CreateTidiedOperator<OrOperator>;

// ------------------------- Solver --------------------------

Solver::Solver( shared_ptr<Equation> equation_ ) :
    equation( equation_ )
{
}


shared_ptr<SymbolExpression> Solver::TrySolveForSymbol( shared_ptr<SymbolVariable> target ) const
{
    if( auto equal_op = dynamic_pointer_cast<EqualOperator>(equation) )
    {
        shared_ptr<SymbolExpression> other_op;
        for( shared_ptr<SymbolExpression> op : equal_op->GetSymbolOperands() )
        {
            bool is_curr = false;
            if( auto sv_op = dynamic_pointer_cast<SYM::SymbolVariable>(op) )
                if( OnlyElementOf( sv_op->GetRequiredVariables() ) ==
                    OnlyElementOf( target->GetRequiredVariables() ) )
                    is_curr = true;
            if( !is_curr )
                other_op = op;                    
        }
        ASSERT( other_op )
              ("didn't find any other operands or not a symbol expression, target=")(target)
              ("equation:\n")(equation);
        return other_op;
    }
#if 0    
    else if( auto and_op = dynamic_pointer_cast<AndOperator>(equation) )
    {
        shared_ptr<ImplicationOperator> implies;
        shared_ptr<ImplicationOperator> implies;
        for( shared_ptr<SymbolExpression> op : equal_op->GetSymbolOperands() )
        {
            bool is_curr = false;
            if( auto sv_op = dynamic_pointer_cast<SYM::SymbolVariable>(op) )
                if( OnlyElementOf( sv_op->GetRequiredVariables() ) ==
                    OnlyElementOf( target->GetRequiredVariables() ) )
                    is_curr = true;
            if( !is_curr )
                other_op = op;                    
        }
    }
#endif
    return nullptr;
}

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
#if 1
    if( !first_engage ) // TODO should be disallowed
        return original_expr;  
        
    auto original_when_engaged = MakeLazy<ImplicationOperator>( first_engage, original_expr );            
    if( all_disengaged_list.size()==1 )
        return original_when_engaged;

    auto all_disengage_equal = MakeLazy<BoolEqualOperator>( all_disengaged_list ); 
    return all_disengage_equal & original_when_engaged;
#else
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
