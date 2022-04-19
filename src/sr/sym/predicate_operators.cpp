#include "predicate_operators.hpp"
#include "boolean_operators.hpp"
#include "result.hpp"

using namespace SYM;

// ------------------------- PredicateOperator --------------------------

void PredicateOperator::SetForceResult( weak_ptr<BooleanResult> force_result_ )
{
    force_result = force_result_;
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


shared_ptr<BooleanResult> PredicateOperator::Evaluate( const EvalKit &kit ) const
{
    // Apply forces where specified
    shared_ptr<BooleanResult> locked_result = force_result.lock();
    if( locked_result )
        return locked_result;

    return Parent::Evaluate( kit );
}


shared_ptr<PredicateOperator> PredicateOperator::TryDeriveWith( shared_ptr<PredicateOperator> other ) const
{
    // Try to substitute one variable with another 
    if( IsCanSubstituteFrom() ) 
    {
        list<shared_ptr<SymbolExpression>> my_ops = GetSymbolOperands();
        ASSERT( my_ops.size() == 2 );

        // Try both ways round. 
        if( shared_ptr<PredicateOperator> sub = other->TrySubstitute( my_ops.front(), my_ops.back() ) )
            return sub;

        if( shared_ptr<PredicateOperator> sub = other->TrySubstitute( my_ops.back(), my_ops.front() ) )
            return sub;        
            
        return nullptr;
    }
    
    // Derive the implications of transitive operators
    Transitivity t = GetTransitivityWith( other );
    if( t!=NONE )
    {
        list<shared_ptr<SymbolExpression>> my_ops = GetSymbolOperands();
        ASSERT( my_ops.size() == 2 );
        list<shared_ptr<SymbolExpression>> other_ops = other->GetSymbolOperands();
        ASSERT( other_ops.size() == 2 );
        if( (t==FORWARD || t==BIDIRECTIONAL) && OrderCompare(my_ops.back(), other_ops.front())==EQUAL )
        {
            shared_ptr<PredicateOperator> pnew = TrySubstitute( my_ops.back(), other_ops.back() );    
            ASSERT( pnew );
            return pnew;
        }
        if( (t==REVERSE || t==BIDIRECTIONAL) && OrderCompare(my_ops.back(), other_ops.back())==EQUAL )
        {
            shared_ptr<PredicateOperator> pnew = TrySubstitute( my_ops.back(), other_ops.front() );    
            ASSERT( pnew );
            return pnew;
        }
    }
    
    return nullptr;
}


shared_ptr<PredicateOperator> PredicateOperator::TrySubstitute( shared_ptr<SymbolExpression> over,
                                                                shared_ptr<SymbolExpression> with ) const
{
    auto p = shared_ptr<PredicateOperator>(Clone());
    
    list<shared_ptr<SymbolExpression> *> sop = p->GetSymbolOperandPointers();
    for( shared_ptr<SymbolExpression> *s : sop )
    {
        if( OrderCompare( over, *s ) == EQUAL )
        {
            *s = with;
            return p;
        }
    }
            
    return nullptr;
}                                                                


PredicateOperator::Transitivity PredicateOperator::GetTransitivityWith( shared_ptr<PredicateOperator> other ) const
{
    return NONE;
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


// ------------------------- EqualOperator --------------------------

EqualOperator::EqualOperator( shared_ptr<SymbolExpression> a_, 
                              shared_ptr<SymbolExpression> b_ ) :
    a(a_),
    b(b_)
{
}    
    
    
EqualOperator *EqualOperator::Clone() const
{
    return new EqualOperator( a, b );
}
    

list<shared_ptr<SymbolExpression>*> EqualOperator::GetSymbolOperandPointers()
{
    return {&a, &b};
}


shared_ptr<BooleanResult> EqualOperator::Evaluate( const EvalKit &kit,
                                                   const list<shared_ptr<SymbolResultInterface>> &op_results ) const 
{
    ASSERT( op_results.size()==2 );
    // IEEE 754 Equals results in false if an operand is NaS. Not-equals has 
    // no operator class of it's own and is implemented as ¬(==) so will 
    // return true as required.
    for( shared_ptr<SymbolResultInterface> ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_shared<BooleanResult>( false );

    shared_ptr<SymbolResultInterface> ra = op_results.front();
    shared_ptr<SymbolResultInterface> rb = op_results.back();

    // For equality, it is sufficient to compare the x links
    // themselves, which have the required uniqueness properties
    // within the full arrowhead model (cf IndexComparisonOperator) .
    bool res = ( ra->GetAsXLink() == rb->GetAsXLink() );
    return make_shared<BooleanResult>( res );   
}


bool EqualOperator::IsCommutative() const
{
    return true;
}


shared_ptr<Expression> EqualOperator::TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                            shared_ptr<BooleanExpression> to_equal ) const
{
    // Can only deal with to_equal==TRUE
    auto to_equal_bc = dynamic_pointer_cast<BooleanConstant>( to_equal );
    if( !to_equal_bc || !to_equal_bc->GetAsBool() )
        return nullptr;
        
    // This is already an equals operator, so very close to the semantics of
    // TrySolveForToEqual() - we just need to try it both ways around
    
    shared_ptr<Expression> a_solution = a->TrySolveForToEqual( target, b );
    if( a_solution )
        return a_solution;
    
    shared_ptr<Expression> b_solution = b->TrySolveForToEqual( target, a );
    if( b_solution )
        return b_solution;
    
    return nullptr;
}


bool EqualOperator::IsCanSubstituteFrom() const
{
    return true;
}


string EqualOperator::RenderNF() const
{
    return RenderForMe(a) + " == " + RenderForMe(b);

}


Expression::Precedence EqualOperator::GetPrecedenceNF() const
{
    return Precedence::COMPARE;
}


Over<BooleanExpression> SYM::operator==( Over<SymbolExpression> a, Over<SymbolExpression> b )
{
    return MakeOver<EqualOperator>( a, b );
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
    

list<shared_ptr<SymbolExpression> *> IndexComparisonOperator::GetSymbolOperandPointers()
{
    return {&a, &b};
}


shared_ptr<BooleanResult> IndexComparisonOperator::Evaluate( const EvalKit &kit,
                                                                      const list<shared_ptr<SymbolResultInterface>> &op_results ) const 
{    
    ASSERT( op_results.size()==2 );
    // IEEE 754 All inequalities result in false if an operand is NaS
    for( shared_ptr<SymbolResultInterface> ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_shared<BooleanResult>( false );

    shared_ptr<SymbolResultInterface> ra = op_results.front();
    shared_ptr<SymbolResultInterface> rb = op_results.back();

    // For greater/less, we need to consult the knowledge. We use the 
    // overall depth-first ordering.
    const SR::TheKnowledge::Nugget &nugget_a( kit.knowledge->GetNugget(ra->GetAsXLink()) );   
    const SR::TheKnowledge::Nugget &nugget_b( kit.knowledge->GetNugget(rb->GetAsXLink()) );   
    SR::TheKnowledge::IndexType index_a = nugget_a.depth_first_index;
    SR::TheKnowledge::IndexType index_b = nugget_b.depth_first_index;
    
    bool res = EvalBoolFromIndexes( index_a, index_b );
    return make_shared<BooleanResult>( res );  
}


Expression::Precedence IndexComparisonOperator::GetPrecedenceNF() const
{
    return Precedence::COMPARE;
}

// ------------------------- GreaterOperator --------------------------

GreaterOperator *GreaterOperator::Clone() const
{
    return new GreaterOperator( a, b );
}
    

bool GreaterOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                           SR::TheKnowledge::IndexType index_b ) const
{
    return index_a > index_b;
}                    
            
                                  
PredicateOperator::Transitivity GreaterOperator::GetTransitivityWith( shared_ptr<PredicateOperator> other ) const
{
    if( dynamic_pointer_cast<GreaterOrEqualOperator>(other) || dynamic_pointer_cast<GreaterOperator>(other))
        return FORWARD;

    if( dynamic_pointer_cast<LessOrEqualOperator>(other) || dynamic_pointer_cast<LessOperator>(other))
        return REVERSE;
        
    return NONE;
}


string GreaterOperator::RenderNF() const
{
    return RenderForMe(a) + " > " + RenderForMe(b);
}


Over<BooleanExpression> SYM::operator>( Over<SymbolExpression> a, Over<SymbolExpression> b )
{
    return MakeOver<GreaterOperator>( a, b );
}

// ------------------------- LessOperator --------------------------

LessOperator *LessOperator::Clone() const
{
    return new LessOperator( a, b );
}
    

bool LessOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                        SR::TheKnowledge::IndexType index_b ) const
{
    return index_a < index_b;
}                    
            
                                  
PredicateOperator::Transitivity LessOperator::GetTransitivityWith( shared_ptr<PredicateOperator> other ) const
{
    if( dynamic_pointer_cast<LessOrEqualOperator>(other) || dynamic_pointer_cast<LessOperator>(other))
        return FORWARD;
    
    if( dynamic_pointer_cast<GreaterOrEqualOperator>(other) || dynamic_pointer_cast<GreaterOperator>(other))
        return REVERSE;
    
    return NONE;
}


string LessOperator::RenderNF() const
{
    return RenderForMe(a) + " < " + RenderForMe(b);
}


Over<BooleanExpression> SYM::operator<( Over<SymbolExpression> a, Over<SymbolExpression> b )
{
    return MakeOver<LessOperator>( a, b );
}

// ------------------------- GreaterOrEqualOperator --------------------------

GreaterOrEqualOperator *GreaterOrEqualOperator::Clone() const
{
    return new GreaterOrEqualOperator( a, b );
}
    

bool GreaterOrEqualOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                                  SR::TheKnowledge::IndexType index_b ) const
{
    return index_a >= index_b;
}                    
            
                                  
PredicateOperator::Transitivity GreaterOrEqualOperator::GetTransitivityWith( shared_ptr<PredicateOperator> other ) const
{
    if( dynamic_pointer_cast<GreaterOrEqualOperator>(other) )
        return FORWARD;

    if( dynamic_pointer_cast<LessOrEqualOperator>(other) )
        return REVERSE;
        
    return NONE;
}


string GreaterOrEqualOperator::RenderNF() const
{
    return RenderForMe(a) + " >= " + RenderForMe(b);
}


Over<BooleanExpression> SYM::operator>=( Over<SymbolExpression> a, Over<SymbolExpression> b )
{
    return MakeOver<GreaterOrEqualOperator>( a, b );
}

// ------------------------- LessOrEqualOperator --------------------------

LessOrEqualOperator *LessOrEqualOperator::Clone() const
{
    return new LessOrEqualOperator( a, b );
}
    

bool LessOrEqualOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                               SR::TheKnowledge::IndexType index_b ) const
{
    return index_a <= index_b;
}                    
            
                                  
PredicateOperator::Transitivity LessOrEqualOperator::GetTransitivityWith( shared_ptr<PredicateOperator> other ) const
{
    if( dynamic_pointer_cast<LessOrEqualOperator>(other) )
        return FORWARD;
    
    if( dynamic_pointer_cast<GreaterOrEqualOperator>(other) )
        return REVERSE;
    
    return NONE;
}


