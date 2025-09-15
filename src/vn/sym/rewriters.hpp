#ifndef REWRITES_HPP
#define REWRITES_HPP

#include "expression.hpp"

namespace SYM
{         

class SymbolVariable;

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

};

#endif
