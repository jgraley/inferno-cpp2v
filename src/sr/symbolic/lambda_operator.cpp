#include "lambda_operator.hpp"

using namespace SYM;

LambdaOperator::LambdaOperator( const BooleanLambda &lambda_ ) :
    lambda(lambda_)
{
}


void LambdaOperator::Evaluate( const EvalKit &kit )
{
    lambda(kit); // throws on mismatch
}



