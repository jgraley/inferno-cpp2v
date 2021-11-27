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
    
    LambdaOperator( const BooleanLambda &lambda_ );
    virtual void Evaluate( const EvalKit &kit ) override;
    
private:
    const BooleanLambda &lambda;
};

};

#endif // include guard
