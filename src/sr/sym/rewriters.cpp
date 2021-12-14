#include "rewriters.hpp"

#include "boolean_operators.hpp"
#include "comparison_operators.hpp"

using namespace SYM;

BooleanExpressionList PreprocessForEngine::operator()( BooleanExpressionList in ) const
{
    BooleanExpressionList l1;
    for( auto bexpr : in )
    {
        if( auto and_expr = dynamic_pointer_cast<AndOperator>((shared_ptr<BooleanExpression>)bexpr) )
        {
            set<shared_ptr<Expression>> se = and_expr->GetOperands();
            for( shared_ptr<Expression> sub_expr : se )
                l1.push_back( dynamic_pointer_cast<BooleanExpression>(sub_expr) );
        }   
        else
        {
            l1.push_back( bexpr );
        }
    }

    BooleanExpressionList l2;
    for( auto bexpr : l1 )
    {
        if( auto bool_const_expr = dynamic_pointer_cast<BooleanConstant>((shared_ptr<BooleanExpression>)bexpr) )
        {
            BooleanResult r = bool_const_expr->GetValue();
            switch(r.matched)
            {
            case BooleanResult::UNKNOWN:
                ASSERT(false)("Got UNKNOWN from a BooleanConstant");
                break;
            case BooleanResult::TRUE:
                break; // no action required
            case BooleanResult::FALSE:
                ASSERT(false)("Got a FALSE BooleanConstant clause in engine and-rule context");
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

// ------------------------- Solver --------------------------

Solver::Solver( shared_ptr<Equation> equation_ ) :
    equation( equation_ )
{
}


shared_ptr<SymbolExpression> Solver::TrySolveForSymbol( shared_ptr<SymbolVariable> target )
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
