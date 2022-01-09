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


set<SR::PatternLink> BooleanLambda::GetRequiredVariables() const
{
    return required_plinks;
}


shared_ptr<BooleanResult> BooleanLambda::Evaluate( const EvalKit &kit ) const
{
    ASSERT( lambda );
    ASSERT( kit.hypothesis_links );
    ASSERT( kit.knowledge );    
    try
    {
        lambda(kit); // throws on mismatch
        return make_shared<BooleanResult>(BooleanResult::TRUE);
    }
    catch( const ::Mismatch &e )
    {
        return make_shared<BooleanResult>(BooleanResult::FALSE);
    }
}


Orderable::Result BooleanLambda::OrderCompareLocal( const Orderable *candidate, 
                                                    OrderProperty order_property ) const 
{
    ASSERT( candidate );
    auto *c = dynamic_cast<const BooleanLambda *>(candidate);    
    ASSERT(c);

    Orderable::Result r;
    switch( order_property )
    {
    case STRICT:
        // Unique order uses address to ensure different lambdas compare differently
        r = (int)(this > c) - (int)(this < c);
        // Note: just subtracting could overflow
        break;
    case REPEATABLE:
        // Repeatable ordering stops after name check since address compare is not repeatable
        r = Orderable::EQUAL;
        break;
    }
    return r;
}  


string BooleanLambda::Render() const
{
    return "[](){"+description+"}"; // Look like a lambda!
}


Expression::Precedence BooleanLambda::GetPrecedence() const
{
    return Precedence::LITERAL;
}
