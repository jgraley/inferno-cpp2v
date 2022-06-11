#include "predicate_operators.hpp"
#include "boolean_operators.hpp"
#include "set_operators.hpp"
#include "result.hpp"
#include "../lacing.hpp"

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
    auto p = shared_ptr<PredicateOperator>(Clone());
    
    list<shared_ptr<SymbolExpression> *> sop = p->GetSymbolOperandPointers();
    for( shared_ptr<SymbolExpression> *s : sop )
    {
        if( OrderCompareEqual( over, *s ) )
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
    
    
IsEqualOperator *IsEqualOperator::Clone() const
{
    return new IsEqualOperator( a, b );
}
    

list<shared_ptr<SymbolExpression>*> IsEqualOperator::GetSymbolOperandPointers()
{
    return {&a, &b};
}


unique_ptr<BooleanResult> IsEqualOperator::Evaluate( const EvalKit &kit,
                                                     list<unique_ptr<SymbolResultInterface>> &&op_results ) const 
{
    ASSERT( op_results.size()==2 );
    // IEEE 754 Equals results in false if an operand is NaS. Not-equals has 
    // no operator class of it's own and is implemented as ¬(==) so will 
    // return true as required.
    for( const unique_ptr<SymbolResultInterface> &ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_unique<BooleanResult>( false );

    unique_ptr<SymbolResultInterface> ra = move( op_results.front() );
    unique_ptr<SymbolResultInterface> rb = move( op_results.back() );

    // For equality, it is sufficient to compare the x links
    // themselves, which have the required uniqueness properties
    // within the full arrowhead model (cf IndexComparisonOperator) .
    bool res = ( ra->GetOnlyXLink() == rb->GetOnlyXLink() );
    return make_unique<BooleanResult>( res );   
}


bool IsEqualOperator::IsCommutative() const
{
    return true;
}


shared_ptr<SymbolExpression> IsEqualOperator::TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const
{
    // This is already an equals operator, so very close to the semantics of
    // SymbolExpression::TrySolveForToEqual() - we just need to try it both ways around
    
    shared_ptr<SymbolExpression> a_solution = a->TrySolveForToEqual( kit, target, b );
    if( a_solution )
        return a_solution;
    
    shared_ptr<SymbolExpression> b_solution = b->TrySolveForToEqual( kit, target, a );
    if( b_solution )
        return b_solution;
    
    return nullptr;
}


Relationship IsEqualOperator::GetRelationshipWith( shared_ptr<PredicateOperator> other ) const
{
    if( dynamic_pointer_cast<IsGreaterOrEqualOperator>(other) || 
        dynamic_pointer_cast<IsLessOrEqualOperator>(other) ||
        dynamic_pointer_cast<IsCouplingEquivalentOperator>(other))
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


Over<BooleanExpression> SYM::operator==( Over<SymbolExpression> a, Over<SymbolExpression> b )
{
    return MakeOver<IsEqualOperator>( a, b );
}

// ------------------------- (not equal operator) --------------------------

Over<BooleanExpression> SYM::operator!=( Over<SymbolExpression> a, Over<SymbolExpression> b )
{
    return ~(a==b);
}

// ------------------------- IndexComparisonOperator --------------------------

IndexComparisonOperator::IndexComparisonOperator( shared_ptr<SymbolExpression> a_, 
                                                  shared_ptr<SymbolExpression> b_ ) :
    a(a_),
    b(b_)
{
    // Note: not an alldiff, see #429
}    
    

Expression::VariablesRequiringNuggets IndexComparisonOperator::GetVariablesRequiringNuggets() const
{
    return GetRequiredVariables();
}


list<shared_ptr<SymbolExpression> *> IndexComparisonOperator::GetSymbolOperandPointers()
{
    return {&a, &b};
}


unique_ptr<BooleanResult> IndexComparisonOperator::Evaluate( const EvalKit &kit,
                                                             list<unique_ptr<SymbolResultInterface>> &&op_results ) const 
{    
    ASSERT( op_results.size()==2 );
    // IEEE 754 All inequalities result in false if an operand is NaS
    for( const unique_ptr<SymbolResultInterface> &ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_unique<BooleanResult>( false );

    unique_ptr<SymbolResultInterface> ra = move( op_results.front() );
    unique_ptr<SymbolResultInterface> rb = move( op_results.back() );

    // For greater/less, we need to consult the knowledge. We use the 
    // overall depth-first ordering.
    const SR::TheKnowledge::Nugget &nugget_a( kit.knowledge->GetNugget(ra->GetOnlyXLink()) );   
    const SR::TheKnowledge::Nugget &nugget_b( kit.knowledge->GetNugget(rb->GetOnlyXLink()) );   
    SR::TheKnowledge::IndexType index_a = nugget_a.depth_first_index;
    SR::TheKnowledge::IndexType index_b = nugget_b.depth_first_index;
    
    bool res = EvalBoolFromIndexes( index_a, index_b );
    return make_unique<BooleanResult>( res );  
}


shared_ptr<SymbolExpression> IndexComparisonOperator::TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const
{ 
    auto ranges = GetRanges();
    
    shared_ptr<SymbolExpression> a_solution = a->TrySolveForToEqual( kit, target, ranges.first );
    if( a_solution )
        return a_solution;
    
    shared_ptr<SymbolExpression> b_solution = b->TrySolveForToEqual( kit, target, ranges.second );
    if( b_solution )
        return b_solution;
    
    return nullptr;
}


Expression::Precedence IndexComparisonOperator::GetPrecedenceNF() const
{
    return Precedence::COMPARE;
}

// ------------------------- IsGreaterOperator --------------------------

IsGreaterOperator *IsGreaterOperator::Clone() const
{
    return new IsGreaterOperator( a, b );
}
    

bool IsGreaterOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                           SR::TheKnowledge::IndexType index_b ) const
{
    return index_a > index_b;
}                    
                                        

pair<shared_ptr<SymbolExpression>, shared_ptr<SymbolExpression>> IsGreaterOperator::GetRanges() const
{
    return make_pair( make_shared<AllGreaterOperator>(b),
                      make_shared<AllLessOperator>(a) );
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


Over<BooleanExpression> SYM::operator>( Over<SymbolExpression> a, Over<SymbolExpression> b )
{
    return MakeOver<IsGreaterOperator>( a, b );
}

// ------------------------- IsLessOperator --------------------------

IsLessOperator *IsLessOperator::Clone() const
{
    return new IsLessOperator( a, b );
}
    

bool IsLessOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                        SR::TheKnowledge::IndexType index_b ) const
{
    return index_a < index_b;
}                    
            
                                  
pair<shared_ptr<SymbolExpression>, shared_ptr<SymbolExpression>> IsLessOperator::GetRanges() const
{
    return make_pair( make_shared<AllLessOperator>(b),
                      make_shared<AllGreaterOperator>(a) );
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


Over<BooleanExpression> SYM::operator<( Over<SymbolExpression> a, Over<SymbolExpression> b )
{
    return MakeOver<IsLessOperator>( a, b );
}

// ------------------------- IsGreaterOrEqualOperator --------------------------

IsGreaterOrEqualOperator *IsGreaterOrEqualOperator::Clone() const
{
    return new IsGreaterOrEqualOperator( a, b );
}
    

bool IsGreaterOrEqualOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                                  SR::TheKnowledge::IndexType index_b ) const
{
    return index_a >= index_b;
}                    
            
                                  
pair<shared_ptr<SymbolExpression>, shared_ptr<SymbolExpression>> IsGreaterOrEqualOperator::GetRanges() const
{
    return make_pair( make_shared<AllGreaterOrEqualOperator>(b),
                      make_shared<AllLessOrEqualOperator>(a) );
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


Over<BooleanExpression> SYM::operator>=( Over<SymbolExpression> a, Over<SymbolExpression> b )
{
    return MakeOver<IsGreaterOrEqualOperator>( a, b );
}

// ------------------------- IsLessOrEqualOperator --------------------------

IsLessOrEqualOperator *IsLessOrEqualOperator::Clone() const
{
    return new IsLessOrEqualOperator( a, b );
}
    

bool IsLessOrEqualOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                               SR::TheKnowledge::IndexType index_b ) const
{
    return index_a <= index_b;
}                    
            
                                  
pair<shared_ptr<SymbolExpression>, shared_ptr<SymbolExpression>> IsLessOrEqualOperator::GetRanges() const
{
    return make_pair( make_shared<AllLessOrEqualOperator>(b),
                      make_shared<AllGreaterOrEqualOperator>(a) );
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


Over<BooleanExpression> SYM::operator<=( Over<SymbolExpression> a, Over<SymbolExpression> b )
{
    return MakeOver<IsLessOrEqualOperator>( a, b );
}

// ------------------------- IsAllDiffOperator --------------------------

IsAllDiffOperator::IsAllDiffOperator( list< shared_ptr<SymbolExpression> > sa_ ) :
    sa(sa_)
{
    ASSERT( sa.size() >= 2 );
}    
    

IsAllDiffOperator *IsAllDiffOperator::Clone() const
{
    return new IsAllDiffOperator( sa );
}
    

list<shared_ptr<SymbolExpression> *> IsAllDiffOperator::GetSymbolOperandPointers()
{
    list<shared_ptr<SymbolExpression> *> lp;
    for( shared_ptr<SymbolExpression> &r : sa )
        lp.push_back( &r );
    return lp;
}


unique_ptr<BooleanResult> IsAllDiffOperator::Evaluate( const EvalKit &kit,
                                                     list<unique_ptr<SymbolResultInterface>> &&op_results ) const 
{
    for( const unique_ptr<SymbolResultInterface> &ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_unique<BooleanResult>( false );
    
    // Note: could be done faster using a set<XLink>
    bool m = true;
    ForAllCommutativeDistinctPairs( op_results, [&](const unique_ptr<SymbolResultInterface> &ra,
                                                    const unique_ptr<SymbolResultInterface> &rb) 
    {    
        // For equality, it is sufficient to compare the x links
        // themselves, which have the required uniqueness properties
        // within the full arrowhead model (cf IndexComparisonOperator).
        if( ra->GetOnlyXLink() == rb->GetOnlyXLink() )
        {
            m = false;
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

// ------------------------- IsKindOfOperator --------------------------

IsKindOfOperator::IsKindOfOperator( TreePtr<Node> archetype_node_,
                                shared_ptr<SymbolExpression> a_ ) :
    a( a_ ),
    archetype_node( archetype_node_ )
{    
}                                                


IsKindOfOperator *IsKindOfOperator::Clone() const
{
    return new IsKindOfOperator( archetype_node, a );
}
    

list<shared_ptr<SymbolExpression> *> IsKindOfOperator::GetSymbolOperandPointers()
{
    return { &a };
}


unique_ptr<BooleanResult> IsKindOfOperator::Evaluate( const EvalKit &kit,
                                                    list<unique_ptr<SymbolResultInterface>> &&op_results ) const 
{
    ASSERT( op_results.size()==1 );        
    // IEEE 754 Kind-of can be said to be C(a) ∈ C(arch) where C propogates 
    // NaS. Possibly like a < ?
    unique_ptr<SymbolResultInterface> ra = OnlyElementOf(move(op_results));
    if( !ra->IsDefinedAndUnique() )
        return make_unique<BooleanResult>( false );
    
    bool matches = archetype_node->IsLocalMatch( ra->GetOnlyXLink().GetChildX().get() );
    return make_unique<BooleanResult>( matches );
}


shared_ptr<SYM::SymbolExpression> IsKindOfOperator::TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const
{
    // Get lacing index range
    const list<pair<int, int>> &int_range_list = kit.knowledge->GetLacing()->GetRangeListForCategory(archetype_node);
    
    // Get specially hacked XLinks that can be used with the category ordering
    CategoryRangeResult::XLinkBoundsList vxlink_range_list;
    for( pair<int, int> int_range : int_range_list )
    {
        // int_range is a half-open minimax
        vxlink_range_list.push_back( make_pair( make_unique<SR::TheKnowledge::CategoryVXLink>(int_range.first),
                                                make_unique<SR::TheKnowledge::CategoryVXLink>(int_range.second) ) );
    }
    TRACE(archetype_node)("\n")(int_range_list)("\n")(vxlink_range_list)("\n");
    
    auto r = make_shared<AllInCategoryRange>( move(vxlink_range_list), true, false );  
    return a->TrySolveForToEqual( kit, target, r );
}                                                                                                                                             
                                              
                                              
Orderable::Result IsKindOfOperator::OrderCompareLocal( const Orderable *candidate, 
                                                     OrderProperty order_property ) const 
{
    auto c = GET_THAT_POINTER(candidate);
    //FTRACE(Render())("\n");
    return OrderCompare(archetype_node.get(), 
                        c->archetype_node.get(), 
                        order_property);
}  


string IsKindOfOperator::RenderNF() const
{
    return "KindOf<" + archetype_node->GetTypeName() + ">" + a->RenderWithParentheses(); 
}


Expression::Precedence IsKindOfOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}


TreePtr<Node> IsKindOfOperator::GetArchetypeNode() const
{
    return archetype_node;
}

// ------------------------- IsCollectionSizedOperator --------------------------

IsCollectionSizedOperator::IsCollectionSizedOperator( TreePtr<Node> archetype_node_,
                                                          int item_index_, 
                                                          shared_ptr<SymbolExpression> a_,
                                                          int size_ ) :
    archetype_node( archetype_node_ ),
    item_index( item_index_ ),
    a( a_ ),
    size( size_ )
{
    ASSERT( item_index >= 0 );
}    


IsCollectionSizedOperator *IsCollectionSizedOperator::Clone() const
{
    return new IsCollectionSizedOperator( archetype_node, item_index, a, size );
}
    

list<shared_ptr<SymbolExpression> *> IsCollectionSizedOperator::GetSymbolOperandPointers()
{
    return { &a };
}


unique_ptr<BooleanResult> IsCollectionSizedOperator::Evaluate( const EvalKit &kit,
                                                                 list<unique_ptr<SymbolResultInterface>> &&op_results ) const
{
    ASSERT( op_results.size()==1 );        

    // Evaluate operand and ensure we got an XLink
    unique_ptr<SymbolResultInterface> ra = OnlyElementOf(move(op_results));

    // IEEE 754 Kind-of can be said to be S(a) == S0 where S propogates 
    // NaS. So like ==
    if( !ra->IsDefinedAndUnique() )
        return make_unique<BooleanResult>( false );

    // XLink must match our referee (i.e. be non-strict subtype)
    // If not, we will say that the size was wrong
    if( !archetype_node->IsLocalMatch( ra->GetOnlyXLink().GetChildX().get() ) )
        return make_unique<BooleanResult>( false ); 
    
    // Itemise the child node of the XLink we got, according to the "schema"
    // of the referee node (note: link number is only valid wrt referee)
    vector< Itemiser::Element * > keyer_itemised = archetype_node->Itemise( ra->GetOnlyXLink().GetChildX().get() );   
    ASSERT( item_index < keyer_itemised.size() );     
    
    // Cast based on assumption that we'll be looking at a collection
    auto p_x_col = dynamic_cast<CollectionInterface *>(keyer_itemised[item_index]);    
    ASSERT( p_x_col )("item_index didn't lead to a collection");
    
    // Check that the size is as required
    bool res = ( p_x_col->size() == size );
    return make_unique<BooleanResult>( res );
}


Orderable::Result IsCollectionSizedOperator::OrderCompareLocal( const Orderable *candidate, 
                                                                  OrderProperty order_property ) const 
{
    auto c = GET_THAT_POINTER(candidate);
    //FTRACE(Render())("\n");
    Result r1 = OrderCompare(archetype_node.get(), 
                             c->archetype_node.get(), 
                             order_property);
    if( r1 != EQUAL )
        return r1;

    if( item_index != c->item_index )
        return item_index - c->item_index;

    return size - c->size;
}  


string IsCollectionSizedOperator::RenderNF() const
{
    string name = archetype_node->GetTypeName();

    // Not using RenderForMe() because we always want () here
    return "Item<" + 
           name + 
           "@" + 
           to_string(item_index) + 
           ":col size=" +
           to_string(size) + 
           ">" + 
           a->RenderWithParentheses(); 
}


Expression::Precedence IsCollectionSizedOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}

// ------------------------- IsCouplingEquivalentOperator --------------------------

IsCouplingEquivalentOperator::IsCouplingEquivalentOperator( shared_ptr<SymbolExpression> a_, 
                                        shared_ptr<SymbolExpression> b_ ) :
    a(a_),
    b(b_)
{
}    
    

IsCouplingEquivalentOperator *IsCouplingEquivalentOperator::Clone() const
{
    return new IsCouplingEquivalentOperator( a, b );
}
    

list<shared_ptr<SymbolExpression> *> IsCouplingEquivalentOperator::GetSymbolOperandPointers()
{
    return { &a, &b };
}


unique_ptr<BooleanResult> IsCouplingEquivalentOperator::Evaluate( const EvalKit &kit,
                                                        list<unique_ptr<SymbolResultInterface>> &&op_results ) const 
{
    // IEEE 754 Kind-of can be said to be E(a) == E(b) where E propagates 
    // NaS. So like ==
    for( const unique_ptr<SymbolResultInterface> &ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_unique<BooleanResult>( false );

    unique_ptr<SymbolResultInterface> ra = move( op_results.front() );
    unique_ptr<SymbolResultInterface> rb = move( op_results.back() );

    // For equality, it is sufficient to compare the x links
    // themselves, which have the required uniqueness properties
    // within the full arrowhead model (cf IndexComparisonOperator).
    bool res = ( equivalence_relation.Compare(ra->GetOnlyXLink(), rb->GetOnlyXLink()) == EQUAL );
    return make_unique<BooleanResult>( res );    
}


shared_ptr<SymbolExpression> IsCouplingEquivalentOperator::TrySolveFor( const SolveKit &kit, shared_ptr<SymbolVariable> target ) const
{  
    shared_ptr<SymbolExpression> rb = make_shared<AllCouplingEquivalentOperator>(b);
    shared_ptr<SymbolExpression> a_solution = a->TrySolveForToEqual( kit, target, rb );
    if( a_solution )
        return a_solution;
    
    shared_ptr<SymbolExpression> ra = make_shared<AllCouplingEquivalentOperator>(a);   
    shared_ptr<SymbolExpression> b_solution = b->TrySolveForToEqual( kit, target, ra );
    if( b_solution )
        return b_solution;
    
    return nullptr;
}


bool IsCouplingEquivalentOperator::IsCommutative() const
{
    return true;
}


Transitivity IsCouplingEquivalentOperator::GetTransitivityWith( shared_ptr<PredicateOperator> other ) const
{
    return (bool)dynamic_pointer_cast<IsCouplingEquivalentOperator>(other) ? Transitivity::BIDIRECTIONAL : Transitivity::NONE;
}


string IsCouplingEquivalentOperator::RenderNF() const
{
    return RenderForMe(a) + " ≡ " + RenderForMe(b);
}


Expression::Precedence IsCouplingEquivalentOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}
