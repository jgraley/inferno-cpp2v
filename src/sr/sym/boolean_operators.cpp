#include "boolean_operators.hpp"

using namespace SYM;

// ------------------------- AndOperator --------------------------

AndOperator::AndOperator( set< shared_ptr<BooleanExpression> > sa_ ) :
    sa(sa_)
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
    return MakeLazy<AndOperator>( set< shared_ptr<BooleanExpression> >({ a, b }) );
}

// ------------------------- OrOperator --------------------------

OrOperator::OrOperator( set< shared_ptr<BooleanExpression> > sa_ ) :
    sa(sa_)
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
    return MakeLazy<OrOperator>( set< shared_ptr<BooleanExpression> >({ a, b }) );
}


