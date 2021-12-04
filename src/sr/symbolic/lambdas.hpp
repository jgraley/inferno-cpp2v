#ifndef LAMBDAS_HPP
#define LAMBDAS_HPP

#include "expression.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

#include <functional>

namespace SYM
{ 

class BooleanLambda : public BooleanExpression
{
public:
    typedef BooleanExpression EvalType;
    typedef function<void( const EvalKit &kit )> LambdaType;
    
    BooleanLambda( set<SR::PatternLink> input_plinks_,
                    const LambdaType &lambda_,
                    string description );
    virtual set<SR::PatternLink> GetInputPatternLinks() const override;
    virtual BooleanResult Evaluate( const EvalKit &kit ) const override;
    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const set<SR::PatternLink> input_plinks;
    const LambdaType lambda;
    const string description;
};

};

#endif // include guard
