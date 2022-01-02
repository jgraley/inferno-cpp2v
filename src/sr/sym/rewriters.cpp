#include "rewriters.hpp"

#include "boolean_operators.hpp"
#include "comparison_operators.hpp"

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

// ------------------------- CreateTimeTidier --------------------------

template<typename OP>
CreateTimeTidier<OP>::CreateTimeTidier( bool identity_ ) :
    identity(identity_)
{
}


template<typename OP>
shared_ptr<BooleanExpression> CreateTimeTidier<OP>::operator()( list< shared_ptr<BooleanExpression> > in ) const
{
    list< shared_ptr<BooleanExpression> > out;
    for( auto bexpr : in )
    {
        if( auto op_expr = dynamic_pointer_cast<OP>((shared_ptr<BooleanExpression>)bexpr) )
        {
            for( shared_ptr<Expression> sub_expr : op_expr->GetOperands() )
                out.push_back( dynamic_pointer_cast<BooleanExpression>(sub_expr) );
        }   
        else if( auto bconst_expr = dynamic_pointer_cast<BooleanConstant>((shared_ptr<BooleanExpression>)bexpr) )
        {
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
    
    if( out.empty() )
        return MakeLazy<BooleanConstant>(identity);
    else if( out.size()==1 )
        return OnlyElementOf(out);
    else
        return make_shared<OP>( out );
}

// Explicit template instantiations
template class CreateTimeTidier<AndOperator>;
template class CreateTimeTidier<OrOperator>;

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
    for( SR::PatternLink plink : original_expr->GetRequiredVariables() )
    {
        auto x = MakeLazy<SymbolVariable>(plink);
        all_disengaged_expr &= ( x == disengager_expr );
        all_engaged_expr &= ( x != disengager_expr );
    }

    return (original_expr & all_engaged_expr) | all_disengaged_expr;
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
