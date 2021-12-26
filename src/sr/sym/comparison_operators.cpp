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
        if( !ra->xlink || !rb->xlink )
        {
            if( m == BooleanResult::TRUE )
                m = BooleanResult::UNKNOWN;
        }
        else if( ra->xlink != rb->xlink )
        {
            m = BooleanResult::FALSE;
        }
    } );
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
    return Precedence::COMPARE;
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

    if( !ra->xlink || !rb->xlink )
        return make_shared<BooleanResult>(BooleanResult::UNKNOWN);
    
    // For (un)equality, it is sufficient to compare the x links
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
    return Precedence::COMPARE;
}


Lazy<BooleanExpression> SYM::operator!=( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<NotEqualOperator>( a, b );
}

// ------------------------- IndexComparisonOperator --------------------------

IndexComparisonOperator::IndexComparisonOperator( shared_ptr<SymbolExpression> a_, 
                                                  shared_ptr<SymbolExpression> b_ ) :
    a(a_),
    b(b_)
{
    // Note: not an alldiff, see #429
}    
    

list<shared_ptr<SymbolExpression>> IndexComparisonOperator::GetSymbolOperands() const
{
    return {a, b};
}


shared_ptr<BooleanResult> IndexComparisonOperator::Evaluate( const EvalKit &kit,
                                                             const list<shared_ptr<SymbolResult>> &op_results ) const 
{    
    ASSERT( op_results.size()==2 );
    shared_ptr<SymbolResult> ra = op_results.front();
    shared_ptr<SymbolResult> rb = op_results.back();

    if( !ra->xlink || !rb->xlink )
        return make_shared<BooleanResult>(BooleanResult::UNKNOWN);
    
    // For greater/less, we need to consult the knowledge. We use the 
    // overall depth-first ordering.
    const SR::TheKnowledge::Nugget &nugget_a( kit.knowledge->GetNugget(ra->xlink) );   
    const SR::TheKnowledge::Nugget &nugget_b( kit.knowledge->GetNugget(rb->xlink) );   
    SR::TheKnowledge::IndexType index_a = nugget_a.depth_first_index;
    SR::TheKnowledge::IndexType index_b = nugget_b.depth_first_index;
    
    if( EvalBoolFromIndexes( index_a, index_b ) )
        return make_shared<BooleanResult>(BooleanResult::TRUE);
    else
        return make_shared<BooleanResult>(BooleanResult::FALSE);   
}


Expression::Precedence IndexComparisonOperator::GetPrecedence() const
{
    return Precedence::COMPARE;
}

// ------------------------- GreaterOperator --------------------------

bool GreaterOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                           SR::TheKnowledge::IndexType index_b ) const
{
    return index_a > index_b;
}                    
            
                                  
string GreaterOperator::Render() const
{
    return RenderForMe(a) + " > " + RenderForMe(b);
}


Lazy<BooleanExpression> SYM::operator>( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<GreaterOperator>( a, b );
}

// ------------------------- LessOperator --------------------------

bool LessOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                        SR::TheKnowledge::IndexType index_b ) const
{
    return index_a < index_b;
}                    
            
                                  
string LessOperator::Render() const
{
    return RenderForMe(a) + " < " + RenderForMe(b);
}


Lazy<BooleanExpression> SYM::operator<( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<LessOperator>( a, b );
}

// ------------------------- GreaterOrEqualOperator --------------------------

bool GreaterOrEqualOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                                  SR::TheKnowledge::IndexType index_b ) const
{
    return index_a >= index_b;
}                    
            
                                  
string GreaterOrEqualOperator::Render() const
{
    return RenderForMe(a) + " >= " + RenderForMe(b);
}


Lazy<BooleanExpression> SYM::operator>=( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<GreaterOrEqualOperator>( a, b );
}

// ------------------------- LessOrEqualOperator --------------------------

bool LessOrEqualOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                               SR::TheKnowledge::IndexType index_b ) const
{
    return index_a <= index_b;
}                    
            
                                  
string LessOrEqualOperator::Render() const
{
    return RenderForMe(a) + " <= " + RenderForMe(b);
}


Lazy<BooleanExpression> SYM::operator<=( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<LessOrEqualOperator>( a, b );
}

// ------------------------- AllDiffOperator --------------------------

AllDiffOperator::AllDiffOperator( list< shared_ptr<SymbolExpression> > sa_ ) :
    sa(sa_)
{
}    
    

list<shared_ptr<SymbolExpression>> AllDiffOperator::GetSymbolOperands() const
{
    return sa;
}


shared_ptr<BooleanResult> AllDiffOperator::Evaluate( const EvalKit &kit,
                                                    const list<shared_ptr<SymbolResult>> &op_results ) const 
{
    BooleanResult::BooleanValue m = BooleanResult::TRUE;
    ForAllCommutativeDistinctPairs( op_results, [&](shared_ptr<SymbolResult> ra,
                                                    shared_ptr<SymbolResult> rb) 
    {
        // For equality, it is sufficient to compare the x links
        // themselves, which have the required uniqueness properties
        // within the full arrowhead model.
        if( !ra->xlink || !rb->xlink )
        {
            if( m == BooleanResult::TRUE )
                m = BooleanResult::UNKNOWN;
        }
        else if( ra->xlink == rb->xlink )
        {
            m = BooleanResult::FALSE;
        }
    } );
    return make_shared<BooleanResult>( m );   
}


string AllDiffOperator::Render() const
{
    list<string> ls;
    for( shared_ptr<SymbolExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return "AllDiff" + Join( ls, ", ", "( ", " )" );
}


Expression::Precedence AllDiffOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
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
