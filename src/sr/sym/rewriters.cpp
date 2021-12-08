#include "rewriters.hpp"

#include "boolean_operators.hpp"

using namespace SYM;

BooleanExpressionList Splitter::operator()( BooleanExpressionList bel_in ) const
{
    BooleanExpressionList bel_out;
    for( auto bexpr : bel_in )
    {
        if( auto pand = dynamic_pointer_cast<SYM::AndOperator>((shared_ptr<SYM::BooleanExpression>)bexpr) )
        {
            set<shared_ptr<SYM::Expression>> se = pand->GetOperands();
            for( shared_ptr<SYM::Expression> e : se )
                bel_out.push_back( dynamic_pointer_cast<SYM::BooleanExpression>(e) );
        }   
        else
        {
            bel_out.push_back( bexpr );
        }
    }
    return bel_out;
}