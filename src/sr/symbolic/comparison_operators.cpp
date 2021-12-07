#include "comparison_operators.hpp"
#include "../agents/agent.hpp"

using namespace SYM;

// ------------------------- EqualsOperator --------------------------

EqualsOperator::EqualsOperator( set< shared_ptr<SymbolExpression> > sa_ ) :
    sa(sa_)
{
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
    list<SymbolResult> results;
    for( shared_ptr<SymbolExpression> a : sa )
        results.push_back( a->Evaluate(kit) );
    bool equal = true;
    ForOverlappingAdjacentPairs( results, [&](const SymbolResult &ra,
                                              const SymbolResult &rb) 
    {
        // For equality, it is sufficient to compare the x links
        // themselves, which have the required uniqueness properties
        // within the full arrowhead model.
        equal = equal && ( ra.xlink == rb.xlink );
    });
    return {equal, nullptr};   
}


string EqualsOperator::Render() const
{
    list<string> ls;
    for( shared_ptr<SymbolExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, "==" );
}


Expression::Precedence EqualsOperator::GetPrecedence() const
{
    return AND;
}


Lazy<BooleanExpression> SYM::operator==( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<EqualsOperator>( set< shared_ptr<SymbolExpression> >({ a, b }) );
}

// ------------------------- PreRestrictionOperator --------------------------

PreRestrictionOperator::PreRestrictionOperator( shared_ptr<SymbolExpression> a_,
                                                const SR::Agent *pre_restrictor_ ) :
    a( a_ ),
    pre_restrictor( pre_restrictor_ )
{    
}                                                

set<shared_ptr<Expression>> PreRestrictionOperator::GetOperands() const
{
    return { a };
}


BooleanResult PreRestrictionOperator::Evaluate( const EvalKit &kit ) const 
{
    SymbolResult ar = a->Evaluate( kit );
    bool matches = pre_restrictor->IsPreRestrictionMatch(ar.xlink);
    return { matches, nullptr };
}


string PreRestrictionOperator::Render() const
{
    // Not using RenderForMe() because we always want () here
    return "Pre(" + a->Render() + ")"; 
}


Expression::Precedence PreRestrictionOperator::GetPrecedence() const
{
    return PREFIX;
}
