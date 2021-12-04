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


BooleanResult BooleanLambda::Evaluate( const EvalKit &kit ) const
{
    ASSERT( lambda );
    ASSERT( kit.required_links );
    ASSERT( kit.knowledge );    
    try
    {
        lambda(kit); // throws on mismatch
        return {true, nullptr};
    }
    catch( const ::Mismatch &e )
    {
        return {false, current_exception()};
    }
}


string BooleanLambda::Render() const
{
    return "["+description+"]";
}


Expression::Precedence BooleanLambda::GetPrecedence() const
{
    return LITERAL;
}
