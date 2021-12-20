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

typedef list< Lazy<BooleanExpression> > BooleanExpressionList;
typedef set< shared_ptr<BooleanExpression> > BooleanExpressionSet;


class PreprocessForEngine
{
public:
    BooleanExpressionList operator()( BooleanExpressionList in ) const;
    
private:
    void SplitAnds( BooleanExpressionList &split, 
                    shared_ptr<BooleanExpression> original ) const;
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
                list<shared_ptr<Expression>> se = and_expr->GetOperands();
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


template<typename OP>
class ListFlattener
{
public:
    list< shared_ptr<BooleanExpression> > operator()( list< shared_ptr<BooleanExpression> > in ) const
    {
        list< shared_ptr<BooleanExpression> > out;
        for( auto bexpr : in )
        {
            // Use non-solo inserts because we do want to de-duplciate if
            // the same expr appears more than once
            if( auto and_expr = dynamic_pointer_cast<OP>((shared_ptr<BooleanExpression>)bexpr) )
            {
                list<shared_ptr<Expression>> se = and_expr->GetOperands();
                for( shared_ptr<Expression> sub_expr : se )
                    out.push_back( dynamic_pointer_cast<BooleanExpression>(sub_expr) );
            }   
            else
            {
                out.push_back( bexpr );
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
    shared_ptr<SymbolExpression> TrySolveForSymbol( shared_ptr<SymbolVariable> target ) const;

private:
    shared_ptr<Equation> equation;
};

// ------------------------- ClutchRewriter --------------------------

class ClutchRewriter
{
public:    
    ClutchRewriter( shared_ptr<SymbolExpression> disengager );
    
    // Wrap the given expression in clutch logic, using GetRequiredVariables() to
    // determine which variables should be de-clutched by the disengager expression
    shared_ptr<BooleanExpression> ApplyUnified(shared_ptr<BooleanExpression> original_expr) const;

    // Apply the unified rewrite individually to each clause of a conjunctive expression
    shared_ptr<BooleanExpression> ApplyDistributed(shared_ptr<BooleanExpression> original_expr) const;
    
private:
    shared_ptr<SymbolExpression> disengager_expr;
};

};

#endif
