#include "set_operators.hpp"
#include "result.hpp"

using namespace SYM;

// ------------------------- ComplementOperator --------------------------

ComplementOperator::ComplementOperator( shared_ptr<SymbolExpression> a_ ) :
    a( a_ )
{
}

    
list<shared_ptr<SymbolExpression>> ComplementOperator::GetSymbolOperands() const
{
    return {a};
}


shared_ptr<SymbolResultInterface> ComplementOperator::Evaluate( const EvalKit &kit,
                                                                const list<shared_ptr<SymbolResultInterface>> &op_results ) const                                                                    
{
    shared_ptr<SymbolResultInterface> ar = OnlyElementOf(op_results);       
    shared_ptr<SymbolSetResult> asr = make_shared<SymbolSetResult>( ar );
    return asr->GetComplement();
}


string ComplementOperator::Render() const
{
    return "ç" + RenderForMe(a);
}


Expression::Precedence ComplementOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}

// ------------------------- UnionOperator --------------------------

UnionOperator::UnionOperator( list< shared_ptr<SymbolExpression> > sa_ ) :
    sa( sa_ )
{   
}    


list<shared_ptr<SymbolExpression>> UnionOperator::GetSymbolOperands() const
{
    return sa;
}


shared_ptr<SymbolResultInterface> UnionOperator::Evaluate( const EvalKit &kit,
                                                           const list<shared_ptr<SymbolResultInterface>> &op_results ) const
{
    list<shared_ptr<SymbolSetResult>> ssrs;
    for( shared_ptr<SymbolResultInterface> ar : op_results )       
        ssrs.push_back( make_shared<SymbolSetResult>( ar ) );
    return SymbolSetResult::GetUnion( ssrs );
}


string UnionOperator::Render() const
{
    if( sa.empty() )
        return "{}";
    list<string> ls;
    for( shared_ptr<SymbolExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, " ∪ " );
}


Expression::Precedence UnionOperator::GetPrecedence() const
{
    return Precedence::OR;
}

// ------------------------- IntersectionOperator --------------------------

IntersectionOperator::IntersectionOperator( list< shared_ptr<SymbolExpression> > sa_ ) :
    sa( sa_ )
{   
}    


list<shared_ptr<SymbolExpression>> IntersectionOperator::GetSymbolOperands() const
{
    return sa;
}


shared_ptr<SymbolResultInterface> IntersectionOperator::Evaluate( const EvalKit &kit,
                                                                  const list<shared_ptr<SymbolResultInterface>> &op_results ) const
{
    list<shared_ptr<SymbolSetResult>> ssrs;
    for( shared_ptr<SymbolResultInterface> ar : op_results )       
        ssrs.push_back( make_shared<SymbolSetResult>( ar ) );
    return SymbolSetResult::GetIntersection( ssrs );
}


string IntersectionOperator::Render() const
{
    if( sa.empty() )
        return "ç{}";
    list<string> ls;
    for( shared_ptr<SymbolExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return Join( ls, " ∩ " );
}


Expression::Precedence IntersectionOperator::GetPrecedence() const
{
    return Precedence::OR;
}

// ------------------------- AllGreaterOperator --------------------------

AllGreaterOperator::AllGreaterOperator( shared_ptr<SymbolExpression> a_ ) :
    a( a_ )
{
}

    
list<shared_ptr<SymbolExpression>> AllGreaterOperator::GetSymbolOperands() const
{
    return {a};
}


shared_ptr<SymbolResultInterface> AllGreaterOperator::Evaluate( const EvalKit &kit,
                                                                const list<shared_ptr<SymbolResultInterface>> &op_results ) const                                                                    
{
    shared_ptr<SymbolResultInterface> ar = OnlyElementOf(op_results);       
    return make_shared<SymbolRangeResult>( kit.knowledge, ar->GetOnlyXLink(), false, SR::XLink(), false );
}


string AllGreaterOperator::Render() const
{
    return "{>" + RenderForMe(a) + "}";
}


Expression::Precedence AllGreaterOperator::GetPrecedence() const
{
    return Precedence::COMPARE;
}

// ------------------------- AllLessOperator --------------------------

AllLessOperator::AllLessOperator( shared_ptr<SymbolExpression> a_ ) :
    a( a_ )
{
}

    
list<shared_ptr<SymbolExpression>> AllLessOperator::GetSymbolOperands() const
{
    return {a};
}


shared_ptr<SymbolResultInterface> AllLessOperator::Evaluate( const EvalKit &kit,
                                                             const list<shared_ptr<SymbolResultInterface>> &op_results ) const                                                                    
{
    shared_ptr<SymbolResultInterface> ar = OnlyElementOf(op_results);       
    return make_shared<SymbolRangeResult>( kit.knowledge, SR::XLink(), false, ar->GetOnlyXLink(), false );
}


string AllLessOperator::Render() const
{
    return "{<" + RenderForMe(a) + "}";
}


Expression::Precedence AllLessOperator::GetPrecedence() const
{
    return Precedence::COMPARE;
}

// ------------------------- AllGreaterOrEqualOperator --------------------------

AllGreaterOrEqualOperator::AllGreaterOrEqualOperator( shared_ptr<SymbolExpression> a_ ) :
    a( a_ )
{
}

    
list<shared_ptr<SymbolExpression>> AllGreaterOrEqualOperator::GetSymbolOperands() const
{
    return {a};
}


shared_ptr<SymbolResultInterface> AllGreaterOrEqualOperator::Evaluate( const EvalKit &kit,
                                                                       const list<shared_ptr<SymbolResultInterface>> &op_results ) const                                                                    
{
    shared_ptr<SymbolResultInterface> ar = OnlyElementOf(op_results);       
    return make_shared<SymbolRangeResult>( kit.knowledge, ar->GetOnlyXLink(), true, SR::XLink(), false );
}


string AllGreaterOrEqualOperator::Render() const
{
    return "{>=" + RenderForMe(a) + "}";
}


Expression::Precedence AllGreaterOrEqualOperator::GetPrecedence() const
{
    return Precedence::COMPARE;
}

// ------------------------- AllLessOrEqualOperator --------------------------

AllLessOrEqualOperator::AllLessOrEqualOperator( shared_ptr<SymbolExpression> a_ ) :
    a( a_ )
{
}

    
list<shared_ptr<SymbolExpression>> AllLessOrEqualOperator::GetSymbolOperands() const
{
    return {a};
}


shared_ptr<SymbolResultInterface> AllLessOrEqualOperator::Evaluate( const EvalKit &kit,
                                                                    const list<shared_ptr<SymbolResultInterface>> &op_results ) const                                                                    
{
    shared_ptr<SymbolResultInterface> ar = OnlyElementOf(op_results);       
    return make_shared<SymbolRangeResult>( kit.knowledge, SR::XLink(), false, ar->GetOnlyXLink(), true );
}


string AllLessOrEqualOperator::Render() const
{
    return "{<=" + RenderForMe(a) + "}";
}


Expression::Precedence AllLessOrEqualOperator::GetPrecedence() const
{
    return Precedence::COMPARE;
}
