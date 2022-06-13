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

// ------------------------- AllInSimpleCompareRangeOperator --------------------------

AllInSimpleCompareRangeOperator::AllInSimpleCompareRangeOperator( shared_ptr<SymbolExpression> lower_,
                                                                  bool lower_incl_,
                                                                  shared_ptr<SymbolExpression> upper_,
                                                                  bool upper_incl_ ) :
    lower( lower_ ),
    lower_incl( lower_incl_ ),
    upper( upper_ ),
    upper_incl( upper_incl_ )
{
}

      
list<shared_ptr<SymbolExpression>> AllInSimpleCompareRangeOperator::GetSymbolOperands() const
{
    return { lower, upper };
}


unique_ptr<SymbolResultInterface> AllInSimpleCompareRangeOperator::Evaluate( const EvalKit &kit ) const                                                                    
{
    SR::XLink lower_xlink = lower->Evaluate(kit)->GetOnlyXLink();

    // Optimise case when operands are equal by only evaluating once
    SR::XLink upper_xlink = (upper==lower) ? 
                            lower_xlink : 
                            upper->Evaluate(kit)->GetOnlyXLink();

    return make_unique<SimpleCompareRangeResult>( kit.knowledge, lower_xlink, lower_incl, upper_xlink, upper_incl ); 
}


string AllInSimpleCompareRangeOperator::Render() const
{
    list<string> restrictions;
    
    if( lower )
        restrictions.push_back( string(lower_incl?"[":"(") + lower->Render() );
    if( upper )
        restrictions.push_back( upper->Render() + string(upper_incl?"]":")") );
        
    return Join(restrictions, ", ", "{SC ", " }");
}


Expression::Precedence AllInSimpleCompareRangeOperator::GetPrecedence() const
{
    return Precedence::COMPARE;
}

// ------------------------- AllInCategoryFixedRangeOperator --------------------------

AllInCategoryFixedRangeOperator::AllInCategoryFixedRangeOperator( CategoryRangeResult::XLinkBoundsList &&bounds_list_, bool lower_incl_, bool upper_incl_ ) :
    bounds_list( move(bounds_list_) ),
    lower_incl( lower_incl_ ),
    upper_incl( upper_incl_ )
{
}


list<shared_ptr<SymbolExpression>> AllInCategoryFixedRangeOperator::GetSymbolOperands() const
{
    return {};
}


unique_ptr<SymbolResultInterface> AllInCategoryFixedRangeOperator::Evaluate( const EvalKit &kit,
                                                                        list<unique_ptr<SymbolResultInterface>> &&op_results ) const                                                                    
{        
    return make_unique<CategoryRangeResult>( kit.knowledge, bounds_list, lower_incl, upper_incl );    
}


string AllInCategoryFixedRangeOperator::Render() const
{
    // No operands, so I always evaluate to the same thing, so my render 
    // string can be my result's render string.
    EvalKit empty_kit;
    return Evaluate(empty_kit, {})->Render();
}


Expression::Precedence AllInCategoryFixedRangeOperator::GetPrecedence() const
{
    return Precedence::SCOPE;
}


// ------------------------- AllInCategoryRangeOperator --------------------------

AllInCategoryRangeOperator::AllInCategoryRangeOperator( ExprBoundsList &&bounds_list_, bool lower_incl_, bool upper_incl_ ) :
    bounds_list( move(bounds_list_) ),
    lower_incl( lower_incl_ ),
    upper_incl( upper_incl_ )
{
}


list<shared_ptr<SymbolExpression>> AllInCategoryRangeOperator::GetSymbolOperands() const
{
    list<shared_ptr<SymbolExpression>> ops;
    for( const ExprBounds &bounds : bounds_list )
    {
        ops.push_back( bounds.first );
        ops.push_back( bounds.second );
    }
    return ops;
}


unique_ptr<SymbolResultInterface> AllInCategoryRangeOperator::Evaluate( const EvalKit &kit ) const                                                                    
{        
    CategoryRangeResult::XLinkBoundsList xlink_bounds_list;
    for( const ExprBounds &bounds : bounds_list )
    {
        SR::XLink lower_xlink = bounds.first->Evaluate(kit)->GetOnlyXLink();

        // Optimise case when operands are equal by only evaluating once
        SR::XLink upper_xlink = (bounds.second==bounds.first) ? 
                                lower_xlink : 
                                bounds.second->Evaluate(kit)->GetOnlyXLink();
                              
        xlink_bounds_list.push_back( make_pair( make_unique<SR::XLink>(lower_xlink),
                                                make_unique<SR::XLink>(upper_xlink) ) );
    }
    return make_unique<CategoryRangeResult>( kit.knowledge, xlink_bounds_list, lower_incl, upper_incl );    
}


string AllInCategoryRangeOperator::Render() const
{
    list<string> terms;;
    for( const ExprBounds &bounds : bounds_list )
    {
        list<string> restrictions;
        restrictions.push_back( string(lower_incl?"[":"(") + bounds.first->Render() );
        restrictions.push_back( bounds.second->Render() + string(upper_incl?"]":")") );
        terms.push_back( Join(restrictions, ", ") );
    }
    return Join(terms, " ∪ ", "{CAT ", " }");
}


Expression::Precedence AllInCategoryRangeOperator::GetPrecedence() const
{
    return Precedence::SCOPE;
}

