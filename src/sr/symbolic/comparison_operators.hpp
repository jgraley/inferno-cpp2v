#ifndef BOOLEAN_OPERATORS_HPP
#define BOOLEAN_OPERATORS_HPP

#include "expression.hpp"
#include "lazy.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SYM
{ 

class EqualsOperator : public BooleanExpression
{
public:    
    class Mismatch : public ::Mismatch {}; // only one kind of mismatch here
    typedef BooleanExpression EvalType;
    EqualsOperator( set< shared_ptr<SymbolExpression> > sa );
    virtual set<SR::PatternLink> GetRequiredPatternLinks() const override;
    virtual set<shared_ptr<Expression>> GetOperands() const;
    virtual BooleanResult Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    set< shared_ptr<SymbolExpression> > sa;
};

Lazy<BooleanExpression> operator==( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b );

};

#endif // include guard
