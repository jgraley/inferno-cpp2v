#include "comparison_operators.hpp"
#include "boolean_operators.hpp"
#include "primary_expressions.hpp"

using namespace SYM;

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


shared_ptr<BooleanResult> EqualOperator::Evaluate( const EvalKit &kit,
                                                   const list<shared_ptr<SymbolResult>> &op_results ) const 
{
    ASSERT( op_results.size()==2 );
    for( shared_ptr<SymbolResult> ra : op_results )
        if( ra->cat == SymbolResult::UNDEFINED )
            return make_shared<BooleanResult>( BooleanResult::UNDEFINED );

    shared_ptr<SymbolResult> ra = op_results.front();
    shared_ptr<SymbolResult> rb = op_results.back();

    // For equality, it is sufficient to compare the x links
    // themselves, which have the required uniqueness properties
    // within the full arrowhead model (cf IndexComparisonOperator) .
    if( ra->xlink == rb->xlink )
        return make_shared<BooleanResult>( BooleanResult::TRUE );
    else
        return make_shared<BooleanResult>( BooleanResult::FALSE );        
}


shared_ptr<SymbolExpression> EqualOperator::TrySolveFor( shared_ptr<SymbolVariable> target ) const
{
    shared_ptr<SymbolExpression> dep_op, indep_op;
    for( shared_ptr<SymbolExpression> op : list<shared_ptr<SymbolExpression>>{a, b} )
    {
        if( op->IsIndependentOf( target ) )
            indep_op = op;
        else
            dep_op = op;                  
    }
    
    if( !indep_op || !dep_op )
        return nullptr; // need at least one
                
    // TODO should be "can dep_op be solved to equal indep_op wrt target"? See #466
    if( auto dep_sv = dynamic_pointer_cast<SymbolVariable>( dep_op ) ) 
    {
        // We already know dep_op is not independent of target. If it's also a 
        // SymbolVariable then it must be the target.        
        ASSERT( dep_sv->GetPatternLink()==target->GetPatternLink() );
        return indep_op;
    }
    
    return nullptr;
}



string EqualOperator::Render() const
{
    return RenderForMe(a) + " == " + RenderForMe(b);

}


Expression::Precedence EqualOperator::GetPrecedence() const
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


