#include "boolean_operators.hpp"

using namespace SYM;

// ------------------------- BooleanLambda --------------------------

BooleanLambda::BooleanLambda( set<SR::PatternLink> input_plinks_,
                                const LambdaType &lambda_ ) :
    input_plinks(input_plinks_),
    lambda(lambda_)
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

// ------------------------- AndOperator --------------------------

AndOperator::AndOperator( shared_ptr<BooleanOperator> a_,
                          shared_ptr<BooleanOperator> b_ ) :
    a(a_),
    b(b_)
{
}    
    

set<SR::PatternLink> AndOperator::GetInputPatternLinks() const
{
    // Non-strict union (i.e. not Solo) because comment links are fine
    return UnionOf( a->GetInputPatternLinks(), 
                    b->GetInputPatternLinks() );
}


void AndOperator::Evaluate( const EvalKit &kit ) const
{
    // This works with exceptions for mismatch but will prioritise
    // a's exception in the case where both mismatch.
    a->Evaluate(kit);
    b->Evaluate(kit);
}
