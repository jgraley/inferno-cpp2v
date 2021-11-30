#ifndef BOOLEAN_OPERATORS_HPP
#define BOOLEAN_OPERATORS_HPP

#include "expression.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

namespace SYM
{ 

class AndOperator : public BooleanExpression
{
public:    
    AndOperator( shared_ptr<BooleanExpression> a,
                 shared_ptr<BooleanExpression> b );
    virtual set<SR::PatternLink> GetInputPatternLinks() const override;
    virtual void Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    
private:
    shared_ptr<BooleanExpression> a;
    shared_ptr<BooleanExpression> b;
};

};

#endif // include guard