string LessOrEqualOperator::RenderNF() const
{
    return RenderForMe(a) + " <= " + RenderForMe(b);
}


Over<BooleanExpression> SYM::operator<=( Over<SymbolExpression> a, Over<SymbolExpression> b )
{
    return MakeOver<LessOrEqualOperator>( a, b );
}

// ------------------------- AllDiffOperator --------------------------

AllDiffOperator::AllDiffOperator( list< shared_ptr<SymbolExpression> > sa_ ) :
    sa(sa_)
{
    ASSERT( sa.size() >= 2 );
}    
    

AllDiffOperator *AllDiffOperator::Clone() const
{
    return new AllDiffOperator( sa );
}
    

list<shared_ptr<SymbolExpression> *> AllDiffOperator::GetSymbolOperandPointers()
{
    list<shared_ptr<SymbolExpression> *> lp;
    for( shared_ptr<SymbolExpression> &r : sa )
        lp.push_back( &r );
    return lp;
}


shared_ptr<BooleanResult> AllDiffOperator::Evaluate( const EvalKit &kit,
                                                     const list<shared_ptr<SymbolResultInterface>> &op_results ) const 
{
    for( shared_ptr<SymbolResultInterface> ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_shared<BooleanResult>( false );
    
    // Note: could be done faster using a set<XLink>
    bool m = true;
    ForAllCommutativeDistinctPairs( op_results, [&](shared_ptr<SymbolResultInterface> ra,
                                                    shared_ptr<SymbolResultInterface> rb) 
    {    
        // For equality, it is sufficient to compare the x links
        // themselves, which have the required uniqueness properties
        // within the full arrowhead model (cf IndexComparisonOperator).
        if( ra->GetAsXLink() == rb->GetAsXLink() )
        {
            m = false;
        }
    } );
    return make_shared<BooleanResult>( m );   
}


bool AllDiffOperator::IsCommutative() const
{
    return true; 
}


string AllDiffOperator::RenderNF() const
{
    list<string> ls;
    for( shared_ptr<SymbolExpression> a : sa )
        ls.push_back( RenderForMe(a) );
    return "AllDiff" + Join( ls, ", ", "( ", " )" );
}


Expression::Precedence AllDiffOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}

