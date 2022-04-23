#include "symbol_operators.hpp"

#include "result.hpp"

#include "helpers/flatten.hpp"
#include "node/node.hpp"

using namespace SYM;

// ------------------------- SymbolConstant --------------------------

SymbolConstant::SymbolConstant( SR::XLink xlink_ ) :
    xlink( xlink_ )
{
}


shared_ptr<SymbolResultInterface> SymbolConstant::Evaluate( const EvalKit &kit ) const
{
    return make_shared<SymbolResult>( xlink );
}


shared_ptr<SymbolResultInterface> SymbolConstant::GetValue() const
{
    return make_shared<SymbolResult>( xlink );
}


SR::XLink SymbolConstant::GetOnlyXLink() const
{
    return xlink;
}


Orderable::Result SymbolConstant::OrderCompareLocal( const Orderable *candidate, 
                                                     OrderProperty order_property ) const 
{
    auto c = GET_THAT_POINTER(candidate);

    if( xlink == c->xlink )
        return 0;
    else if( xlink < c->xlink )
        return -1;
    else
        return 1;
}  


string SymbolConstant::Render() const
{
    return xlink.GetName();
}


Expression::Precedence SymbolConstant::GetPrecedence() const
{
    return Precedence::LITERAL;
}

// ------------------------- SymbolVariable --------------------------

SymbolVariable::SymbolVariable( const SR::PatternLink &plink_ ) :
    plink( plink_ )
{
}


set<SR::PatternLink> SymbolVariable::GetRequiredVariables() const
{
    return { plink };
}


shared_ptr<SymbolResultInterface> SymbolVariable::Evaluate( const EvalKit &kit ) const
{
    // This is an ERROR. You could perfectly easily have called GetRequiredVariables(),
    // done a quick set difference and KNOWN that it would come to this.
    ASSERT( kit.hypothesis_links->count(plink) > 0 );
    
    return make_shared<SymbolResult>( kit.hypothesis_links->at(plink) );
}


SR::PatternLink SymbolVariable::GetPatternLink() const
{
    return plink;
}


Orderable::Result SymbolVariable::OrderCompareLocal( const Orderable *candidate, 
                                                     OrderProperty order_property ) const 
{
    auto c = GET_THAT_POINTER(candidate);

    if( plink == c->plink )
        return 0;
    else if( plink < c->plink )
        return -1;
    else
        return 1;
}  


string SymbolVariable::Render() const
{
    return "[" + plink.GetShortName() + "]";
}


Expression::Precedence SymbolVariable::GetPrecedence() const
{
    return Precedence::LITERAL;
}

// ------------------------- ItemiseToSymbolOperator --------------------------

ItemiseToSymbolOperator::ItemiseToSymbolOperator( TreePtr<Node> archetype_node_,
                                                  int item_index_, 
                                                  shared_ptr<SymbolExpression> a_ ) :
    archetype_node( archetype_node_ ),
    item_index( item_index_ ),
    a( a_ )
{
    ASSERT( item_index >= 0 );
}    


list<shared_ptr<SymbolExpression>> ItemiseToSymbolOperator::GetSymbolOperands() const
{
    return {a};
}


shared_ptr<SymbolResultInterface> ItemiseToSymbolOperator::Evaluate( const EvalKit &kit,
                                                            const list<shared_ptr<SymbolResultInterface>> &op_results ) const
{
    ASSERT( op_results.size()==1 );

    // XLink must match our referee (i.e. be non-strict subtype)
    shared_ptr<SymbolResultInterface> ar = OnlyElementOf(op_results);
    if( !ar->IsDefinedAndUnique() )
        return ar;

    if( !archetype_node->IsLocalMatch( ar->GetOnlyXLink().GetChildX().get() ) )
        return make_shared<SymbolResult>(SymbolResult::NOT_A_SYMBOL); // Will not be able to itemise due incompatible type
    
    // Itemise the child node of the XLink we got, according to the "schema"
    // of the referee node (note: link number is only valid wrt referee)
    vector< Itemiser::Element * > keyer_itemised = archetype_node->Itemise( ar->GetOnlyXLink().GetChildX().get() );   
    ASSERT( item_index < keyer_itemised.size() );     
    
    // Extract the item indicated by item_index. 
    return EvalFromItem( ar->GetOnlyXLink(), keyer_itemised[item_index] );
}


