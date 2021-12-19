#include "rewriters.hpp"

#include "boolean_operators.hpp"
#include "comparison_operators.hpp"

using namespace SYM;

BooleanExpressionList PreprocessForEngine::operator()( BooleanExpressionList in ) const
{
    BooleanExpressionList l1;
    for( auto bexpr : in )
    {
        SplitAnds( l1, bexpr );
    }

    BooleanExpressionList l2;
    for( auto bexpr : l1 )
    {
        if( auto bool_const_expr = dynamic_pointer_cast<BooleanConstant>((shared_ptr<BooleanExpression>)bexpr) )
        {
            unique_ptr<BooleanResult> r = bool_const_expr->GetValue();
            switch(r->matched)
            {
            case BooleanResult::UNKNOWN:
                ASSERT(false)("Got UNKNOWN from a BooleanConstant");
                break;
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
            l2.push_back( bexpr );
        }
    }
    
    return l2;
}


void PreprocessForEngine::SplitAnds( BooleanExpressionList &split, 
                                     shared_ptr<BooleanExpression> original ) const
{
    if( auto and_expr = dynamic_pointer_cast<AndOperator>(original) )
    {
        set<shared_ptr<Expression>> se = and_expr->GetOperands();
        for( shared_ptr<Expression> sub_expr : se )
        {
            auto bse = dynamic_pointer_cast<BooleanExpression>(sub_expr);
            ASSERT( bse );
            SplitAnds( split, bse );
        }
    }   
    else
    {
        split.push_back( original );
    }
}

// ------------------------- Solver --------------------------

Solver::Solver( shared_ptr<Equation> equation_ ) :
    equation( equation_ )
{
}


shared_ptr<SymbolExpression> Solver::TrySolveForSymbol( shared_ptr<SymbolVariable> target ) const
{
    if( auto equal_op = dynamic_pointer_cast<EqualsOperator>(equation) )
    {
        shared_ptr<SymbolExpression> other_op;
        for( shared_ptr<Expression> op : equal_op->GetOperands() )
        {
            bool is_curr = false;
            if( auto sv_op = dynamic_pointer_cast<SYM::SymbolVariable>(op) )
                if( OnlyElementOf( sv_op->GetRequiredVariables() ) ==
                    OnlyElementOf( target->GetRequiredVariables() ) )
                    is_curr = true;
            if( !is_curr )
                other_op = dynamic_pointer_cast<SYM::SymbolExpression>(op);                    
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
	set< shared_ptr<BooleanExpression> > s_disengaged, s_engaged;
    for( SR::PatternLink plink : original_expr->GetRequiredVariables() )
    {
        auto x = MakeLazy<SymbolVariable>(plink);
        s_disengaged.insert( x == disengager_expr );
        s_engaged.insert( x != disengager_expr );
    }

    auto all_disengaged_expr = MakeLazy<AndOperator>(s_disengaged);
    auto all_engaged_expr = MakeLazy<AndOperator>(s_engaged);

    return all_disengaged_expr | (all_engaged_expr & original_expr);
}    
    

shared_ptr<BooleanExpression> ClutchRewriter::ApplyDistributed(shared_ptr<BooleanExpression> original_expr) const
{
    if( auto and_expr = dynamic_pointer_cast<AndOperator>(original_expr) )
    {
        set<shared_ptr<Expression>> se = and_expr->GetOperands();
        set<shared_ptr<BooleanExpression>> new_clauses;
        for( shared_ptr<Expression> sub_expr : se )
        {
            shared_ptr<BooleanExpression> clause;
            clause = dynamic_pointer_cast<BooleanExpression>(sub_expr);            
            new_clauses.insert( ApplyDistributed( clause ) ); // recurse in case of nested AndOperator
        }
        return make_shared<AndOperator>( new_clauses );
    }   
    else
    {
        return ApplyUnified( original_expr );
    }
}
