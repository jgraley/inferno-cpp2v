#include "lambdas.hpp"

using namespace SYM;

BooleanLambda::BooleanLambda( set<SR::PatternLink> input_plinks_,
                              const LambdaType &lambda_,
                              string description_ ) :
    input_plinks(input_plinks_),
    lambda(lambda_),
    description(description_)
{
    ASSERT( lambda );
}


set<SR::PatternLink> BooleanLambda::GetInputPatternLinks() const
{
    return input_plinks;
}


void BooleanLambda::Evaluate( const EvalKit &kit ) const
{
    ASSERT( lambda );
    ASSERT( kit.required_links );
    ASSERT( kit.knowledge );    
    lambda(kit); // throws on mismatch
}


string BooleanLambda::Render() const
{
    return "["+description+"]";
};
