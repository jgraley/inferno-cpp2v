#include "set_operators.hpp"
#include "result.hpp"
#include "../the_knowledge.hpp"

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


unique_ptr<SymbolResultInterface> ComplementOperator::Evaluate( const EvalKit &kit,
                                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const                                                                    
{
    unique_ptr<SymbolResultInterface> ar = OnlyElementOf(move(op_results));       
    unique_ptr<SetResult> asr = make_unique<SetResult>( move(ar) );
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


unique_ptr<SymbolResultInterface> UnionOperator::Evaluate( const EvalKit &kit,
                                                           list<unique_ptr<SymbolResultInterface>> &&op_results ) const
{
    list<unique_ptr<SetResult>> ssrs;
    for( unique_ptr<SymbolResultInterface> &ar : op_results )       
        ssrs.push_back( make_unique<SetResult>( move(ar) ) );
    return SetResult::GetUnion( move(ssrs) );
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


unique_ptr<SymbolResultInterface> IntersectionOperator::Evaluate( const EvalKit &kit,
                                                                  list<unique_ptr<SymbolResultInterface>> &&op_results ) const
{
    list<unique_ptr<SetResult>> ssrs;
    for( unique_ptr<SymbolResultInterface> &ar : op_results )       
        ssrs.push_back( make_unique<SetResult>( move(ar) ) );
    return SetResult::GetIntersection( move(ssrs) );
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

    
Expression::VariablesRequiringNuggets AllGreaterOperator::GetVariablesRequiringNuggets() const
{
    return GetRequiredVariables();
}


list<shared_ptr<SymbolExpression>> AllGreaterOperator::GetSymbolOperands() const
{
    return {a};
}


unique_ptr<SymbolResultInterface> AllGreaterOperator::Evaluate( const EvalKit &kit,
                                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const                                                                    
{
    unique_ptr<SymbolResultInterface> ar = OnlyElementOf(move(op_results));       
    return make_unique<DepthFirstRangeResult>( kit.knowledge, ar->GetOnlyXLink(), false, SR::XLink(), false );
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

    
Expression::VariablesRequiringNuggets AllLessOperator::GetVariablesRequiringNuggets() const
{
    return GetRequiredVariables();
}


list<shared_ptr<SymbolExpression>> AllLessOperator::GetSymbolOperands() const
{
    return {a};
}


unique_ptr<SymbolResultInterface> AllLessOperator::Evaluate( const EvalKit &kit,
                                                             list<unique_ptr<SymbolResultInterface>> &&op_results ) const                                                                    
{
    unique_ptr<SymbolResultInterface> ar = OnlyElementOf(move(op_results));       
    return make_unique<DepthFirstRangeResult>( kit.knowledge, SR::XLink(), false, ar->GetOnlyXLink(), false );
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

    
Expression::VariablesRequiringNuggets AllGreaterOrEqualOperator::GetVariablesRequiringNuggets() const
{
    return GetRequiredVariables();
}


list<shared_ptr<SymbolExpression>> AllGreaterOrEqualOperator::GetSymbolOperands() const
{
    return {a};
}


unique_ptr<SymbolResultInterface> AllGreaterOrEqualOperator::Evaluate( const EvalKit &kit,
                                                                       list<unique_ptr<SymbolResultInterface>> &&op_results ) const                                                                    
{
    unique_ptr<SymbolResultInterface> ar = OnlyElementOf(move(op_results));       
    return make_unique<DepthFirstRangeResult>( kit.knowledge, ar->GetOnlyXLink(), true, SR::XLink(), false );
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

    
Expression::VariablesRequiringNuggets AllLessOrEqualOperator::GetVariablesRequiringNuggets() const
{
    return GetRequiredVariables();
}


list<shared_ptr<SymbolExpression>> AllLessOrEqualOperator::GetSymbolOperands() const
{
    return {a};
}


unique_ptr<SymbolResultInterface> AllLessOrEqualOperator::Evaluate( const EvalKit &kit,
                                                                    list<unique_ptr<SymbolResultInterface>> &&op_results ) const                                                                    
{
    unique_ptr<SymbolResultInterface> ar = OnlyElementOf(move(op_results));       
    return make_unique<DepthFirstRangeResult>( kit.knowledge, SR::XLink(), false, ar->GetOnlyXLink(), true );
}


string AllLessOrEqualOperator::Render() const
{
    return "{<=" + RenderForMe(a) + "}";
}


Expression::Precedence AllLessOrEqualOperator::GetPrecedence() const
{
    return Precedence::COMPARE;
}

// ------------------------- AllSimpleCompareEquivalentOperator --------------------------

AllSimpleCompareEquivalentOperator::AllSimpleCompareEquivalentOperator( shared_ptr<SymbolExpression> a_ ) :
    a( a_ )
{
}

    
list<shared_ptr<SymbolExpression>> AllSimpleCompareEquivalentOperator::GetSymbolOperands() const
{
    return {a};
}


unique_ptr<SymbolResultInterface> AllSimpleCompareEquivalentOperator::Evaluate( const EvalKit &kit,
                                                                                list<unique_ptr<SymbolResultInterface>> &&op_results ) const                                                                    
{
    unique_ptr<SymbolResultInterface> ar = OnlyElementOf(move(op_results));       

    // Simulate multiset::equal_range() with our ordered domain as an
    // ordering in order to get to the set of equivalent elements without
    // having to iterate over the whole domain. We're still gaining entropy
    // here though. It would be faster to get to the range via nuggets 
    // (because XLink native comparison will be faster than SimpleCompare)
    // but ar might be an arbitrary force, and not in the domain.
    // See #522 #525
    // X, true, X, true gets simulates equal_range()    
    return make_unique<SimpleCompareRangeResult>( kit.knowledge, ar->GetOnlyXLink(), true, ar->GetOnlyXLink(), true ); 
}


string AllSimpleCompareEquivalentOperator::Render() const
{
    return "{≡" + RenderForMe(a) + "}";
}


Expression::Precedence AllSimpleCompareEquivalentOperator::GetPrecedence() const
{
    return Precedence::COMPARE;
}

// ------------------------- AllInSimpleCompareRangeOperator --------------------------

AllInSimpleCompareRangeOperator::AllInSimpleCompareRangeOperator( pair<SR::XLink, SR::XLink> &&bounds_, bool lower_incl_, bool upper_incl_ ) :
    bounds( move(bounds_) ),
    lower_incl( lower_incl_ ),
    upper_incl( upper_incl_ )
{
}


AllInSimpleCompareRangeOperator::AllInSimpleCompareRangeOperator( pair<TreePtr<Node>, TreePtr<Node>> &&bounds_, bool lower_incl_, bool upper_incl_ ) :
    bounds( make_pair(SR::XLink::CreateDistinct(bounds_.first), SR::XLink::CreateDistinct(bounds_.second) ) ),
    lower_incl( lower_incl_ ),
    upper_incl( upper_incl_ )
{
}


list<shared_ptr<SymbolExpression>> AllInSimpleCompareRangeOperator::GetSymbolOperands() const
{
    return {};
}


unique_ptr<SymbolResultInterface> AllInSimpleCompareRangeOperator::Evaluate( const EvalKit &kit,
                                                                             list<unique_ptr<SymbolResultInterface>> &&op_results ) const                                                                    
{        
    return make_unique<SimpleCompareRangeResult>( kit.knowledge, bounds.first, lower_incl, bounds.second, upper_incl );
}


string AllInSimpleCompareRangeOperator::Render() const
{
    // No operands, so I always evaluate to the same thing, so my render 
    // string can be my result's render string.
    EvalKit empty_kit;
    return Evaluate(empty_kit, {})->Render();
}


Expression::Precedence AllInSimpleCompareRangeOperator::GetPrecedence() const
{
    return Precedence::SCOPE;
}

// ------------------------- AllInCategoryRangeOperator --------------------------

AllInCategoryRangeOperator::AllInCategoryRangeOperator( CategoryRangeResult::XLinkBoundsList &&bounds_list_, bool lower_incl_, bool upper_incl_ ) :
    bounds_list( move(bounds_list_) ),
    lower_incl( lower_incl_ ),
    upper_incl( upper_incl_ )
{
}


list<shared_ptr<SymbolExpression>> AllInCategoryRangeOperator::GetSymbolOperands() const
{
    return {};
}


unique_ptr<SymbolResultInterface> AllInCategoryRangeOperator::Evaluate( const EvalKit &kit,
                                                                        list<unique_ptr<SymbolResultInterface>> &&op_results ) const                                                                    
{        
    return make_unique<CategoryRangeResult>( kit.knowledge, bounds_list, lower_incl, upper_incl );    
}


string AllInCategoryRangeOperator::Render() const
{
    // No operands, so I always evaluate to the same thing, so my render 
    // string can be my result's render string.
    EvalKit empty_kit;
    return Evaluate(empty_kit, {})->Render();
}


Expression::Precedence AllInCategoryRangeOperator::GetPrecedence() const
{
    return Precedence::SCOPE;
}

