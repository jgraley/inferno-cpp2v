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


shared_ptr<BooleanResult> AndOperator::Evaluate( const EvalKit &kit,
                                                 const list<shared_ptr<BooleanResult>> &op_results ) const
{
    BooleanResult::BooleanValue m = BooleanResult::TRUE;
    for( const shared_ptr<BooleanResult> &r : op_results )
    {
        switch( r->value )
        {            
        case BooleanResult::TRUE:
            break;
        case BooleanResult::FALSE:
            return make_shared<BooleanResult>( BooleanResult::FALSE );
        }
    }
    return make_shared<BooleanResult>( m );
}


string AndOperator::Render() const
{
    if( sa.empty() )
        return "TRUE(empty-And)";
    list<string> ls;
    for( shared_ptr<BooleanExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, " & " );
}


Expression::Precedence AndOperator::GetPrecedence() const
{
    return Precedence::AND;
}


Over<BooleanExpression> SYM::operator&( Over<BooleanExpression> a, Over<BooleanExpression> b )
{
    // Overloaded operator can only take 2 args, but operator is commutative and
    // associative: we want a o b o c to generate Operator({a, b, c}) not
    // some nested pair. Note: this can over-kill but I don't expect that to cause
    // problems.
    return CreateTidiedOperator<AndOperator>(true)({ a, b });
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


shared_ptr<BooleanResult> OrOperator::Evaluate( const EvalKit &kit,
                                                const list<shared_ptr<BooleanResult>> &op_results ) const
{
    BooleanResult::BooleanValue m = BooleanResult::FALSE;
    for( const shared_ptr<BooleanResult> &r : op_results )
    {
        switch( r->value )
        {   
        case BooleanResult::TRUE:
            return make_shared<BooleanResult>( BooleanResult::TRUE );
        case BooleanResult::FALSE:
            break;
        }
    }
    return make_shared<BooleanResult>( m );
}


string OrOperator::Render() const
{
    if( sa.empty() )
        return "FALSE(empty-Or)";
    list<string> ls;
    for( shared_ptr<BooleanExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, " | " );
}


Expression::Precedence OrOperator::GetPrecedence() const
{
    return Precedence::OR;
}


Over<BooleanExpression> SYM::operator|( Over<BooleanExpression> a, Over<BooleanExpression> b )
{
    // Overloaded operator can only take 2 args, but operator is commutative and
    // associative: we want a o b o c to generate Operator({a, b, c}) not
    // some nested pair. Note: this can over-kill but I don't expect that to cause
    // problems.
    return CreateTidiedOperator<OrOperator>(false)({ a, b });
}


// ------------------------- BoolEqualOperator --------------------------

BoolEqualOperator::BoolEqualOperator( list< shared_ptr<BooleanExpression> > sa_ ) :
    sa( sa_ )
{   
    ASSERT( sa.size() >= 2 ); 
}    


list<shared_ptr<BooleanExpression>> BoolEqualOperator::GetBooleanOperands() const
{
    return sa;
}


shared_ptr<BooleanResult> BoolEqualOperator::Evaluate( const EvalKit &kit,
                                                       const list<shared_ptr<BooleanResult>> &op_results ) const
{
    BooleanResult::BooleanValue m = BooleanResult::TRUE;
    ForOverlappingAdjacentPairs( op_results, [&](shared_ptr<BooleanResult> ra,
                                                 shared_ptr<BooleanResult> rb) 
    {
        if( ra->value != rb->value )
            m = BooleanResult::FALSE;
    } );
    return make_shared<BooleanResult>( m );
}


string BoolEqualOperator::Render() const
{
    list<string> ls;
    for( shared_ptr<BooleanExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, " == " );
}


Expression::Precedence BoolEqualOperator::GetPrecedence() const
{
    return Precedence::COMPARE;
}


Over<BooleanExpression> SYM::operator==( Over<BooleanExpression> a, Over<BooleanExpression> b )
{
    // Overloaded operator can only take 2 args, but operator is commutative and
    // associative: we want a o b o c to generate Operator({a, b, c}) not
    // some nested pair. Note: this can over-kill but I don't expect that to cause
    // problems.
    return MakeOver<BoolEqualOperator>( list<shared_ptr<BooleanExpression>>({ a, b }) );
}

// ------------------------- ImplicationOperator --------------------------

ImplicationOperator::ImplicationOperator( shared_ptr<BooleanExpression> a_,
                                          shared_ptr<BooleanExpression> b_ ) :
    a( a_ ),
    b( b_ )
{   
}    


list<shared_ptr<BooleanExpression>> ImplicationOperator::GetBooleanOperands() const
{
    return { a, b };
}


shared_ptr<BooleanResult> ImplicationOperator::Evaluate( const EvalKit &kit,
                                                         const list<shared_ptr<BooleanResult>> &op_results ) const
{
    shared_ptr<BooleanResult> ar = op_results.front();
    shared_ptr<BooleanResult> br = op_results.back();
    if( ar->value == BooleanResult::TRUE )
        return br;
    else 
        return make_shared<BooleanResult>( BooleanResult::TRUE );
}


string ImplicationOperator::Render() const
{
    return RenderForMe(a)+" ⇒ "+RenderForMe(b);
}


Expression::Precedence ImplicationOperator::GetPrecedence() const
{
    return Precedence::IMPLICATION;
}
