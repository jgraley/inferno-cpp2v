#include "boolean_operators.hpp"
#include "rewriters.hpp"

using namespace SYM;

// ------------------------- AndOperator --------------------------

AndOperator::AndOperator( set< shared_ptr<BooleanExpression> > sa_ ) :
    sa( sa_ )
{
}    


set<shared_ptr<Expression>> AndOperator::GetOperands() const
{
    set<shared_ptr<Expression>> ops;
    for( shared_ptr<BooleanExpression> a : sa )
        ops.insert(a);
    return ops;
}


BooleanResult AndOperator::Evaluate( const EvalKit &kit ) const
{
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
    return Join( ls, " & " );
}


Expression::Precedence AndOperator::GetPrecedence() const
{
    return Precedence::AND;
}


Lazy<BooleanExpression> SYM::operator&( Lazy<BooleanExpression> a, Lazy<BooleanExpression> b )
{
    // Overloaded operator can only take 2 args, but operator is commutative and
    // associative: we want a o b o c to generate Operator({a, b, c}) not
    // some nested pair. Note: this can over-kill but I don't expect that to cause
    // problems.
    auto flattened_sa = SetFlattener<AndOperator>()({ a, b });
    return MakeLazy<AndOperator>( flattened_sa );
}

// ------------------------- OrOperator --------------------------

OrOperator::OrOperator( set< shared_ptr<BooleanExpression> > sa_ ) :
    sa( sa_ )
{   
}    


set<shared_ptr<Expression>> OrOperator::GetOperands() const
{
    set<shared_ptr<Expression>> ops;
    for( shared_ptr<BooleanExpression> a : sa )
        ops.insert(a);
    return ops;
}


BooleanResult OrOperator::Evaluate( const EvalKit &kit ) const
{
    for( shared_ptr<BooleanExpression> a : sa )
    {
        BooleanResult r = a->Evaluate(kit);
        if( r.matched )
            return r; // early out on first match
    }
    return {false, nullptr};
}


string OrOperator::Render() const
{
    list<string> ls;
    for( shared_ptr<BooleanExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, " | " );
}


Expression::Precedence OrOperator::GetPrecedence() const
{
    return Precedence::OR;
}


Lazy<BooleanExpression> SYM::operator|( Lazy<BooleanExpression> a, Lazy<BooleanExpression> b )
{
    // Overloaded operator can only take 2 args, but operator is commutative and
    // associative: we want a o b o c to generate Operator({a, b, c}) not
    // some nested pair. Note: this can over-kill but I don't expect that to cause
    // problems.
    auto flattened_sa = SetFlattener<OrOperator>()({ a, b });
    return MakeLazy<OrOperator>( flattened_sa );
}


