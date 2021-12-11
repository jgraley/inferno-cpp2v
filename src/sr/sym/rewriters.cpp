#include "rewriters.hpp"

#include "boolean_operators.hpp"
#include "comparison_operators.hpp"

using namespace SYM;

BooleanExpressionList ListSplitter::operator()( BooleanExpressionList in ) const
{
    BooleanExpressionList out;
    for( auto bexpr : in )
    {
        if( auto pand = dynamic_pointer_cast<AndOperator>((shared_ptr<BooleanExpression>)bexpr) )
        {
            set<shared_ptr<Expression>> se = pand->GetOperands();
            for( shared_ptr<Expression> e : se )
                out.push_back( dynamic_pointer_cast<BooleanExpression>(e) );
        }   
        else
        {
            out.push_back( bexpr );
        }
    }
    return out;
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
        shared_ptr<SymbolExpression> other;
        for( shared_ptr<Expression> op : equal_op->GetOperands() )
        {
            bool is_curr = false;
            if( auto sv_op = dynamic_pointer_cast<SYM::SymbolVariable>(op) )
                if( OnlyElementOf( sv_op->GetRequiredPatternLinks() ) ==
                    OnlyElementOf( target->GetRequiredPatternLinks() ) )
                    is_curr = true;
            if( !is_curr )
                other = dynamic_pointer_cast<SYM::SymbolExpression>(op);                    
        }
        ASSERT( other )
              ("didn't find any other operands or not a symbol expression, target=")(target)
              ("equation:\n")(equation);
        return other;
    }

    return nullptr;
}
