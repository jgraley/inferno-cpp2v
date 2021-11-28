#include "lambda_operator.hpp"

using namespace SYM;

LambdaOperator::LambdaOperator( set<SR::PatternLink> input_plinks_,
                                const BooleanLambda &lambda_ ) :
    input_plinks(input_plinks_),
    lambda(lambda_)
{
    ASSERT( lambda );
}


set<SR::PatternLink> LambdaOperator::GetInputPatternLinks() const
{
    return input_plinks;
}


void LambdaOperator::Evaluate( const EvalKit &kit ) const
{
    ASSERT( lambda );
    lambda(kit); // throws on mismatch
}



