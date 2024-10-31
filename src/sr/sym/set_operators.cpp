#include "set_operators.hpp"
#include "result.hpp"
#include "../db/x_tree_database.hpp"

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


unique_ptr<SymbolicResult> ComplementOperator::Evaluate( const EvalKit &kit,
                                                                list<unique_ptr<SymbolicResult>> &&op_results ) const                                                                    
{
    unique_ptr<SymbolicResult> ar = OnlyElementOf(move(op_results));       
    unique_ptr<SubsetResult> asr = make_unique<SubsetResult>( move(ar) );
    return asr->GetComplement();
}


string ComplementOperator::Render() const
{
    // ç is being used to mean complement of, since existing symbols not that great
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


unique_ptr<SymbolicResult> UnionOperator::Evaluate( const EvalKit &kit,
                                                           list<unique_ptr<SymbolicResult>> &&op_results ) const
{
    list<unique_ptr<SubsetResult>> ssrs;
    for( unique_ptr<SymbolicResult> &ar : op_results )       
        ssrs.push_back( make_unique<SubsetResult>( move(ar) ) );
    return SubsetResult::GetUnion( move(ssrs) );
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


unique_ptr<SymbolicResult> IntersectionOperator::Evaluate( const EvalKit &kit,
                                                                  list<unique_ptr<SymbolicResult>> &&op_results ) const
{
    list<unique_ptr<SubsetResult>> ssrs;
    for( unique_ptr<SymbolicResult> &ar : op_results )       
        ssrs.push_back( make_unique<SubsetResult>( move(ar) ) );
    return SubsetResult::GetIntersection( move(ssrs) );
}


string IntersectionOperator::Render() const
{
    // ç{} is universal set aka whole domain
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

    
Expression::VariablesRequiringRows AllGreaterOperator::GetVariablesRequiringRows() const
{
    return GetRequiredVariables();
}


list<shared_ptr<SymbolExpression>> AllGreaterOperator::GetSymbolOperands() const
{
    return {a};
}


unique_ptr<SymbolicResult> AllGreaterOperator::Evaluate( const EvalKit &kit,
                                                                list<unique_ptr<SymbolicResult>> &&op_results ) const                                                                    
{
    unique_ptr<SymbolicResult> ar = OnlyElementOf(move(op_results));       
    return make_unique<DepthFirstRangeResult>( kit.x_tree_db, ar->GetOnlyXLink(), false, SR::XLink(), false ); // TODO ar could be a range
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

    
Expression::VariablesRequiringRows AllLessOperator::GetVariablesRequiringRows() const
{
    return GetRequiredVariables();
}


list<shared_ptr<SymbolExpression>> AllLessOperator::GetSymbolOperands() const
{
    return {a};
}


unique_ptr<SymbolicResult> AllLessOperator::Evaluate( const EvalKit &kit,
                                                             list<unique_ptr<SymbolicResult>> &&op_results ) const                                                                    
{
    unique_ptr<SymbolicResult> ar = OnlyElementOf(move(op_results));       
    return make_unique<DepthFirstRangeResult>( kit.x_tree_db, SR::XLink(), false, ar->GetOnlyXLink(), false );
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

    
Expression::VariablesRequiringRows AllGreaterOrEqualOperator::GetVariablesRequiringRows() const
{
    return GetRequiredVariables();
}


list<shared_ptr<SymbolExpression>> AllGreaterOrEqualOperator::GetSymbolOperands() const
{
    return {a};
}


unique_ptr<SymbolicResult> AllGreaterOrEqualOperator::Evaluate( const EvalKit &kit,
                                                                       list<unique_ptr<SymbolicResult>> &&op_results ) const                                                                    
{
    unique_ptr<SymbolicResult> ar = OnlyElementOf(move(op_results));       
    return make_unique<DepthFirstRangeResult>( kit.x_tree_db, ar->GetOnlyXLink(), true, SR::XLink(), false );
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

    
Expression::VariablesRequiringRows AllLessOrEqualOperator::GetVariablesRequiringRows() const
{
    return GetRequiredVariables();
}


list<shared_ptr<SymbolExpression>> AllLessOrEqualOperator::GetSymbolOperands() const
{
    return {a};
}


unique_ptr<SymbolicResult> AllLessOrEqualOperator::Evaluate( const EvalKit &kit,
                                                                    list<unique_ptr<SymbolicResult>> &&op_results ) const                                                                    
{
    unique_ptr<SymbolicResult> ar = OnlyElementOf(move(op_results));       
    return make_unique<DepthFirstRangeResult>( kit.x_tree_db, SR::XLink(), false, ar->GetOnlyXLink(), true );
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
                                                                  Orderable::BoundingRole lower_role_,
                                                                  bool lower_incl_,
                                                                  shared_ptr<SymbolExpression> upper_,
                                                                  Orderable::BoundingRole upper_role_,
                                                                  bool upper_incl_ ) :
    lower( lower_ ),
    lower_role( lower_role_ ),
    lower_incl( lower_incl_ ),
    upper( upper_ ),
    upper_role( upper_role_ ),
    upper_incl( upper_incl_ )
{
}

      
list<shared_ptr<SymbolExpression>> AllInSimpleCompareRangeOperator::GetSymbolOperands() const
{
    return { lower, upper };
}


unique_ptr<SymbolicResult> AllInSimpleCompareRangeOperator::Evaluate( const EvalKit &kit ) const                                                                    
{
    SR::XLink lower_xlink = lower->Evaluate(kit)->GetOnlyXLink();

    // Optimise case when operands are equal by only evaluating once
    SR::XLink upper_xlink = (upper==lower) ? 
                            lower_xlink : 
                            upper->Evaluate(kit)->GetOnlyXLink();

	if( lower_role != BoundingRole::NONE )
	{
		auto node = MakeTreeNode<SR::SimpleCompareRelation::MinimaxNode>( lower_xlink.GetChildTreePtr(), lower_role );
		lower_xlink = SR::XLink::CreateDistinct( node );
	}

	if( upper_role != BoundingRole::NONE )
	{
		auto node = MakeTreeNode<SR::SimpleCompareRelation::MinimaxNode>( upper_xlink.GetChildTreePtr(), upper_role );
		upper_xlink = SR::XLink::CreateDistinct( node );
	}

    return make_unique<SimpleCompareRangeResult>( kit.x_tree_db, lower_xlink, lower_incl, upper_xlink, upper_incl ); 
}


string AllInSimpleCompareRangeOperator::Render() const
{
    // ∈ etc means elt included (closed bound); ∉ means elt not included (open bound)
    list<string> restrictions;
    
    if( lower )
        restrictions.push_back( string(lower_incl?"∈":"∉") + lower->Render() );
    if( upper )
        restrictions.push_back( upper->Render() + string(upper_incl?"∋":"∌") );
        
    return Join(restrictions, ", ", "{SC ", "}");
}


Expression::Precedence AllInSimpleCompareRangeOperator::GetPrecedence() const
{
    return Precedence::COMPARE;
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


unique_ptr<SymbolicResult> AllInCategoryRangeOperator::Evaluate( const EvalKit &kit ) const                                                                    
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
    return make_unique<CategoryRangeResult>( kit.x_tree_db, move(xlink_bounds_list), lower_incl, upper_incl );    
}


string AllInCategoryRangeOperator::Render() const
{
    // ∈ etc means elt included (closed bound); ∉ means elt not included (open bound)
    list<string> terms;;
    for( const ExprBounds &bounds : bounds_list )
    {
        list<string> restrictions;
        restrictions.push_back( string(lower_incl?"∈":"∉") + bounds.first->Render() );
        restrictions.push_back( bounds.second->Render() + string(upper_incl?"∋":"∌") );
        terms.push_back( Join(restrictions, ", ") );
    }
    return Join(terms, " ∪ ", "{CAT ", "}");
}


Expression::Precedence AllInCategoryRangeOperator::GetPrecedence() const
{
    return Precedence::SCOPE;
}

