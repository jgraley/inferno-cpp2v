#include "predicate_operators.hpp"
#include "boolean_operators.hpp"
#include "symbol_operators.hpp"
#include "set_operators.hpp"
#include "result.hpp"
#include "../db/lacing.hpp"
#include "common/lambda_loops.hpp"

using namespace SYM;

// ------------------------- PredicateOperator --------------------------

void PredicateOperator::SetForceExpression( weak_ptr<BooleanExpression> force_expression_ )
{
    force_expression = force_expression_;
}


void PredicateOperator::SetForceRender( weak_ptr<string> force_render_ )
{
    force_render = force_render_;
}


list<shared_ptr<SymbolExpression>> PredicateOperator::GetSymbolOperands() const
{
    // Casting away the const because we don't modify *p
    list<shared_ptr<SymbolExpression> *> lp = const_cast<PredicateOperator *>(this)->GetSymbolOperandPointers();
    list<shared_ptr<SymbolExpression>> l;
    for( shared_ptr<SymbolExpression> *p : lp )
        l.push_back( *p );
    return l;
}


unique_ptr<BooleanResult> PredicateOperator::Evaluate( const EvalKit &kit ) const
{
    // Apply forces where specified
    shared_ptr<BooleanExpression> locked_expression = force_expression.lock();
    if( locked_expression )
        return locked_expression->Evaluate( kit );

    return Parent::Evaluate( kit );
}


shared_ptr<PredicateOperator> PredicateOperator::TrySubstitute( shared_ptr<SymbolExpression> over,
                                                                shared_ptr<SymbolExpression> with ) const
{
    auto p = Clone();
    
    list<shared_ptr<SymbolExpression> *> sop = p->GetSymbolOperandPointers();
    for( shared_ptr<SymbolExpression> *s : sop )
    {
        if( OrderCompare3Way( *over, **s )==0 )
        {
            *s = with;
            return p;
        }
    }
            
    return nullptr;
}                                                                


Relationship PredicateOperator::GetRelationshipWith( shared_ptr<PredicateOperator> other ) const
{
    return Relationship::NONE;
}


Transitivity PredicateOperator::GetTransitivityWith( shared_ptr<PredicateOperator> other ) const
{
    return Transitivity::NONE;
}


bool PredicateOperator::IsCanSubstituteFrom() const
{
    return false;
}


string PredicateOperator::Render() const 
{
    shared_ptr<string> locked_render = force_render.lock();
    if( locked_render )
        return *locked_render;

    return RenderNF();
}


Expression::Precedence PredicateOperator::GetPrecedence() const
{
    shared_ptr<string> locked_render = force_render.lock();
    if( locked_render )
        return Precedence::LITERAL;

    return GetPrecedenceNF();
}


// ------------------------- IsEqualOperator --------------------------

IsEqualOperator::IsEqualOperator( shared_ptr<SymbolExpression> a_, 
                              shared_ptr<SymbolExpression> b_ ) :
    a(a_),
    b(b_)
{
}    
    
    
shared_ptr<PredicateOperator> IsEqualOperator::Clone() const
{
    return make_shared<IsEqualOperator>( a, b );
}
    

list<shared_ptr<SymbolExpression>*> IsEqualOperator::GetSymbolOperandPointers()
{
    return {&a, &b};
}


