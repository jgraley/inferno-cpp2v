#include "comparison_operators.hpp"
#include "../agents/agent.hpp"

using namespace SYM;

// ------------------------- EqualOperator --------------------------

EqualOperator::EqualOperator( list< shared_ptr<SymbolExpression> > sa_ ) :
    sa(sa_)
{
}    
    

list<shared_ptr<SymbolExpression>> EqualOperator::GetSymbolOperands() const
{
    return sa;
}


shared_ptr<BooleanResult> EqualOperator::Evaluate( const EvalKit &kit,
                                                    const list<shared_ptr<SymbolResult>> &op_results ) const 
{
    BooleanResult::BooleanValue m = BooleanResult::TRUE;
    ForOverlappingAdjacentPairs( op_results, [&](shared_ptr<SymbolResult> ra,
                                                 shared_ptr<SymbolResult> rb) 
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
    return make_shared<BooleanResult>( m );   
}


string EqualOperator::Render() const
{
    list<string> ls;
    for( shared_ptr<SymbolExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, " == " );
}


Expression::Precedence EqualOperator::GetPrecedence() const
{
    return Precedence::COMPARE_EQNE;
}


Lazy<BooleanExpression> SYM::operator==( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<EqualOperator>( list< shared_ptr<SymbolExpression> >({ a, b }) );
}

// ------------------------- NotEqualOperator --------------------------

NotEqualOperator::NotEqualOperator( shared_ptr<SymbolExpression> a_, 
                                    shared_ptr<SymbolExpression> b_ ) :
    a(a_),
    b(b_)
{
    // Note: not an alldiff, see #429
}    
    

list<shared_ptr<SymbolExpression>> NotEqualOperator::GetSymbolOperands() const
{
    return {a, b};
}


shared_ptr<BooleanResult> NotEqualOperator::Evaluate( const EvalKit &kit,
                                                      const list<shared_ptr<SymbolResult>> &op_results ) const 
{    
    ASSERT( op_results.size()==2 );
    shared_ptr<SymbolResult> ra = op_results.front();
    shared_ptr<SymbolResult> rb = op_results.back();

    if( (!ra->xlink || !rb->xlink) )
        return make_shared<BooleanResult>(BooleanResult::UNKNOWN);
    
    // For (in)equality, it is sufficient to compare the x links
    // themselves, which have the required uniqueness properties
    // within the full arrowhead model.
    if( ra->xlink != rb->xlink )
        return make_shared<BooleanResult>(BooleanResult::TRUE);
    else
        return make_shared<BooleanResult>(BooleanResult::FALSE);   
}


string NotEqualOperator::Render() const
{
    return RenderForMe(a) + " != " + RenderForMe(b);
}


Expression::Precedence NotEqualOperator::GetPrecedence() const
{
    return Precedence::COMPARE_EQNE;
}


Lazy<BooleanExpression> SYM::operator!=( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<NotEqualOperator>( a, b );
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


shared_ptr<BooleanResult> KindOfOperator::Evaluate( const EvalKit &kit,
                                                    const list<shared_ptr<SymbolResult>> &op_results ) const 
{
    shared_ptr<SymbolResult> ar = OnlyElementOf(op_results);
    if( !ar->xlink )
        return make_shared<BooleanResult>( BooleanResult::UNKNOWN );
    bool matches = ref_agent->IsLocalMatch( ar->xlink.GetChildX().get() );
    return make_shared<BooleanResult>( matches ? BooleanResult::TRUE : BooleanResult::FALSE );
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
