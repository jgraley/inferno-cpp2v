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

// ------------------------- SymSolver --------------------------

class SymSolver
{
public:
    explicit SymSolver( shared_ptr<SymbolVariable> target );
    shared_ptr<SymbolExpression> TrySolve( shared_ptr<Equation> equation ) const;
    bool IsIndependent( shared_ptr<Expression> expr ) const;

private:
    const shared_ptr<SymbolVariable> target;
};

};

#endif
