#include "rewriters.hpp"

#include "boolean_operators.hpp"

using namespace SYM;

BooleanExpressionList Splitter::operator()( BooleanExpressionList bel_in ) const
{
    BooleanExpressionList bel_out;
    for( auto p : bel_in )
    {
        if( auto pand = dynamic_pointer_cast<SYM::AndOperator>((shared_ptr<SYM::BooleanExpression>)p.first) )
        {
            set<shared_ptr<SYM::Expression>> se = pand->GetOperands();
            for( shared_ptr<SYM::Expression> e : se )
                bel_out.push_back( make_pair(dynamic_pointer_cast<SYM::BooleanExpression>(e), p.second) );
        }   
        else
        {
            bel_out.push_back( p );
        }
    }
    return bel_out;
}