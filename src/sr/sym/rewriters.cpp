#include "rewriters.hpp"

#include "boolean_operators.hpp"

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