Orderable::Result ItemiseToSymbolOperator::OrderCompareLocal( const Orderable *candidate, 
                                                              OrderProperty order_property ) const 
{
    auto c = GET_THAT_POINTER(candidate);
    //FTRACE(Render())("\n");
    Result r1 = OrderCompare(archetype_node.get(), 
                             c->archetype_node.get(), 
                             order_property);
    if( r1 != EQUAL )
        return r1;

    return item_index - c->item_index;
}  


string ItemiseToSymbolOperator::Render() const
{
    string name = archetype_node->GetTypeName();

    // Not using RenderForMe() because we always want () here
    return "Item<" + 
           name + 
           "@" + 
           to_string(item_index) + 
           ":" + 
           GetItemTypeName() + 
           ">" + 
           a->RenderWithParentheses(); 
}


Expression::Precedence ItemiseToSymbolOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}

// ------------------------- ChildOperator --------------------------

shared_ptr<Expression> ChildOperator::TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                            shared_ptr<SymbolExpression> to_equal ) const
{   
    // ParentOperator uniquely inverts all the ChildOperators
    auto a_to_equal = make_shared<ParentOperator>( to_equal );
    return a->TrySolveForToEqual( target, a_to_equal );
}

// ------------------------- ChildSequenceFrontOperator --------------------------

shared_ptr<SymbolResultInterface> ChildSequenceFrontOperator::EvalFromItem( SR::XLink parent_xlink, 
                                                                   Itemiser::Element *item ) const
{
    // Cast based on assumption that we'll be looking at a sequence
    auto p_x_seq = dynamic_cast<SequenceInterface *>(item);    
    ASSERT( p_x_seq )("item_index didn't lead to a sequence");
    
    SR::XLink result_xlink;
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>) or OffEnd if container empty.
    if( p_x_seq->empty() )
        result_xlink = SR::XLink::OffEndXLink; // OffEnd IS allowed in this case
    else
        result_xlink = SR::XLink(parent_xlink.GetChildX(), &(p_x_seq->front()));        
        
    return make_shared<SymbolResult>( result_xlink );
}


string ChildSequenceFrontOperator::GetItemTypeName() const
{
    return "seq front";
}

// ------------------------- ChildSequenceBackOperator --------------------------

shared_ptr<SymbolResultInterface> ChildSequenceBackOperator::EvalFromItem( SR::XLink parent_xlink, 
                                                                  Itemiser::Element *item ) const
{
    // Cast based on assumption that we'll be looking at a sequence
    auto p_x_seq = dynamic_cast<SequenceInterface *>(item);    
    ASSERT( p_x_seq )("item_index didn't lead to a sequence");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>).
    if( p_x_seq->empty() )
        return make_shared<SymbolResult>( SymbolResult::NOT_A_SYMBOL );
    
    auto result_xlink = SR::XLink(parent_xlink.GetChildX(), &(p_x_seq->back()));        
    return make_shared<SymbolResult>( result_xlink );
}


string ChildSequenceBackOperator::GetItemTypeName() const
{
    return "seq back";
}

// ------------------------- ChildCollectionFrontOperator --------------------------