unique_ptr<BooleanResult> IsEqualOperator::Evaluate( const EvalKit &kit,
                                                     list<unique_ptr<SymbolicResult>> &&op_results ) const 
{
    ASSERT( op_results.size()==2 );
    // IEEE 754 Equals results in false if an operand is NaS. Not-equals has 
    // no operator class of it's own and is implemented as ¬(==) so will 
    // return true as required.
    for( const unique_ptr<SymbolicResult> &ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_unique<BooleanResult>( false );

    unique_ptr<SymbolicResult> ra = move( op_results.front() );
    unique_ptr<SymbolicResult> rb = move( op_results.back() );

    // For equality, it is sufficient to compare the x links
    // themselves, which have the required uniqueness properties
    // within the full arrowhead model (cf DepthFirstComparisonOperator) .
    bool res = ( ra->GetOnlyXLink() == rb->GetOnlyXLink() );
    return make_unique<BooleanResult>( res );   
}


bool IsEqualOperator::IsCommutative() const
{
    return true;
}


shared_ptr<SymbolExpression> IsEqualOperator::TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const
{
    shared_ptr<SymbolExpression> solution = nullptr;
    ForAllDistinctPairs( list<shared_ptr<SymbolExpression>>{a, b},    // TODO add ForForwardAndReverse for clarity
                         [&](const shared_ptr<SymbolExpression> &first, 
                             const shared_ptr<SymbolExpression> &second)
    {    
        solution = first->TrySolveForToEqual( kit, target, second );
        if( solution )
            LLBreak(); // TODO add early return support to lambda loops via exception
    } );

    return solution;
}


Relationship IsEqualOperator::GetRelationshipWith( shared_ptr<PredicateOperator> other ) const
{
    if( dynamic_pointer_cast<IsGreaterOrEqualOperator>(other) || 
        dynamic_pointer_cast<IsLessOrEqualOperator>(other) ||
        dynamic_pointer_cast<IsSimpleCompareEquivalentOperator>(other))
        return Relationship::IMPLIES;
    
    // Note: CONTRADICTS and TAUTOLOGY are symmetrical could be determined by either predicate - 
    // we choose to let the most "specialised" one do those, to reduce making long lists here.
    return Relationship::NONE; 
}


bool IsEqualOperator::IsCanSubstituteFrom() const
{
    return true;
}


string IsEqualOperator::RenderNF() const
{
    return RenderForMe(a) + " == " + RenderForMe(b);

}


Expression::Precedence IsEqualOperator::GetPrecedenceNF() const
{
    return Precedence::COMPARE;
}


Lazy<BooleanExpression> SYM::operator==( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<IsEqualOperator>( a, b );
}

// ------------------------- (not equal operator) --------------------------

Lazy<BooleanExpression> SYM::operator!=( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return ~(a==b);
}

// ------------------------- DepthFirstComparisonOperator --------------------------

DepthFirstComparisonOperator::DepthFirstComparisonOperator( shared_ptr<SymbolExpression> a_, 
                                                  shared_ptr<SymbolExpression> b_ ) :
    a(a_),
    b(b_)
{
    // Note: not an alldiff, see #429
}    
    

Expression::VariablesRequiringRows DepthFirstComparisonOperator::GetVariablesRequiringRows() const
{
    return GetRequiredVariables();
}


list<shared_ptr<SymbolExpression> *> DepthFirstComparisonOperator::GetSymbolOperandPointers()
{
    return {&a, &b};
}


unique_ptr<BooleanResult> DepthFirstComparisonOperator::Evaluate( const EvalKit &kit,
                                                             list<unique_ptr<SymbolicResult>> &&op_results ) const 
{    
    ASSERT( op_results.size()==2 );
    // IEEE 754 All inequalities result in false if an operand is NaS
    for( const unique_ptr<SymbolicResult> &ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_unique<BooleanResult>( false );

    unique_ptr<SymbolicResult> ra = move( op_results.front() );
    unique_ptr<SymbolicResult> rb = move( op_results.back() );

    // For greater/less, we need to consult the x_tree_db. We use the 
    // depth-first relation.
    SR::DepthFirstRelation dfr( kit.x_tree_db ); 
    Orderable::Diff diff = dfr.Compare3Way(ra->GetOnlyXLink(), rb->GetOnlyXLink() );
    bool res = EvalBoolFromDiff( diff );
    
    return make_unique<BooleanResult>( res );  
}


shared_ptr<SymbolExpression> DepthFirstComparisonOperator::TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const
{ 
    auto ranges = GetRanges();

    shared_ptr<SymbolExpression> solution = nullptr;
    for( auto p : Zip( list<shared_ptr<SymbolExpression>>{a, b}, ranges ) )
    {    
        solution = p.first->TrySolveForToEqual( kit, target, p.second );
        if( solution )
            break;
    }
    
    return solution;
}


Expression::Precedence DepthFirstComparisonOperator::GetPrecedenceNF() const
{
    return Precedence::COMPARE;
}

// ------------------------- IsGreaterOperator --------------------------

shared_ptr<PredicateOperator> IsGreaterOperator::Clone() const
{
    return make_shared<IsGreaterOperator>( a, b );
}
    

bool IsGreaterOperator::EvalBoolFromDiff( Orderable::Diff diff ) const
{
    return diff > 0;
}                    
                                        

list<shared_ptr<SymbolExpression>> IsGreaterOperator::GetRanges() const
{
    return { make_shared<AllGreaterOperator>(b),
             make_shared<AllLessOperator>(a) };
}


Relationship IsGreaterOperator::GetRelationshipWith( shared_ptr<PredicateOperator> other ) const
{
    if( dynamic_pointer_cast<IsGreaterOrEqualOperator>(other) )
        return Relationship::IMPLIES;
    
    if( dynamic_pointer_cast<IsEqualOperator>(other) ||
        dynamic_pointer_cast<IsLessOperator>(other) ||
        dynamic_pointer_cast<IsLessOrEqualOperator>(other) )
        return Relationship::CONTRADICTS;
    
    return Relationship::NONE; 
}


Transitivity IsGreaterOperator::GetTransitivityWith( shared_ptr<PredicateOperator> other ) const
{
    if( dynamic_pointer_cast<IsGreaterOrEqualOperator>(other) || dynamic_pointer_cast<IsGreaterOperator>(other) )
        return Transitivity::FORWARD;

    if( dynamic_pointer_cast<IsLessOrEqualOperator>(other) || dynamic_pointer_cast<IsLessOperator>(other) )
        return Transitivity::REVERSE;
        
    return Transitivity::NONE;
}


string IsGreaterOperator::RenderNF() const
{
    return RenderForMe(a) + " > " + RenderForMe(b);
}


Lazy<BooleanExpression> SYM::operator>( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<IsGreaterOperator>( a, b );
}

// ------------------------- IsLessOperator --------------------------

shared_ptr<PredicateOperator> IsLessOperator::Clone() const
{
    return make_shared<IsLessOperator>( a, b );
}
    

bool IsLessOperator::EvalBoolFromDiff( Orderable::Diff diff ) const
{
    return diff < 0;
}                    
            
                                  
list<shared_ptr<SymbolExpression>> IsLessOperator::GetRanges() const
{
    return { make_shared<AllLessOperator>(b),
             make_shared<AllGreaterOperator>(a) };
}


Relationship IsLessOperator::GetRelationshipWith( shared_ptr<PredicateOperator> other ) const
{
    if( dynamic_pointer_cast<IsLessOrEqualOperator>(other) )
        return Relationship::IMPLIES;
    
    if( dynamic_pointer_cast<IsEqualOperator>(other) ||
        dynamic_pointer_cast<IsGreaterOperator>(other) ||
        dynamic_pointer_cast<IsGreaterOrEqualOperator>(other) )
        return Relationship::CONTRADICTS;
    
    return Relationship::NONE; 
}


Transitivity IsLessOperator::GetTransitivityWith( shared_ptr<PredicateOperator> other ) const
{
    if( dynamic_pointer_cast<IsLessOrEqualOperator>(other) || dynamic_pointer_cast<IsLessOperator>(other))
        return Transitivity::FORWARD;
    
    if( dynamic_pointer_cast<IsGreaterOrEqualOperator>(other) || dynamic_pointer_cast<IsGreaterOperator>(other))
        return Transitivity::REVERSE;
    
    return Transitivity::NONE;
}


string IsLessOperator::RenderNF() const
{
    return RenderForMe(a) + " < " + RenderForMe(b);
}


Lazy<BooleanExpression> SYM::operator<( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<IsLessOperator>( a, b );
}

// ------------------------- IsGreaterOrEqualOperator --------------------------

shared_ptr<PredicateOperator> IsGreaterOrEqualOperator::Clone() const
{
    return make_shared<IsGreaterOrEqualOperator>( a, b );
}
    

bool IsGreaterOrEqualOperator::EvalBoolFromDiff( Orderable::Diff diff ) const
{
    return diff >= 0;
}                    
            
                                  
list<shared_ptr<SymbolExpression>> IsGreaterOrEqualOperator::GetRanges() const
{
    return { make_shared<AllGreaterOrEqualOperator>(b),
             make_shared<AllLessOrEqualOperator>(a) };
}


Relationship IsGreaterOrEqualOperator::GetRelationshipWith( shared_ptr<PredicateOperator> other ) const
{
    if( dynamic_pointer_cast<IsLessOperator>(other) )
        return Relationship::CONTRADICTS;
    
    return Relationship::NONE; 
}


Transitivity IsGreaterOrEqualOperator::GetTransitivityWith( shared_ptr<PredicateOperator> other ) const
{
    if( dynamic_pointer_cast<IsGreaterOrEqualOperator>(other) )
        return Transitivity::FORWARD;

    if( dynamic_pointer_cast<IsLessOrEqualOperator>(other) )
        return Transitivity::REVERSE;
        
    return Transitivity::NONE;
}


string IsGreaterOrEqualOperator::RenderNF() const
{
    return RenderForMe(a) + " >= " + RenderForMe(b);
}


Lazy<BooleanExpression> SYM::operator>=( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<IsGreaterOrEqualOperator>( a, b );
}

// ------------------------- IsLessOrEqualOperator --------------------------

shared_ptr<PredicateOperator> IsLessOrEqualOperator::Clone() const
{
    return make_shared<IsLessOrEqualOperator>( a, b );
}
    

bool IsLessOrEqualOperator::EvalBoolFromDiff( Orderable::Diff diff ) const
{
    return diff <= 0;
}                    
            
                                  
list<shared_ptr<SymbolExpression>> IsLessOrEqualOperator::GetRanges() const
{
    return { make_shared<AllLessOrEqualOperator>(b),
             make_shared<AllGreaterOrEqualOperator>(a) };
}


Relationship IsLessOrEqualOperator::GetRelationshipWith( shared_ptr<PredicateOperator> other ) const
{
    if( dynamic_pointer_cast<IsGreaterOperator>(other) )
        return Relationship::CONTRADICTS;
    
    return Relationship::NONE; 
}


Transitivity IsLessOrEqualOperator::GetTransitivityWith( shared_ptr<PredicateOperator> other ) const
{
    if( dynamic_pointer_cast<IsLessOrEqualOperator>(other) )
        return Transitivity::FORWARD;
    
    if( dynamic_pointer_cast<IsGreaterOrEqualOperator>(other) )
        return Transitivity::REVERSE;
    
    return Transitivity::NONE;
}


string IsLessOrEqualOperator::RenderNF() const
{
    return RenderForMe(a) + " <= " + RenderForMe(b);
}


Lazy<BooleanExpression> SYM::operator<=( Lazy<SymbolExpression> a, Lazy<SymbolExpression> b )
{
    return MakeLazy<IsLessOrEqualOperator>( a, b );
}

// ------------------------- IsAllDiffOperator --------------------------

IsAllDiffOperator::IsAllDiffOperator( list< shared_ptr<SymbolExpression> > sa_ ) :
    sa(sa_)
{
    ASSERT( sa.size() >= 2 );
}    
    

shared_ptr<PredicateOperator> IsAllDiffOperator::Clone() const
{
    return make_shared<IsAllDiffOperator>( sa );
}
    

list<shared_ptr<SymbolExpression> *> IsAllDiffOperator::GetSymbolOperandPointers()
{
    list<shared_ptr<SymbolExpression> *> lp;
    for( shared_ptr<SymbolExpression> &r : sa )
        lp.push_back( &r );
    return lp;
}


unique_ptr<BooleanResult> IsAllDiffOperator::Evaluate( const EvalKit &kit,
                                                     list<unique_ptr<SymbolicResult>> &&op_results ) const 
{
    for( const unique_ptr<SymbolicResult> &ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_unique<BooleanResult>( false );
    
    // Note: could be done faster using a set<XLink>
    bool m = true;
    ForAllUnorderedPairs( op_results, [&](const unique_ptr<SymbolicResult> &ra,
                                                    const unique_ptr<SymbolicResult> &rb) 
    {    
        // For equality, it is sufficient to compare the x links
        // themselves, which have the required uniqueness properties
        // within the full arrowhead model (cf DepthFirstComparisonOperator).
        if( ra->GetOnlyXLink() == rb->GetOnlyXLink() )
        {
            m = false;
            LLBreak();
        }
    } );
    return make_unique<BooleanResult>( m );   
}


shared_ptr<SymbolExpression> IsAllDiffOperator::TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const
{      
    list< shared_ptr<SymbolExpression> > e_others = sa;    
    for( shared_ptr<SymbolExpression> e0 : sa )
    {
        e_others.pop_front(); // we will rotate e_others, leaving a gap that should co-incide with e0
        
        // target must differ from all others. So we want ¬ { e1, e2, e3, ... }. Union
        // will act to compose this set if given singles.
        auto r = make_shared<ComplementOperator>( make_shared<UnionOperator>(e_others) );
        shared_ptr<SymbolExpression> solution = e0->TrySolveForToEqual( kit, target, r );
        if( solution )
            return solution;
        
        e_others.push_back( e0 ); // rotating e_others
    }

    return nullptr;
}


bool IsAllDiffOperator::IsCommutative() const
{
    return true; 
}


Relationship IsAllDiffOperator::GetRelationshipWith( shared_ptr<PredicateOperator> other ) const
{
    if( dynamic_pointer_cast<IsEqualOperator>(other) )
        return Relationship::CONTRADICTS;
    
    return Relationship::NONE; 
}


string IsAllDiffOperator::RenderNF() const
{
    list<string> ls;
    for( shared_ptr<SymbolExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return "AllDiff" + Join( ls, ", ", "( ", " )" );
}


Expression::Precedence IsAllDiffOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}

// ------------------------- IsInCategoryOperator --------------------------

IsInCategoryOperator::IsInCategoryOperator( TreePtr<Node> archetype_node_,
                                shared_ptr<SymbolExpression> a_ ) :
    a( a_ ),
    archetype_node( archetype_node_ )
{    
}                                                


shared_ptr<PredicateOperator> IsInCategoryOperator::Clone() const
{
    return make_shared<IsInCategoryOperator>( archetype_node, a );
}
    

list<shared_ptr<SymbolExpression> *> IsInCategoryOperator::GetSymbolOperandPointers()
{
    return { &a };
}


unique_ptr<BooleanResult> IsInCategoryOperator::Evaluate( const EvalKit &kit,
                                                          list<unique_ptr<SymbolicResult>> &&op_results ) const 
{
    ASSERT( op_results.size()==1 );        
    // IEEE 754 Kind-of can be said to be C(a) ∈ C(arch) where C propogates 
    // NaS. Possibly like a < ?
    unique_ptr<SymbolicResult> ra = SoloElementOf(move(op_results));
    if( !ra->IsDefinedAndUnique() )
        return make_unique<BooleanResult>( false );
    
    bool matches = archetype_node->IsSubcategory( *(ra->GetOnlyXLink().GetChildTreePtr()) );
    return make_unique<BooleanResult>( matches );
}


shared_ptr<SYM::SymbolExpression> IsInCategoryOperator::TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const
{
    // Get half-open lacing index ranges
    const list<pair<int, int>> &int_range_list = kit.lacing->GetRangeListForCategory(archetype_node);
    
    // Get specially hacked XLinks that can be used with the category ordering
    AllInCategoryRangeOperator::ExprBoundsList expr_range_list;
    for( pair<int, int> int_range : int_range_list )
    {
		// Half-open range means use minimus for both (minimus compares less than the node it was created from)
        AllInCategoryRangeOperator::ExprBounds expr_range;
        expr_range.first = make_shared<SYM::SymbolConstant>( MakeTreeNode<SR::CategoryRelation::MinimusNode>(int_range.first) );
        expr_range.second = make_shared<SYM::SymbolConstant>( MakeTreeNode<SR::CategoryRelation::MinimusNode>(int_range.second) );        
        expr_range_list.push_back( expr_range );
    }
    TRACE(archetype_node)("\n")(int_range_list)("\n")(expr_range_list)("\n");
    // int_range is a half-open minimax
    auto r = make_shared<AllInCategoryRangeOperator>( move(expr_range_list), true, false );     

    return a->TrySolveForToEqual( kit, target, r );
}                                                                                                                                             
                                              
                                              
Relationship IsInCategoryOperator::GetRelationshipWith( shared_ptr<PredicateOperator> other ) const
{
    if( auto ko_other = dynamic_pointer_cast<IsInCategoryOperator>(other) )
    {
        // For implication, the SUBCATEGORY implies the SUPERCATEGORY
        if( ko_other->GetArchetypeNode()->IsSubcategory( *archetype_node ) ) // like "contains"
            return Relationship::IMPLIES;
            
        // TODO could determine CONTRADICTS by studying the lacings, or by
        // solving, evaluating solution and testing the results (since
        // the range is fixed).
    }
    
    return Relationship::NONE; 
}


Orderable::Diff IsInCategoryOperator::OrderCompare3WayCovariant( const Orderable &right, 
                                                     OrderProperty order_property ) const 
{
    auto &r = GET_THAT_REFERENCE(right);
    //FTRACE(Render())("\n");
    return OrderCompare3Way(*archetype_node, 
                            *r.archetype_node, 
                            order_property);
}  


string IsInCategoryOperator::RenderNF() const
{
    return "CAT<" + archetype_node->GetTypeName() + ">" + a->RenderWithParentheses(); 
}


Expression::Precedence IsInCategoryOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}


TreePtr<Node> IsInCategoryOperator::GetArchetypeNode() const
{
    return archetype_node;
}

// ------------------------- IsChildCollectionSizedOperator --------------------------

IsChildCollectionSizedOperator::IsChildCollectionSizedOperator( TreePtr<Node> archetype_node_,
                                                          vector< Itemiser::Element * >::size_type item_index_, 
                                                          shared_ptr<SymbolExpression> a_,
                                                          int size_ ) :
    archetype_node( archetype_node_ ),
    item_index( item_index_ ),
    a( a_ ),
    size( size_ )
{
    ASSERT( item_index >= 0 );
}    


shared_ptr<PredicateOperator> IsChildCollectionSizedOperator::Clone() const
{
    return make_shared<IsChildCollectionSizedOperator>( archetype_node, item_index, a, size );
}
    

list<shared_ptr<SymbolExpression> *> IsChildCollectionSizedOperator::GetSymbolOperandPointers()
{
    return { &a };
}


unique_ptr<BooleanResult> IsChildCollectionSizedOperator::Evaluate( const EvalKit &kit,
                                                                 list<unique_ptr<SymbolicResult>> &&op_results ) const
{
    ASSERT( op_results.size()==1 );        

    // Evaluate operand and ensure we got an XLink
    unique_ptr<SymbolicResult> ra = SoloElementOf(move(op_results));

    // IEEE 754 Kind-of can be said to be S(a) == S0 where S propogates 
    // NaS. So like ==
    if( !ra->IsDefinedAndUnique() )
        return make_unique<BooleanResult>( false );

    // XLink must match our referee (i.e. be non-strict subtype)
    // If not, we will say that the size was wrong
    if( !archetype_node->IsSubcategory( *(ra->GetOnlyXLink().GetChildTreePtr()) ) )
        return make_unique<BooleanResult>( false ); 
    
    // Itemise the child node of the XLink we got, according to the "schema"
    // of the referee node (note: link number is only valid wrt referee)
    vector< Itemiser::Element * > keyer_items = archetype_node->Itemise( ra->GetOnlyXLink().GetChildTreePtr().get() );   
    ASSERT( item_index < keyer_items.size() );     
    
    // Cast based on assumption that we'll be looking at a collection
    auto p_x_col = dynamic_cast<CollectionInterface *>(keyer_items[item_index]);    
    ASSERT( p_x_col )("item_index didn't lead to a collection");
    
    // Check that the size is as required
    bool res = ( p_x_col->size() == size );
    return make_unique<BooleanResult>( res );
}


Orderable::Diff IsChildCollectionSizedOperator::OrderCompare3WayCovariant( const Orderable &right, 
                                                                  OrderProperty order_property ) const 
{
    auto &r = GET_THAT_REFERENCE(right);
    //FTRACE(Render())("\n");
    if( Diff d1 = OrderCompare3Way(*archetype_node, 
                                   *r.archetype_node, 
                                   order_property) )
        return d1;

    if( int d2 = item_index - r.item_index )
        return d2;

    return size - r.size;
}  


string IsChildCollectionSizedOperator::RenderNF() const
{
    string name = archetype_node->GetTypeName();

    // Not using RenderForMe() because we always want () here
    return "Child<" + 
           name + 
           "@" + 
           to_string(item_index) + 
           ":col size=" +
           to_string(size) + 
           ">" + 
           a->RenderWithParentheses(); 
}


Expression::Precedence IsChildCollectionSizedOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}

// ------------------------- IsSimpleCompareEquivalentOperator --------------------------

IsSimpleCompareEquivalentOperator::IsSimpleCompareEquivalentOperator( shared_ptr<SymbolExpression> a_, 
                                                                      shared_ptr<SymbolExpression> b_ ) :
    a(a_),
    b(b_)
{
}    
    

shared_ptr<PredicateOperator> IsSimpleCompareEquivalentOperator::Clone() const
{
    return make_shared<IsSimpleCompareEquivalentOperator>( a, b );
}
    

list<shared_ptr<SymbolExpression> *> IsSimpleCompareEquivalentOperator::GetSymbolOperandPointers()
{
    return { &a, &b };
}


unique_ptr<BooleanResult> IsSimpleCompareEquivalentOperator::Evaluate( const EvalKit &kit,
                                                                       list<unique_ptr<SymbolicResult>> &&op_results ) const 
{
    // IEEE 754 Kind-of can be said to be E(a) == E(b) where E propagates 
    // NaS. So like ==
    for( const unique_ptr<SymbolicResult> &ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_unique<BooleanResult>( false );

    unique_ptr<SymbolicResult> ra = move( op_results.front() );
    unique_ptr<SymbolicResult> rb = move( op_results.back() );

    Orderable::Diff res = equivalence_relation.Compare3Way( ra->GetOnlyXLink().GetChildTreePtr(), 
                                                            rb->GetOnlyXLink().GetChildTreePtr() );
    return make_unique<BooleanResult>( res == 0 );    
}


shared_ptr<SymbolExpression> IsSimpleCompareEquivalentOperator::TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const
{  
    shared_ptr<SymbolExpression> solution = nullptr;
    ForAllDistinctPairs( list<shared_ptr<SymbolExpression>>{a, b}, 
                         [&](const shared_ptr<SymbolExpression> &first, 
                             const shared_ptr<SymbolExpression> &second)
    {    
        // Simulate multiset::equal_range() with our ordered domain as an
        // ordering in order to get to the set of equivalent elements without
        // having to iterate over the whole domain. We're still gaining entropy
        // here though. It would be faster to get to the range via xlink_table 
        // (because XLink native comparison will be faster than SimpleCompare)
        // but ar might be an arbitrary force, and not in the domain.
        // See #522 #525
        // X, true, X, true simulates equal_range()    
        shared_ptr<SymbolExpression> r = make_shared<AllInSimpleCompareRangeOperator>(second, BoundingRole::MINIMUS, true, 
                                                                                      second, BoundingRole::MAXIMUS, true);
        solution = first->TrySolveForToEqual( kit, target, r );
        if( solution )
            LLBreak();
    } );
    
    return nullptr;
}


bool IsSimpleCompareEquivalentOperator::IsCommutative() const
{
    return true;
}


Transitivity IsSimpleCompareEquivalentOperator::GetTransitivityWith( shared_ptr<PredicateOperator> other ) const
{
    return (bool)dynamic_pointer_cast<IsSimpleCompareEquivalentOperator>(other) ? Transitivity::BIDIRECTIONAL : Transitivity::NONE;
}


string IsSimpleCompareEquivalentOperator::RenderNF() const
{
    return RenderForMe(a) + " ≡ " + RenderForMe(b);
}


Expression::Precedence IsSimpleCompareEquivalentOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}

// ------------------------- IsLocalMatchOperator --------------------------

IsLocalMatchOperator::IsLocalMatchOperator( const Node *pattern_node_,
                                            shared_ptr<SymbolExpression> a_ ) :
    pattern_node( pattern_node_ ),
    a( a_ )
{
}    


shared_ptr<PredicateOperator> IsLocalMatchOperator::Clone() const
{
    return make_shared<IsLocalMatchOperator>( pattern_node, a );
}
    

list<shared_ptr<SymbolExpression> *> IsLocalMatchOperator::GetSymbolOperandPointers()
{
    return { &a };
}


unique_ptr<BooleanResult> IsLocalMatchOperator::Evaluate( const EvalKit &kit,
                                                          list<unique_ptr<SymbolicResult>> &&op_results ) const
{
    ASSERT( op_results.size()==1 );        

    // Evaluate operand and ensure we got an XLink
    unique_ptr<SymbolicResult> ra = SoloElementOf(move(op_results));

    // IEEE 754 Kind-of can be said to be S(a) == S0 where S propogates 
    // NaS. So like ==
    if( !ra->IsDefinedAndUnique() )
        return make_unique<BooleanResult>( false );

    // Use IsLocalMatch on the pattern node 
    bool match = pattern_node->IsLocalMatch( *(ra->GetOnlyXLink().GetChildTreePtr()) );
    return make_unique<BooleanResult>( match );
}


Orderable::Diff IsLocalMatchOperator::OrderCompare3WayCovariant( const Orderable &right, 
                                                             OrderProperty order_property ) const 
{
    auto &r = GET_THAT_REFERENCE(right);
    return OrderCompare3Way( *pattern_node, 
                             *r.pattern_node, 
                             order_property);
}  


string IsLocalMatchOperator::RenderNF() const
{
    string name = pattern_node->GetName();

    // Not using RenderForMe() because we always want () here
    return name + a->RenderWithParentheses(); 
}


Expression::Precedence IsLocalMatchOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}

