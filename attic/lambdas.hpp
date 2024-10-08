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
    typedef BooleanExpression NominalType;
    typedef function<void( const EvalKit &kit )> LambdaType;
    
    explicit BooleanLambda( set<SR::PatternLink> required_plinks_,
                             const LambdaType &lambda_,
                             string description );
    virtual set<SR::PatternLink> GetRequiredVariables() const override;
    virtual shared_ptr<BooleanResultInterface> Evaluate( const EvalKit &kit ) const override;

    virtual Orderable::Result OrderCompareLocal( const Orderable *candidate, 
                                                 OrderProperty order_property ) const override;

    virtual string Render() const override;
    virtual Precedence GetPrecedence() const override;
    
private:
    const set<SR::PatternLink> required_plinks;
    const LambdaType lambda;
    const string description;
};

};

#endif // include guard
