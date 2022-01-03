#ifndef REWRITES_HPP
#define REWRITES_HPP

#include "expression.hpp"

namespace SR
{ 
    class Agent;
};

namespace SYM
{         

class SymbolVariable;

// Bake in the idea that an "equation" is really just a boolean expression
// which is required to evaluate to TRUE i.e. the "== TRUE` is assumed.
typedef BooleanExpression Equation;

typedef set< shared_ptr<BooleanExpression> > BooleanExpressionSet;

// ------------------------- PreprocessForEngine --------------------------

class PreprocessForEngine
{
public:
    BooleanExpressionSet operator()( BooleanExpressionSet in ) const;
    
private:
    void SplitAnds( BooleanExpressionSet &split, 
                    shared_ptr<BooleanExpression> original ) const;
};

// ------------------------- CreateTidiedOperator --------------------------

template<typename OP>
class CreateTidiedOperator
{
public:
    CreateTidiedOperator( bool identity_ );    
    shared_ptr<BooleanExpression> operator()( list< shared_ptr<BooleanExpression> > in ) const;
    
private:
    const bool identity;
};

// ------------------------- Solver --------------------------

class Solver
{
public:
    explicit Solver( shared_ptr<Equation> equation );
    shared_ptr<SymbolExpression> TrySolveForSymbol( shared_ptr<SymbolVariable> target ) const;
    static bool IsIndependentOf( shared_ptr<Expression> expr, shared_ptr<SymbolVariable> target );

private:
    const shared_ptr<Equation> equation;
};

// ------------------------- ClutchRewriter --------------------------

// The clutch idea works like this:
//  - CSP solvers will only find solutions when all constraints are 
//    satisfied. Constraints are made from symbolic expressions.
//  - There are symbolic expressions that we wish to make "optional"
//  - This is likened to disengaging a clutch in a car.
//  - Disengagement occurs when the required variables match the 
//    disengager expression, usually a constant (MMAX)
//  - Given disengager and original expressions, the clutched
//    version is just a combination of these and some new logic.
class ClutchRewriter
{
public:    
    explicit ClutchRewriter( shared_ptr<SymbolExpression> disengager );
    
    // Wrap the given expression in clutch logic, using GetRequiredVariables() to
    // determine which variables should be de-clutched by the disengager expression
    shared_ptr<BooleanExpression> ApplyUnified(shared_ptr<BooleanExpression> original_expr) const;

    // Apply the unified rewrite individually to each clause of a conjunctive expression
    shared_ptr<BooleanExpression> ApplyDistributed(shared_ptr<BooleanExpression> original_expr) const;
    
private:
    const shared_ptr<SymbolExpression> disengager_expr;
};

};

#endif
