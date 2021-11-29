#ifndef LAMBDA_OPERATOR_HPP
#define LAMBDA_OPERATOR_HPP

#include "operator.hpp"

#include "common/common.hpp"
#include "common/read_args.hpp"

#include <functional>

namespace SYM
{ 

class LambdaOperator : public BooleanOperator
{
public:
    typedef function<void( const EvalKit &kit )> BooleanLambda;
    
    LambdaOperator( set<SR::PatternLink> input_plinks_,
                    const BooleanLambda &lambda_ );
    virtual set<SR::PatternLink> GetInputPatternLinks() const override;
    virtual void Evaluate( const EvalKit &kit ) const override;
    
private:
    set<SR::PatternLink> input_plinks;
    const BooleanLambda lambda;
};

};

#endif // include guard
