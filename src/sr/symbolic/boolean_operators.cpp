#include "boolean_operators.hpp"

using namespace SYM;

// ------------------------- AndOperator --------------------------

AndOperator::AndOperator( shared_ptr<BooleanExpression> a_,
                          shared_ptr<BooleanExpression> b_ ) :
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


string AndOperator::Render() const
{
    return a->Render() + " && " + b->Render();
};
