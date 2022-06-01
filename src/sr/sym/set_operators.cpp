#include "set_operators.hpp"
#include "result.hpp"
#include "../the_knowledge.hpp"
#include "../lacing.hpp"

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

    
Expression::KnowledgeLevel AllGreaterOperator::GetRequiredKnowledgeLevel() const
{
    return KnowledgeLevel::NUGGETS;
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

    
Expression::KnowledgeLevel AllLessOperator::GetRequiredKnowledgeLevel() const
{
    return KnowledgeLevel::NUGGETS;
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

    
Expression::KnowledgeLevel AllGreaterOrEqualOperator::GetRequiredKnowledgeLevel() const
{
    return KnowledgeLevel::NUGGETS;
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

    
Expression::KnowledgeLevel AllLessOrEqualOperator::GetRequiredKnowledgeLevel() const
{
    return KnowledgeLevel::NUGGETS;
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

// ------------------------- AllCouplingEquivalentOperator --------------------------

AllCouplingEquivalentOperator::AllCouplingEquivalentOperator( shared_ptr<SymbolExpression> a_ ) :
    a( a_ )
{
}

    
Expression::KnowledgeLevel AllCouplingEquivalentOperator::GetRequiredKnowledgeLevel() const
{
    return KnowledgeLevel::GENERAL;
}


list<shared_ptr<SymbolExpression>> AllCouplingEquivalentOperator::GetSymbolOperands() const
{
    return {a};
}


unique_ptr<SymbolResultInterface> AllCouplingEquivalentOperator::Evaluate( const EvalKit &kit,
                                                                           list<unique_ptr<SymbolResultInterface>> &&op_results ) const                                                                    
{
    unique_ptr<SymbolResultInterface> ar = OnlyElementOf(move(op_results));       
    return make_unique<EquivalenceClassResult>( kit.knowledge, ar->GetOnlyXLink() );
}


string AllCouplingEquivalentOperator::Render() const
{
    return "{≡" + RenderForMe(a) + "}";
}


Expression::Precedence AllCouplingEquivalentOperator::GetPrecedence() const
{
    return Precedence::COMPARE;
}

// ------------------------- AllOfKindOperator --------------------------

AllOfKindOperator::AllOfKindOperator( TreePtr<Node> archetype_node_ ) :
    archetype_node(archetype_node_) 
{
}

    
Expression::KnowledgeLevel AllOfKindOperator::GetRequiredKnowledgeLevel() const
{
    return KnowledgeLevel::GENERAL;
}


list<shared_ptr<SymbolExpression>> AllOfKindOperator::GetSymbolOperands() const
{
    return {};
}


unique_ptr<SymbolResultInterface> AllOfKindOperator::Evaluate( const EvalKit &kit,
                                                               list<unique_ptr<SYM::SymbolResultInterface>> &&op_results ) const                                                                    
{
    // Could be done earlier but needs access to knowledge plan. TODO no reason not to provide this to "Solve" functions.
    const list<pair<int, int>> &int_range_list = kit.knowledge->GetLacing()->GetRangeListForCategory(archetype_node);
    CategoryRangeResult::XLinkBoundsList vxlink_range_list;
    for( pair<int, int> int_range : int_range_list )
    {
        // int_range is a half-open minimax
        vxlink_range_list.push_back( make_pair( make_unique<SR::TheKnowledge::CategoryVXLink>(int_range.first),
                                                make_unique<SR::TheKnowledge::CategoryVXLink>(int_range.second) ) );
    }
    TRACE(archetype_node)("\n")(int_range_list)("\n")(vxlink_range_list)("\n");
        
    return make_unique<CategoryRangeResult>( kit.knowledge, move(vxlink_range_list), true, false );    
}


string AllOfKindOperator::Render() const
{
    return "{KindOf<" + archetype_node->GetTypeName() + ">}";
}


SYM::Expression::Precedence AllOfKindOperator::GetPrecedence() const
{
    return Precedence::COMPARE;
}