shared_ptr<BooleanResult> IndexComparisonOperator::Evaluate( const EvalKit &kit,
                                                             const list<shared_ptr<SymbolResult>> &op_results ) const 
{    
    ASSERT( op_results.size()==2 );
    for( shared_ptr<SymbolResult> ra : op_results )
        if( ra->cat == SymbolResult::UNDEFINED )
            return make_shared<BooleanResult>( BooleanResult::UNDEFINED );

    shared_ptr<SymbolResult> ra = op_results.front();
    shared_ptr<SymbolResult> rb = op_results.back();

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
            
                                  
string LessOperator::Render() const
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
            
                                  
string GreaterOrEqualOperator::Render() const
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
            
                                  
string LessOrEqualOperator::Render() const
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


shared_ptr<BooleanResult> AllDiffOperator::Evaluate( const EvalKit &kit,
                                                     const list<shared_ptr<SymbolResult>> &op_results ) const 
{
    for( shared_ptr<SymbolResult> ra : op_results )
        if( ra->cat == SymbolResult::UNDEFINED )
            return make_shared<BooleanResult>( BooleanResult::UNDEFINED );
    
    // Note: could be done faster using a set<XLink>
    BooleanResult::BooleanValue m = BooleanResult::TRUE;
    ForAllCommutativeDistinctPairs( op_results, [&](shared_ptr<SymbolResult> ra,
                                                    shared_ptr<SymbolResult> rb) 
    {    
        // For equality, it is sufficient to compare the x links
        // themselves, which have the required uniqueness properties
        // within the full arrowhead model (cf IndexComparisonOperator).
        if( ra->xlink == rb->xlink )
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


shared_ptr<BooleanResult> KindOfOperator::Evaluate( const EvalKit &kit,
                                                    const list<shared_ptr<SymbolResult>> &op_results ) const 
{
    ASSERT( op_results.size()==1 );        
    shared_ptr<SymbolResult> ra = OnlyElementOf(op_results);
    if( ra->cat == SymbolResult::UNDEFINED )
        return make_shared<BooleanResult>( BooleanResult::UNDEFINED );
    
    bool matches = archetype_node->IsLocalMatch( ra->xlink.GetChildX().get() );
    return make_shared<BooleanResult>( matches ? BooleanResult::TRUE : BooleanResult::FALSE );
}


Orderable::Result KindOfOperator::OrderCompareLocal( const Orderable *candidate, 
                                                     OrderProperty order_property ) const 
{
    ASSERT( candidate );
    auto *c = dynamic_cast<const KindOfOperator *>(candidate);    
    ASSERT(c);
    //FTRACE(Render())("\n");
    return OrderCompare(archetype_node.get(), 
                        c->archetype_node.get(), 
                        order_property);
}  


string KindOfOperator::Render() const
{
    return "KindOf<" + archetype_node->GetTypeName() + ">(" + a->Render() + ")"; 
}


Expression::Precedence KindOfOperator::GetPrecedence() const
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


shared_ptr<BooleanResult> ChildCollectionSizeOperator::Evaluate( const EvalKit &kit,
                                                                 const list<shared_ptr<SymbolResult>> &op_results ) const
{
    ASSERT( op_results.size()==1 );        

    // Evaluate operand and ensure we got an XLink
    shared_ptr<SymbolResult> ra = OnlyElementOf(op_results);

    // Propagate undefined case
    if( ra->cat == SymbolResult::UNDEFINED )
        return make_shared<BooleanResult>( BooleanResult::UNDEFINED );

    // XLink must match our referee (i.e. be non-strict subtype)
    // If not, we will say that the size was wrong
    if( !archetype_node->IsLocalMatch( ra->xlink.GetChildX().get() ) )
        return make_shared<BooleanResult>(BooleanResult::FALSE); 
    
    // Itemise the child node of the XLink we got, according to the "schema"
    // of the referee node (note: link number is only valid wrt referee)
    vector< Itemiser::Element * > keyer_itemised = archetype_node->Itemise( ra->xlink.GetChildX().get() );   
    ASSERT( item_index < keyer_itemised.size() );     
    
    // Cast based on assumption that we'll be looking at a collection
    auto p_x_col = dynamic_cast<CollectionInterface *>(keyer_itemised[item_index]);    
    ASSERT( p_x_col )("item_index didn't lead to a collection");
    
    // Check that the size is as required
    if( p_x_col->size() == size )
        return make_shared<BooleanResult>(BooleanResult::TRUE);
    else
        return make_shared<BooleanResult>(BooleanResult::FALSE);
}


Orderable::Result ChildCollectionSizeOperator::OrderCompareLocal( const Orderable *candidate, 
                                                                  OrderProperty order_property ) const 
{
    ASSERT( candidate );
    auto *c = dynamic_cast<const ChildCollectionSizeOperator *>(candidate);    
    ASSERT(c);
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


string ChildCollectionSizeOperator::Render() const
{
    string name = archetype_node->GetTypeName();

    // Not using RenderForMe() because we always want () here
    return "Item<" + 
           name + 
           "@" + 
           to_string(item_index) + 
           ":col size=" +
           to_string(size) + 
           ">(" + 
           a->Render() + 
           ")"; 
}


Expression::Precedence ChildCollectionSizeOperator::GetPrecedence() const
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


shared_ptr<BooleanResult> EquivalentOperator::Evaluate( const EvalKit &kit,
                                                        const list<shared_ptr<SymbolResult>> &op_results ) const 
{
    for( shared_ptr<SymbolResult> ra : op_results )
        if( ra->cat == SymbolResult::UNDEFINED )
            return make_shared<BooleanResult>( BooleanResult::UNDEFINED );

    shared_ptr<SymbolResult> ra = op_results.front();
    shared_ptr<SymbolResult> rb = op_results.back();

    // For equality, it is sufficient to compare the x links
    // themselves, which have the required uniqueness properties
    // within the full arrowhead model (cf IndexComparisonOperator).
    if( equivalence_relation.Compare(ra->xlink, rb->xlink) == EQUAL  )
        return make_shared<BooleanResult>( BooleanResult::TRUE );
    else
        return make_shared<BooleanResult>( BooleanResult::FALSE );    

}


string EquivalentOperator::Render() const
{
    return "Equivalent(" + a->Render() + ", " + b->Render() + ")";
}


Expression::Precedence EquivalentOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}


// ------------------------- ConditionalOperator --------------------------

ConditionalOperator::ConditionalOperator( shared_ptr<BooleanExpression> a_,
                                          shared_ptr<SymbolExpression> b_,
                                          shared_ptr<SymbolExpression> c_ ) :
    a( a_ ),
    b( b_ ),
    c( c_ )
{
}

    
list<shared_ptr<Expression>> ConditionalOperator::GetOperands() const
{
    return {a, b, c};
}


shared_ptr<SymbolResult> ConditionalOperator::Evaluate( const EvalKit &kit ) const
{
    shared_ptr<BooleanResult> ra = a->Evaluate(kit);   
    switch( ra->value )
    {   
    case BooleanResult::FALSE:
        return c->Evaluate(kit);
    case BooleanResult::UNDEFINED:
        // TODO could evaluate both, and see if they're defined and equal
        // to each other and then return either of them.
        return make_shared<SymbolResult>( SymbolResult::UNDEFINED );
    case BooleanResult::TRUE:
        return b->Evaluate(kit);
    default:
        ASSERTFAIL("Missing case")
    }  
}


string ConditionalOperator::Render() const
{
    return RenderForMe(a) + " ? " + RenderForMe(b) + " : " + RenderForMe(c);
}


Expression::Precedence ConditionalOperator::GetPrecedence() const
{
    return Precedence::CONDITIONAL;
}



