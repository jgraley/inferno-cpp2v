#include "symbol_operators.hpp"

#include "node/node.hpp"
#include "../agents/agent.hpp"

using namespace SYM;

// ------------------------- ItemiseToSymbolOperator --------------------------

ItemiseToSymbolOperator::ItemiseToSymbolOperator( const SR::Agent *ref_agent_,
                                                  int item_index_, 
                                                  shared_ptr<SymbolExpression> a_ ) :
    ref_agent( ref_agent_ ),
    item_index( item_index_ ),
    a( a_ )
{
    ASSERT( item_index >= 0 );
}    


list<shared_ptr<SymbolExpression>> ItemiseToSymbolOperator::GetSymbolOperands() const
{
    return {a};
}


shared_ptr<SymbolResult> ItemiseToSymbolOperator::Evaluate( const EvalKit &kit,
                                                  const list<shared_ptr<SymbolResult>> &op_results ) const
{
    // XLink must match our referee (i.e. be non-strict subtype)
    shared_ptr<SymbolResult> ar = OnlyElementOf(op_results);
    if( !ref_agent->IsLocalMatch( ar->xlink.GetChildX().get() ) )
        return make_shared<SymbolResult>(SR::XLink::UndefinedXLink); // Will not be able to itemise due incompatible type
    
    // Itemise the child node of the XLink we got, according to the "schema"
    // of the referee node (note: link number is only valid wrt referee)
    vector< Itemiser::Element * > keyer_itemised = ref_agent->Itemise( ar->xlink.GetChildX().get() );   
    ASSERT( item_index < keyer_itemised.size() );     
    
    // Extract the item indicated by item_index. 
    SR::XLink result_xlink = EvalXLinkFromItem( ar->xlink, 
                                                keyer_itemised[item_index] );
    
    return make_shared<SymbolResult>( result_xlink );
}


string ItemiseToSymbolOperator::Render() const
{
    string inner_typename = RemoveOuterTemplate( ref_agent->GetTypeName() );

    // Not using RenderForMe() because we always want () here
    return "Item<" + 
           inner_typename + 
           "@" + 
           to_string(item_index) + 
           ":" + 
           GetItemTypeName() + 
           ">(" + 
           a->Render() + 
           ")"; 
}


Expression::Precedence ItemiseToSymbolOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}

// ------------------------- ChildSequenceFrontOperator --------------------------

SR::XLink ChildSequenceFrontOperator::EvalXLinkFromItem( SR::XLink parent_xlink, 
                                                         Itemiser::Element *item ) const
{
    // Cast based on assumption that we'll be looking at a sequence
    auto p_x_seq = dynamic_cast<SequenceInterface *>(item);    
    ASSERT( p_x_seq )("item_index didn't lead to a sequence");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>) or OffEnd if container empty.
    if( p_x_seq->empty() )
        return SR::XLink::OffEndXLink;
    else
        return SR::XLink(parent_xlink.GetChildX(), &(p_x_seq->front()));        
}


string ChildSequenceFrontOperator::GetItemTypeName() const
{
    return "seq front";
}

// ------------------------- ChildSequenceBackOperator --------------------------

SR::XLink ChildSequenceBackOperator::EvalXLinkFromItem( SR::XLink parent_xlink, 
                                                        Itemiser::Element *item ) const
{
    // Cast based on assumption that we'll be looking at a sequence
    auto p_x_seq = dynamic_cast<SequenceInterface *>(item);    
    ASSERT( p_x_seq )("item_index didn't lead to a sequence");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>) or OffEnd if container empty.
    if( p_x_seq->empty() )
        return SR::XLink::UndefinedXLink;
    else
        return SR::XLink(parent_xlink.GetChildX(), &(p_x_seq->back()));        
}


string ChildSequenceBackOperator::GetItemTypeName() const
{
    return "seq back";
}

// ------------------------- ChildCollectionFrontOperator --------------------------

SR::XLink ChildCollectionFrontOperator::EvalXLinkFromItem( SR::XLink parent_xlink, 
                                                           Itemiser::Element *item ) const
{
    // Cast based on assumption that we'll be looking at a collection
    auto p_x_col = dynamic_cast<CollectionInterface *>(item);    
    ASSERT( p_x_col )("item_index didn't lead to a collection");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>) or OffEnd if container empty.
    if( p_x_col->empty() )
        return SR::XLink::UndefinedXLink;
    else
        return SR::XLink(parent_xlink.GetChildX(), &*(p_x_col->begin()));        
}


string ChildCollectionFrontOperator::GetItemTypeName() const
{
    return "col front";
}

// ------------------------- ChildSingularOperator --------------------------

SR::XLink ChildSingularOperator::EvalXLinkFromItem( SR::XLink parent_xlink, 
                                                    Itemiser::Element *item ) const
{
    // Cast based on assumption that we'll be looking at a singular item
    TreePtrInterface *p_x_singular = dynamic_cast<TreePtrInterface *>(item);
    ASSERT( p_x_singular )("item_index didn't lead to a singular item");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>)
    return SR::XLink(parent_xlink.GetChildX(), p_x_singular);        
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


shared_ptr<SymbolResult> KnowledgeToSymbolOperator::Evaluate( const EvalKit &kit,
                                                              const list<shared_ptr<SymbolResult>> &op_results ) const
{
    // Evaluate operand and ensure we got an XLink
    shared_ptr<SymbolResult> ar = OnlyElementOf(op_results);       
    const SR::TheKnowledge::Nugget &nugget( kit.knowledge->GetNugget(ar->xlink) );   
    SR::XLink result_xlink = EvalXLinkFromNugget( ar->xlink, nugget );
    return make_shared<SymbolResult>( result_xlink );
}


string KnowledgeToSymbolOperator::Render() const
{
    // Not using RenderForMe() because we always want () here
    return GetKnowledgeName() + 
           "(" + 
           a->Render() + 
           ")"; 
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


string MySequenceSuccessorOperator::GetKnowledgeName() const
{
    return "MySeqSuccessor";
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
    if( ra->value == BooleanResult::TRUE )
        return b->Evaluate(kit);
    else
        return c->Evaluate(kit);
}


string ConditionalOperator::Render() const
{
    return RenderForMe(a) + " ? " + RenderForMe(b) + " : " + RenderForMe(c);
}


Expression::Precedence ConditionalOperator::GetPrecedence() const
{
    return Precedence::CONDITIONAL;
}