// ------------------------- KindOfOperator --------------------------

KindOfOperator::KindOfOperator( TreePtr<Node> archetype_node_,
                                shared_ptr<SymbolExpression> a_ ) :
    a( a_ ),
    archetype_node( archetype_node_ )
{    
}                                                


KindOfOperator *KindOfOperator::Clone() const
{
    return new KindOfOperator( archetype_node, a );
}
    

list<shared_ptr<SymbolExpression> *> KindOfOperator::GetSymbolOperandPointers()
{
    return { &a };
}


shared_ptr<BooleanResult> KindOfOperator::Evaluate( const EvalKit &kit,
                                                    const list<shared_ptr<SymbolResultInterface>> &op_results ) const 
{
    ASSERT( op_results.size()==1 );        
    // IEEE 754 Kind-of can be said to be C(a) ∈ C(arch) where C propogates 
    // NaS. Possibly like a < ?
    shared_ptr<SymbolResultInterface> ra = OnlyElementOf(op_results);
    if( !ra->IsDefinedAndUnique() )
        return make_shared<BooleanResult>( false );
    
    bool matches = archetype_node->IsLocalMatch( ra->GetAsXLink().GetChildX().get() );
    return make_shared<BooleanResult>( matches );
}


Orderable::Result KindOfOperator::OrderCompareLocal( const Orderable *candidate, 
                                                     OrderProperty order_property ) const 
{
    auto c = GET_THAT_POINTER(candidate);
    //FTRACE(Render())("\n");
    return OrderCompare(archetype_node.get(), 
                        c->archetype_node.get(), 
                        order_property);
}  


