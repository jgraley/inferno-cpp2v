#include "comparison_operators.hpp"

using namespace SYM;

// ------------------------- EqualsOperator --------------------------

EqualsOperator::EqualsOperator( set< shared_ptr<SymbolExpression> > sa_ ) :
    sa(sa_)
{
}    
    

set<SR::PatternLink> EqualsOperator::GetRequiredPatternLinks() const // TODO should be in base class, making use of GetOperands()
{
    set<SR::PatternLink> sipl;
    // Non-strict union (i.e. not Solo) because common links are fine
    for( shared_ptr<SymbolExpression> a : sa )
        sipl = UnionOf( sipl, a->GetRequiredPatternLinks() );
    return sipl;
}


set<shared_ptr<Expression>> EqualsOperator::GetOperands() const
{
    set<shared_ptr<Expression>> ops;
    for( shared_ptr<SymbolExpression> a : sa )
        ops.insert(a);
    return ops;
}


BooleanResult EqualsOperator::Evaluate( const EvalKit &kit ) const
{
    // This works with exceptions for mismatch but will prioritise
    // a's exception in the case where both mismatch.
    SymbolResult prev_result;
    bool first = true;
    for( shared_ptr<SymbolExpression> a : sa )
    {
        SymbolResult result = a->Evaluate(kit);
        if( !first )
        {
            // For equality, it is sufficient to compare the x links
            // themselves, which have the required uniqueness properties
            // within the full arrowhead model.
            if( result.xlink != prev_result.xlink )
                return {false, nullptr}; // early out on mismatch
        }
        prev_result = result;
        first = false;
    }
    return {true, nullptr};
}


string EqualsOperator::Render() const
{
    list<string> ls;
    for( shared_ptr<SymbolExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, "", "==", "" );
}


Expression::Precedence EqualsOperator::GetPrecedence() const
{
    return AND;
}


Lazy<BooleanExpression> SYM::operator==( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<EqualsOperator>( set< shared_ptr<SymbolExpression> >({ a, b }) );
}