shared_ptr<SymbolResultInterface> ChildCollectionFrontOperator::EvalFromItem( SR::XLink parent_xlink, 
                                                                     Itemiser::Element *item ) const
{
    // Cast based on assumption that we'll be looking at a collection
    auto p_x_col = dynamic_cast<CollectionInterface *>(item);    
    ASSERT( p_x_col )("item_index didn't lead to a collection");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>).
    if( p_x_col->empty() )
        return make_shared<SymbolResult>( SymbolResult::NOT_A_SYMBOL );
    
    auto result_xlink = SR::XLink(parent_xlink.GetChildX(), &*(p_x_col->begin()));        
    return make_shared<SymbolResult>( result_xlink );
}


string ChildCollectionFrontOperator::GetItemTypeName() const
{
    return "col front";
}

// ------------------------- ChildSingularOperator --------------------------

shared_ptr<SymbolResultInterface> ChildSingularOperator::EvalFromItem( SR::XLink parent_xlink, 
                                                              Itemiser::Element *item ) const
{
    // Cast based on assumption that we'll be looking at a singular item
    TreePtrInterface *p_x_singular = dynamic_cast<TreePtrInterface *>(item);
    ASSERT( p_x_singular )("item_index didn't lead to a singular item");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>)
    auto result_xlink = SR::XLink(parent_xlink.GetChildX(), p_x_singular);        
    return make_shared<SymbolResult>( result_xlink );
}


string ChildSingularOperator::GetItemTypeName() const
{
    return "sing";
}

// ------------------------- KnowledgeToSymbolOperator --------------------------

KnowledgeToSymbolOperator::KnowledgeToSymbolOperator( shared_ptr<SymbolExpression> a_ ) :
    a( a_ )
{
    ASSERT(a);
}    


list<shared_ptr<SymbolExpression>> KnowledgeToSymbolOperator::GetSymbolOperands() const
{
    return {a};
}


shared_ptr<SymbolResultInterface> KnowledgeToSymbolOperator::Evaluate( const EvalKit &kit,
                                                              const list<shared_ptr<SymbolResultInterface>> &op_results ) const
{
    // Evaluate operand and ensure we got an XLink
    shared_ptr<SymbolResultInterface> ar = OnlyElementOf(op_results);       

    if( !ar->IsDefinedAndUnique() )
        return ar;
        
    const SR::TheKnowledge::Nugget &nugget( kit.knowledge->GetNugget(ar->GetOnlyXLink()) );   
    SR::XLink result_xlink = EvalXLinkFromNugget( ar->GetOnlyXLink(), nugget );
    if( result_xlink ) 
        return make_shared<SymbolResult>( result_xlink );
    else
        return make_shared<SymbolResult>( SymbolResult::NOT_A_SYMBOL );
}


string KnowledgeToSymbolOperator::Render() const
{
    return GetKnowledgeName() + a->RenderWithParentheses(); 
}


Expression::Precedence KnowledgeToSymbolOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}

// ------------------------- ParentOperator --------------------------
    
SR::XLink ParentOperator::EvalXLinkFromNugget( SR::XLink parent_xlink, 
                                               const SR::TheKnowledge::Nugget &nugget ) const
{
  
    return nugget.parent_xlink;
}


shared_ptr<Expression> ParentOperator::TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                             shared_ptr<SymbolExpression> to_equal ) const
{   
    // AllChildren and Parent are inverse of each other
    auto a_to_equal = make_shared<AllChildrenOperator>( to_equal );
    return a->TrySolveForToEqual( target, a_to_equal );
}


string ParentOperator::GetKnowledgeName() const
{
    return "Parent";
}

// ------------------------- LastDescendantOperator --------------------------
    
SR::XLink LastDescendantOperator::EvalXLinkFromNugget( SR::XLink parent_xlink, 
                                                       const SR::TheKnowledge::Nugget &nugget ) const
{
  
    return nugget.last_descendant_xlink;
}


string LastDescendantOperator::GetKnowledgeName() const
{
    return "LastDescendant";
}

// ------------------------- MyContainerFrontOperator --------------------------
    
