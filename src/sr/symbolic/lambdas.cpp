#include "lambdas.hpp"

using namespace SYM;

BooleanLambda::BooleanLambda( set<SR::PatternLink> required_plinks_,
                              const LambdaType &lambda_,
                              string description_ ) :
    required_plinks(required_plinks_),
    lambda(lambda_),
    description(description_)
{
    ASSERT( lambda );
}


set<SR::PatternLink> BooleanLambda::GetRequiredPatternLinks() const
{
    return required_plinks;
}


BooleanResult BooleanLambda::Evaluate( const EvalKit &kit ) const
{
    ASSERT( lambda );
    ASSERT( kit.hypothesis_links );
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
    return "[](){"+description+"}"; // Look like a lambda!
}


Expression::Precedence BooleanLambda::GetPrecedence() const
{
    return LITERAL;
}
