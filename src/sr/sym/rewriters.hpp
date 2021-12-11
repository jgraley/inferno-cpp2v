#ifndef REWRITES_HPP
#define REWRITES_HPP

#include "lazy.hpp"
#include "primary_expressions.hpp"

namespace SR
{ 
    class Agent;
};

namespace SYM
{         
// Bake in the idea that an "equation" is really just a boolean expression
// which is required to evaluate to TRUE i.e. the "== TRUE` is assumed.
typedef BooleanExpression Equation;

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
            if( auto and_expr = dynamic_pointer_cast<OP>((shared_ptr<BooleanExpression>)bexpr) )
            {
                set<shared_ptr<Expression>> se = and_expr->GetOperands();
                for( shared_ptr<Expression> sub_expr : se )
                    out.insert( dynamic_pointer_cast<BooleanExpression>(sub_expr) );
            }   
            else
            {
                out.insert( bexpr );
            }
        }
        return out;
    }
};

// ------------------------- Solver --------------------------

class Solver
{
public:
    Solver( shared_ptr<Equation> equation );
    shared_ptr<SymbolExpression> TrySolveForSymbol( shared_ptr<SymbolVariable> target );

private:
    shared_ptr<Equation> equation;
};


};

#endif