SR::XLink MyContainerFrontOperator::EvalXLinkFromNugget( SR::XLink parent_xlink, 
                                                         const SR::TheKnowledge::Nugget &nugget ) const
{
  
    return nugget.my_container_front;
}


string MyContainerFrontOperator::GetKnowledgeName() const
{
    return "MyConFront";
}

// ------------------------- MyContainerBackOperator --------------------------

SR::XLink MyContainerBackOperator::EvalXLinkFromNugget( SR::XLink parent_xlink, 
                                                        const SR::TheKnowledge::Nugget &nugget ) const
{
    return nugget.my_container_back;
}


string MyContainerBackOperator::GetKnowledgeName() const
{
    return "MyConBack";
}

// ------------------------- MySequenceSuccessorOperator --------------------------

SR::XLink MySequenceSuccessorOperator::EvalXLinkFromNugget( SR::XLink parent_xlink, 
                                                            const SR::TheKnowledge::Nugget &nugget ) const
{  
    return nugget.my_sequence_successor;
}


shared_ptr<Expression> MySequenceSuccessorOperator::TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                                          shared_ptr<SymbolExpression> to_equal ) const
{   
    // Predecessor and successors are inverse of each other
    auto a_to_equal = make_shared<MySequencePredecessorOperator>( to_equal );
    return a->TrySolveForToEqual( target, a_to_equal );
}


string MySequenceSuccessorOperator::GetKnowledgeName() const
{
    return "MySeqSuccessor";
}

// ------------------------- MySequencePredecessorOperator --------------------------

SR::XLink MySequencePredecessorOperator::EvalXLinkFromNugget( SR::XLink parent_xlink, 
                                                              const SR::TheKnowledge::Nugget &nugget ) const
{  
    return nugget.my_sequence_predecessor;
}


shared_ptr<Expression> MySequencePredecessorOperator::TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                                            shared_ptr<SymbolExpression> to_equal ) const
{   
    // Predecessor and successors are inverse of each other
    auto a_to_equal = make_shared<MySequenceSuccessorOperator>( to_equal );
    return a->TrySolveForToEqual( target, a_to_equal );
}


string MySequencePredecessorOperator::GetKnowledgeName() const
{
    return "MySeqPredecessor";
}

// ------------------------- AllChildrenOperator --------------------------

AllChildrenOperator::AllChildrenOperator( shared_ptr<SymbolExpression> a_ ) :
    a( a_ )
{
}    


list<shared_ptr<SymbolExpression>> AllChildrenOperator::GetSymbolOperands() const
{
    return {a};
}


shared_ptr<SymbolResultInterface> AllChildrenOperator::Evaluate( const EvalKit &kit,
                                                            const list<shared_ptr<SymbolResultInterface>> &op_results ) const
{
    ASSERT( op_results.size()==1 );

    // XLink must match our referee (i.e. be non-strict subtype)
    shared_ptr<SymbolResultInterface> ar = OnlyElementOf(op_results);
    if( !ar->IsDefinedAndUnique() )
        return ar;

    TreePtr<Node> parent_node = ar->GetOnlyXLink().GetChildX();
    FlattenNode flat( parent_node );

    set<SR::XLink> child_xlinks;
    FOREACH(const TreePtrInterface &child_node, flat )
    {
        SR::XLink child_xlink( parent_node, &child_node);
        child_xlinks.insert( child_xlink );
    }
    
    return make_shared<SymbolSetResult>( child_xlinks );
}


shared_ptr<Expression> AllChildrenOperator::TrySolveForToEqualNT( shared_ptr<Expression> target, 
                                                                  shared_ptr<SymbolExpression> to_equal ) const
{   
    // AllChildren and Parent are inverse of each other
    auto a_to_equal = make_shared<ParentOperator>( to_equal );
    return a->TrySolveForToEqual( target, a_to_equal );
}


string AllChildrenOperator::Render() const
{
    return "AllChildren" + a->RenderWithParentheses(); 
}


Expression::Precedence AllChildrenOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}

