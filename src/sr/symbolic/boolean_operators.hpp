#ifndef BOOLEAN_OPERATORS_HPP
#define BOOLEAN_OPERATORS_HPP

#include "expression.hpp"
#include "lazy.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SYM
{ 

class AndOperator : public BooleanExpression
{
public:    
    typedef BooleanExpression EvalType;
    AndOperator( set< shared_ptr<BooleanExpression> > sa );
    virtual set<SR::PatternLink> GetInputPatternLinks() const override;
    virtual BooleanResult Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    set< shared_ptr<BooleanExpression> > sa;
};

Lazy<BooleanExpression> operator&( Lazy<BooleanExpression> a, Lazy<BooleanExpression> b );

};

#endif // include guard
