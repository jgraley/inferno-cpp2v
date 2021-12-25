#include "comparison_operators.hpp"
#include "../agents/agent.hpp"

using namespace SYM;

// ------------------------- EqualsOperator --------------------------

EqualsOperator::EqualsOperator( list< shared_ptr<SymbolExpression> > sa_ ) :
    sa(sa_)
{
}    
    

list<shared_ptr<SymbolExpression>> EqualsOperator::GetSymbolOperands() const
{
    return sa;
}


unique_ptr<BooleanResult> EqualsOperator::Evaluate( const EvalKit &kit,
                                                    const list<unique_ptr<SymbolResult>> &op_results ) const 
{
    BooleanResult::BooleanValue m = BooleanResult::TRUE;
    ForOverlappingAdjacentPairs( op_results, [&](const unique_ptr<SymbolResult> &ra,
                                                 const unique_ptr<SymbolResult> &rb) 
    {
        // For equality, it is sufficient to compare the x links
        // themselves, which have the required uniqueness properties
        // within the full arrowhead model.
        if( (!ra->xlink || !rb->xlink) )
        {
            if( m != BooleanResult::FALSE )
                m = BooleanResult::UNKNOWN;
        }
        else if( ra->xlink != rb->xlink )
            m = BooleanResult::FALSE;
    });
    return make_unique<BooleanResult>( m );   
}


string EqualsOperator::Render() const
{
    list<string> ls;
    for( shared_ptr<SymbolExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, " == " );
}


Expression::Precedence EqualsOperator::GetPrecedence() const
{
    return Precedence::COMPARE_EQNE;
}


Lazy<BooleanExpression> SYM::operator==( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<EqualsOperator>( list< shared_ptr<SymbolExpression> >({ a, b }) );
}

// ------------------------- NotEqualsOperator --------------------------

NotEqualsOperator::NotEqualsOperator( list< shared_ptr<SymbolExpression> > sa_ ) :
    sa(sa_)
{
    ASSERT( sa.size()==2 );
    // Note: not an alldiff, and not well defined for more than 2 operands, see #429
}    
    

list<shared_ptr<SymbolExpression>> NotEqualsOperator::GetSymbolOperands() const
{
    return sa;
}


unique_ptr<BooleanResult> NotEqualsOperator::Evaluate( const EvalKit &kit,
                                                       const list<unique_ptr<SymbolResult>> &op_results ) const 
{    
    BooleanResult::BooleanValue m = BooleanResult::TRUE;
    ForOverlappingAdjacentPairs( op_results, [&](const unique_ptr<SymbolResult> &ra,
                                                 const unique_ptr<SymbolResult> &rb) 
    {
        // For equality, it is sufficient to compare the x links
        // themselves, which have the required uniqueness properties
        // within the full arrowhead model.
        if( (!ra->xlink || !rb->xlink) )
        {
            if( m != BooleanResult::FALSE )
                m = BooleanResult::UNKNOWN;
        }
        else if( ra->xlink == rb->xlink )
            m = BooleanResult::FALSE;
    });
    return make_unique<BooleanResult>( m );   
}


string NotEqualsOperator::Render() const
{
    list<string> ls;
    for( shared_ptr<SymbolExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, " != " );
}


Expression::Precedence NotEqualsOperator::GetPrecedence() const
{
    return Precedence::COMPARE_EQNE;
}


Lazy<BooleanExpression> SYM::operator!=( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<NotEqualsOperator>( list< shared_ptr<SymbolExpression> >({ a, b }) );
}

// ------------------------- KindOfOperator --------------------------

KindOfOperator::KindOfOperator( const SR::Agent *ref_agent_,
                                shared_ptr<SymbolExpression> a_ ) :
    a( a_ ),
    ref_agent( ref_agent_ )
{    
}                                                


list<shared_ptr<SymbolExpression>> KindOfOperator::GetSymbolOperands() const
{
    return { a };
}


unique_ptr<BooleanResult> KindOfOperator::Evaluate( const EvalKit &kit,
                                                    const list<unique_ptr<SymbolResult>> &op_results ) const 
{
    const unique_ptr<SymbolResult> &ar = OnlyElementOf(op_results);
    if( !ar->xlink )
        return make_unique<BooleanResult>( BooleanResult::UNKNOWN );
    bool matches = ref_agent->IsLocalMatch( ar->xlink.GetChildX().get() );
    return make_unique<BooleanResult>( matches ? BooleanResult::TRUE : BooleanResult::FALSE );
}


string KindOfOperator::Render() const
{
    string inner_typename = RemoveOuterTemplate( ref_agent->GetTypeName() );

    // Not using RenderForMe() because we always want () here
    return "KindOf<" + inner_typename + ">(" + a->Render() + ")"; 
}


Expression::Precedence KindOfOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}
