#include "predicate_operators.hpp"
#include "boolean_operators.hpp"
#include "primary_expressions.hpp"
#include "result.hpp"

using namespace SYM;

// ------------------------- PredicateOperator --------------------------

void PredicateOperator::SetForceResult( weak_ptr<BooleanResultInterface> force_result_ )
{
    force_result = force_result_;
}


void PredicateOperator::SetForceRender( weak_ptr<string> force_render_ )
{
    force_render = force_render_;
}


shared_ptr<BooleanResultInterface> PredicateOperator::Evaluate( const EvalKit &kit,
                                                                const list<shared_ptr<SymbolResultInterface>> &op_results ) const
{
    // Apply forces where specified
    shared_ptr<BooleanResultInterface> locked_result = force_result.lock();
    if( locked_result )
        return locked_result;

    return EvaluateNF( kit, op_results );
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
    

list<shared_ptr<SymbolExpression>> EqualOperator::GetSymbolOperands() const
{
    return {a, b};
}


shared_ptr<BooleanResultInterface> EqualOperator::EvaluateNF( const EvalKit &kit,
                                                   const list<shared_ptr<SymbolResultInterface>> &op_results ) const 
{
    ASSERT( op_results.size()==2 );
    for( shared_ptr<SymbolResultInterface> ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_shared<BooleanResult>( BooleanResult::UNDEFINED );

    shared_ptr<SymbolResultInterface> ra = op_results.front();
    shared_ptr<SymbolResultInterface> rb = op_results.back();

    // For equality, it is sufficient to compare the x links
    // themselves, which have the required uniqueness properties
    // within the full arrowhead model (cf IndexComparisonOperator) .
    bool res = ( ra->GetAsXLink() == rb->GetAsXLink() );
    return make_shared<BooleanResult>( BooleanResult::DEFINED, res );   
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
    

list<shared_ptr<SymbolExpression>> IndexComparisonOperator::GetSymbolOperands() const
{
    return {a, b};
}


shared_ptr<BooleanResultInterface> IndexComparisonOperator::EvaluateNF( const EvalKit &kit,
                                                             const list<shared_ptr<SymbolResultInterface>> &op_results ) const 
{    
    ASSERT( op_results.size()==2 );
    for( shared_ptr<SymbolResultInterface> ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_shared<BooleanResult>( BooleanResult::UNDEFINED );

    shared_ptr<SymbolResultInterface> ra = op_results.front();
    shared_ptr<SymbolResultInterface> rb = op_results.back();

    // For greater/less, we need to consult the knowledge. We use the 
    // overall depth-first ordering.
    const SR::TheKnowledge::Nugget &nugget_a( kit.knowledge->GetNugget(ra->GetAsXLink()) );   
    const SR::TheKnowledge::Nugget &nugget_b( kit.knowledge->GetNugget(rb->GetAsXLink()) );   
    SR::TheKnowledge::IndexType index_a = nugget_a.depth_first_index;
    SR::TheKnowledge::IndexType index_b = nugget_b.depth_first_index;
    
    bool res = EvalBoolFromIndexes( index_a, index_b );
    return make_shared<BooleanResult>( BooleanResult::DEFINED, res );  
}


Expression::Precedence IndexComparisonOperator::GetPrecedenceNF() const
{
    return Precedence::COMPARE;
}

// ------------------------- GreaterOperator --------------------------

bool GreaterOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                           SR::TheKnowledge::IndexType index_b ) const
{
    return index_a > index_b;
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

bool LessOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                        SR::TheKnowledge::IndexType index_b ) const
{
    return index_a < index_b;
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

bool GreaterOrEqualOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                                  SR::TheKnowledge::IndexType index_b ) const
{
    return index_a >= index_b;
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

bool LessOrEqualOperator::EvalBoolFromIndexes( SR::TheKnowledge::IndexType index_a,
                                               SR::TheKnowledge::IndexType index_b ) const
{
    return index_a <= index_b;
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
    

list<shared_ptr<SymbolExpression>> AllDiffOperator::GetSymbolOperands() const
{
    return sa;
}


shared_ptr<BooleanResultInterface> AllDiffOperator::EvaluateNF( const EvalKit &kit,
                                                     const list<shared_ptr<SymbolResultInterface>> &op_results ) const 
{
    for( shared_ptr<SymbolResultInterface> ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_shared<BooleanResult>( BooleanResult::UNDEFINED );
    
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
    return make_shared<BooleanResult>( BooleanResult::DEFINED, m );   
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


list<shared_ptr<SymbolExpression>> KindOfOperator::GetSymbolOperands() const
{
    return { a };
}


shared_ptr<BooleanResultInterface> KindOfOperator::EvaluateNF( const EvalKit &kit,
                                                    const list<shared_ptr<SymbolResultInterface>> &op_results ) const 
{
    ASSERT( op_results.size()==1 );        
    shared_ptr<SymbolResultInterface> ra = OnlyElementOf(op_results);
    if( !ra->IsDefinedAndUnique() )
        return make_shared<BooleanResult>( BooleanResult::UNDEFINED );
    
    bool matches = archetype_node->IsLocalMatch( ra->GetAsXLink().GetChildX().get() );
    return make_shared<BooleanResult>( BooleanResult::DEFINED, matches );
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


list<shared_ptr<SymbolExpression>> ChildCollectionSizeOperator::GetSymbolOperands() const
{
    return {a};
}


shared_ptr<BooleanResultInterface> ChildCollectionSizeOperator::EvaluateNF( const EvalKit &kit,
                                                                 const list<shared_ptr<SymbolResultInterface>> &op_results ) const
{
    ASSERT( op_results.size()==1 );        

    // Evaluate operand and ensure we got an XLink
    shared_ptr<SymbolResultInterface> ra = OnlyElementOf(op_results);

    // Propagate undefined case
    if( !ra->IsDefinedAndUnique() )
        return make_shared<BooleanResult>( BooleanResult::UNDEFINED );

    // XLink must match our referee (i.e. be non-strict subtype)
    // If not, we will say that the size was wrong
    if( !archetype_node->IsLocalMatch( ra->GetAsXLink().GetChildX().get() ) )
        return make_shared<BooleanResult>( BooleanResult::DEFINED, false ); 
    
    // Itemise the child node of the XLink we got, according to the "schema"
    // of the referee node (note: link number is only valid wrt referee)
    vector< Itemiser::Element * > keyer_itemised = archetype_node->Itemise( ra->GetAsXLink().GetChildX().get() );   
    ASSERT( item_index < keyer_itemised.size() );     
    
    // Cast based on assumption that we'll be looking at a collection
    auto p_x_col = dynamic_cast<CollectionInterface *>(keyer_itemised[item_index]);    
    ASSERT( p_x_col )("item_index didn't lead to a collection");
    
    // Check that the size is as required
    bool res = ( p_x_col->size() == size );
    return make_shared<BooleanResult>( BooleanResult::DEFINED, res );
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
    

list<shared_ptr<SymbolExpression>> EquivalentOperator::GetSymbolOperands() const
{
    return {a, b};
}


shared_ptr<BooleanResultInterface> EquivalentOperator::EvaluateNF( const EvalKit &kit,
                                                        const list<shared_ptr<SymbolResultInterface>> &op_results ) const 
{
    for( shared_ptr<SymbolResultInterface> ra : op_results )
        if( !ra->IsDefinedAndUnique() )
            return make_shared<BooleanResult>( BooleanResult::UNDEFINED );

    shared_ptr<SymbolResultInterface> ra = op_results.front();
    shared_ptr<SymbolResultInterface> rb = op_results.back();

    // For equality, it is sufficient to compare the x links
    // themselves, which have the required uniqueness properties
    // within the full arrowhead model (cf IndexComparisonOperator).
    bool res = ( equivalence_relation.Compare(ra->GetAsXLink(), rb->GetAsXLink()) == EQUAL );
    return make_shared<BooleanResult>( BooleanResult::DEFINED, res );    
}


string EquivalentOperator::RenderNF() const
{
    return RenderForMe(a) + " ≡ " + RenderForMe(b);
}


Expression::Precedence EquivalentOperator::GetPrecedenceNF() const
{
    return Precedence::PREFIX;
}
