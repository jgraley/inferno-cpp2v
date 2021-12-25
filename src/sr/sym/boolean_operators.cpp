#include "boolean_operators.hpp"
#include "rewriters.hpp"

using namespace SYM;

// ------------------------- AndOperator --------------------------

AndOperator::AndOperator( list< shared_ptr<BooleanExpression> > sa_ ) :
    sa( sa_ )
{
}    


list<shared_ptr<BooleanExpression>> AndOperator::GetBooleanOperands() const
{
    return sa;
}


unique_ptr<BooleanResult> AndOperator::Evaluate( const EvalKit &kit,
                                                 const list<unique_ptr<BooleanResult>> &op_results ) const
{
    BooleanResult::BooleanValue m = BooleanResult::TRUE;
    for( const unique_ptr<BooleanResult> &r : op_results )
    {
        switch( r->value )
        {
        case BooleanResult::UNKNOWN:
            m = BooleanResult::UNKNOWN;
            break;            
        case BooleanResult::TRUE:
            break;
        case BooleanResult::FALSE:
            return make_unique<BooleanResult>( BooleanResult::FALSE );
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
    list< shared_ptr<BooleanExpression> > flattened_sa = ListFlattener<AndOperator>()({ a, b });
    return MakeLazy<AndOperator>( flattened_sa );
}

// ------------------------- OrOperator --------------------------

OrOperator::OrOperator( list< shared_ptr<BooleanExpression> > sa_ ) :
    sa( sa_ )
{   
}    


list<shared_ptr<BooleanExpression>> OrOperator::GetBooleanOperands() const
{
    return sa;
}


unique_ptr<BooleanResult> OrOperator::Evaluate( const EvalKit &kit,
                                                const list<unique_ptr<BooleanResult>> &op_results ) const
{
    BooleanResult::BooleanValue m = BooleanResult::FALSE;
    for( const unique_ptr<BooleanResult> &r : op_results )
    {
        switch( r->value )
        {
        case BooleanResult::UNKNOWN:
            m = BooleanResult::UNKNOWN;
            break;            
        case BooleanResult::TRUE:
            return make_unique<BooleanResult>( BooleanResult::TRUE );
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
    auto flattened_sa = ListFlattener<OrOperator>()({ a, b });
    return MakeLazy<OrOperator>( flattened_sa );
}


