#ifndef REWRITES_HPP
#define REWRITES_HPP

#include "lazy.hpp"

namespace SR
{ 
    class Agent;
};

namespace SYM
{         

typedef list< SYM::Lazy<SYM::BooleanExpression> > BooleanExpressionList;
typedef set< shared_ptr<SYM::BooleanExpression> > BooleanExpressionSet;


class ListSplitter
{
public:
    BooleanExpressionList operator()( BooleanExpressionList in ) const;
};


template<typename OP>
class SetFlattener
{
public:
    BooleanExpressionSet operator()( BooleanExpressionSet in ) const
    {
        BooleanExpressionSet out;
        for( auto bexpr : in )
        {
            // Use non-solo inserts because we do want to de-duplciate if
            // the same expr appears more than once
            if( auto pand = dynamic_pointer_cast<OP>((shared_ptr<BooleanExpression>)bexpr) )
            {
                set<shared_ptr<Expression>> se = pand->GetOperands();
                for( shared_ptr<Expression> e : se )
                    out.insert( dynamic_pointer_cast<BooleanExpression>(e) );
            }   
            else
            {
                out.insert( bexpr );
            }
        }
        return out;
    }
};

};

#endif
