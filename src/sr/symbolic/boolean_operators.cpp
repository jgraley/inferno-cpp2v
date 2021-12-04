#include "boolean_operators.hpp"

using namespace SYM;

// ------------------------- AndOperator --------------------------

AndOperator::AndOperator( set< shared_ptr<BooleanExpression> > sa_ ) :
    sa(sa_)
{
}    
    

set<SR::PatternLink> AndOperator::GetInputPatternLinks() const
{
    set<SR::PatternLink> sipl;
    // Non-strict union (i.e. not Solo) because common links are fine
    for( shared_ptr<BooleanExpression> a : sa )
        sipl = UnionOf( sipl, a->GetInputPatternLinks() );
    return sipl;
}


BooleanResult AndOperator::Evaluate( const EvalKit &kit ) const
{
    // This works with exceptions for mismatch but will prioritise
    // a's exception in the case where both mismatch.
    for( shared_ptr<BooleanExpression> a : sa )
    {
        BooleanResult r = a->Evaluate(kit);
        if( !r.matched )
            return r; // early out on first mismatch
    }
    return {true, nullptr};
}


string AndOperator::Render() const
{
    list<string> ls;
    for( shared_ptr<BooleanExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, "", " & ", "" );
}


Expression::Precedence AndOperator::GetPrecedence() const
{
    return AND;
}


Lazy<BooleanExpression> SYM::operator&( Lazy<BooleanExpression> a, Lazy<BooleanExpression> b )
{
    return MakeLazy<AndOperator>( set< shared_ptr<BooleanExpression> >({ a, b }) );
}