string KindOfOperator::RenderNF() const
{
    return "KindOf<" + archetype_node->GetTypeName() + ">" + a->RenderWithParentheses(); 
}


Expression::Precedence KindOfOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}

// ------------------------- ChildCollectionSizeOperator --------------------------

ChildCollectionSizeOperator::ChildCollectionSizeOperator( TreePtr<Node> archetype_node_,
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


ChildCollectionSizeOperator *ChildCollectionSizeOperator::Clone() const
{
    return new ChildCollectionSizeOperator( archetype_node, item_index, a, size );
}
    

list<shared_ptr<SymbolExpression> *> ChildCollectionSizeOperator::GetSymbolOperandPointers()
{
    return { &a };
}


shared_ptr<BooleanResult> ChildCollectionSizeOperator::Evaluate( const EvalKit &kit,
                                                                 const list<shared_ptr<SymbolResultInterface>> &op_results ) const
{
    ASSERT( op_results.size()==1 );        

    // Evaluate operand and ensure we got an XLink
    shared_ptr<SymbolResultInterface> ra = OnlyElementOf(op_results);

    // IEEE 754 Kind-of can be said to be S(a) == S0 where S propogates 
    // NaS. So like ==
    if( !ra->IsDefinedAndUnique() )
        return make_shared<BooleanResult>( false );

    // XLink must match our referee (i.e. be non-strict subtype)
    // If not, we will say that the size was wrong
    if( !archetype_node->IsLocalMatch( ra->GetAsXLink().GetChildX().get() ) )
        return make_shared<BooleanResult>( false ); 
    
    // Itemise the child node of the XLink we got, according to the "schema"
    // of the referee node (note: link number is only valid wrt referee)
    vector< Itemiser::Element * > keyer_itemised = archetype_node->Itemise( ra->GetAsXLink().GetChildX().get() );   
    ASSERT( item_index < keyer_itemised.size() );     
    
    // Cast based on assumption that we'll be looking at a collection
    auto p_x_col = dynamic_cast<CollectionInterface *>(keyer_itemised[item_index]);    
    ASSERT( p_x_col )("item_index didn't lead to a collection");
    
    // Check that the size is as required
    bool res = ( p_x_col->size() == size );
    return make_shared<BooleanResult>( res );
}


Orderable::Result ChildCollectionSizeOperator::OrderCompareLocal( const Orderable *candidate, 
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


string ChildCollectionSizeOperator::RenderNF() const
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


Expression::Precedence ChildCollectionSizeOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}

// ------------------------- EquivalentOperator --------------------------

EquivalentOperator::EquivalentOperator( shared_ptr<SymbolExpression> a_, 
                                        shared_ptr<SymbolExpression> b_ ) :
    a(a_),
    b(b_)
{
}    
    

EquivalentOperator *EquivalentOperator::Clone() const
{
    return new EquivalentOperator( a, b );
}
    

list<shared_ptr<SymbolExpression> *> EquivalentOperator::GetSymbolOperandPointers()
{
    return { &a, &b };
}


shared_ptr<BooleanResult> EquivalentOperator::Evaluate( const EvalKit &kit,
                                                        const list<shared_ptr<SymbolResultInterface>> &op_results ) const 
{
    // IEEE 754 Kind-of can be said to be E(a) == E(b) where E propogates 
    // NaS. So like ==
    for( shared_ptr<SymbolResultInterface> ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_shared<BooleanResult>( false );

    shared_ptr<SymbolResultInterface> ra = op_results.front();
    shared_ptr<SymbolResultInterface> rb = op_results.back();

    // For equality, it is sufficient to compare the x links
    // themselves, which have the required uniqueness properties
    // within the full arrowhead model (cf IndexComparisonOperator).
    bool res = ( equivalence_relation.Compare(ra->GetAsXLink(), rb->GetAsXLink()) == EQUAL );
    return make_shared<BooleanResult>( res );    
}


bool EquivalentOperator::IsCommutative() const
{
    return true;
}


PredicateOperator::Transitivity EquivalentOperator::GetTransitivityWith( shared_ptr<PredicateOperator> other ) const
{
    return (bool)dynamic_pointer_cast<EquivalentOperator>(other) ? BIDIRECTIONAL : NONE;
}


string EquivalentOperator::RenderNF() const
{
    return RenderForMe(a) + " ≡ " + RenderForMe(b);
}


Expression::Precedence EquivalentOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}
