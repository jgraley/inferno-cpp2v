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


unique_ptr<BooleanResult> AndOperator::Evaluate( const EvalKit &kit ) const
{
    BooleanResult::Matched m = BooleanResult::TRUE;
    for( shared_ptr<BooleanExpression> a : sa )
    {
        unique_ptr<BooleanResult> r = a->Evaluate(kit);
        switch( r->matched )
        {
        case BooleanResult::UNKNOWN:
            m = BooleanResult::UNKNOWN;
            break;            
        case BooleanResult::TRUE:
            break;
        case BooleanResult::FALSE:
            return r; // early out
        }
    }
    return make_unique<BooleanResult>( m );
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


unique_ptr<BooleanResult> OrOperator::Evaluate( const EvalKit &kit ) const
{
    BooleanResult::Matched m = BooleanResult::FALSE;
    for( shared_ptr<BooleanExpression> a : sa )
    {
        unique_ptr<BooleanResult> r = a->Evaluate(kit);
        switch( r->matched )
        {
        case BooleanResult::UNKNOWN:
            m = BooleanResult::UNKNOWN;
            break;            
        case BooleanResult::TRUE:
            return r; // early out
        case BooleanResult::FALSE:
            break;
        }
    }
    return make_unique<BooleanResult>( m );
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


