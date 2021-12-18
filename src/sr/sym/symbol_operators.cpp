#include "symbol_operators.hpp"

#include "node/node.hpp"
#include "../agents/agent.hpp"

using namespace SYM;

// ------------------------- ChildOperator --------------------------

ChildOperator::ChildOperator( const SR::Agent *ref_agent_,
                                              int item_index_, 
                                              shared_ptr<SymbolExpression> a_ ) :
    ref_agent( ref_agent_ ),
    item_index( item_index_ ),
    a( a_ )
{
    ASSERT( item_index >= 0 );
}    


set<shared_ptr<Expression>> ChildOperator::GetOperands() const
{
    set<shared_ptr<Expression>> ops;
    ops.insert(a);
    return ops;
}


unique_ptr<SymbolResult> ChildOperator::Evaluate( const EvalKit &kit ) const
{
    // Evaluate operand and ensure we got an XLink
    unique_ptr<SymbolResult> ar = a->Evaluate( kit );
    if( !ar->xlink )
        return make_unique<SymbolResult>();

    // XLink must match our referee (i.e. be non-strict subtype)
    if( !ref_agent->IsLocalMatch( ar->xlink.GetChildX().get() ) )
        return make_unique<SymbolResult>(); // Will not be able to itemise due incompatible type
    
    // Itemise the child node of the XLink we got, according to the "schema"
    // of the referee node (note: link number is only valid wrt referee)
    vector< Itemiser::Element * > keyer_itemised = ref_agent->Itemise( ar->xlink.GetChildX().get() );   
    ASSERT( item_index < keyer_itemised.size() );     
    
    // Extract the item indicated by item_index. 
    SR::XLink result_xlink = XLinkFromItem( ar->xlink, 
                                            keyer_itemised[item_index] );
    
    return make_unique<SymbolResult>( result_xlink );
}


string ChildOperator::Render() const
{
    string inner_typename = RemoveOuterTemplate( ref_agent->GetTypeName() );

    // Not using RenderForMe() because we always want () here
    return "Item<" + 
           inner_typename + 
           "@" + 
           to_string(item_index) + 
           ":" + 
           GetItemType() + 
           ">(" + 
           a->Render() + 
           ")"; 
}


Expression::Precedence ChildOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}

// ------------------------- SequenceFrontChildOperator --------------------------

SequenceFrontChildOperator::SequenceFrontChildOperator( const SR::Agent *ref_agent,
                                                        int item_index, 
                                                        shared_ptr<SymbolExpression> a ) :
    ChildOperator( ref_agent, item_index, a )
{
}
    

SR::XLink SequenceFrontChildOperator::XLinkFromItem( SR::XLink parent_xlink, 
                                                     Itemiser::Element *item ) const
{
    // Cast based on assumption that we'll be looking at a sequence
    auto p_x_seq = dynamic_cast<SequenceInterface *>(item);    
    ASSERT( p_x_seq )("item_index didn't lead to a sequence");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>)
    return SR::XLink(parent_xlink.GetChildX(), &(p_x_seq->front()));        
}


string SequenceFrontChildOperator::GetItemType() const
{
    return "seq front";
}

// ------------------------- SequenceBackChildOperator --------------------------

SequenceBackChildOperator::SequenceBackChildOperator( const SR::Agent *ref_agent,
                                                        int item_index, 
                                                        shared_ptr<SymbolExpression> a ) :
    ChildOperator( ref_agent, item_index, a )
{
}
    

SR::XLink SequenceBackChildOperator::XLinkFromItem( SR::XLink parent_xlink, 
                                                     Itemiser::Element *item ) const
{
    // Cast based on assumption that we'll be looking at a sequence
    auto p_x_seq = dynamic_cast<SequenceInterface *>(item);    
    ASSERT( p_x_seq )("item_index didn't lead to a sequence");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>)
    return SR::XLink(parent_xlink.GetChildX(), &(p_x_seq->back()));        
}


string SequenceBackChildOperator::GetItemType() const
{
    return "seq back";
}

// ------------------------- CollectionFrontChildOperator --------------------------

CollectionFrontChildOperator::CollectionFrontChildOperator( const SR::Agent *ref_agent,
                                                          int item_index, 
                                                          shared_ptr<SymbolExpression> a ) :
    ChildOperator( ref_agent, item_index, a )
{
}
    

SR::XLink CollectionFrontChildOperator::XLinkFromItem( SR::XLink parent_xlink, 
                                                       Itemiser::Element *item ) const
{
    // Cast based on assumption that we'll be looking at a collection
    auto p_x_col = dynamic_cast<CollectionInterface *>(item);    
    ASSERT( p_x_col )("item_index didn't lead to a collection");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>)
    return SR::XLink(parent_xlink.GetChildX(), &*(p_x_col->begin()));        
}


string CollectionFrontChildOperator::GetItemType() const
{
    return "col front";
}

// ------------------------- SingularChildOperator --------------------------

SingularChildOperator::SingularChildOperator( const SR::Agent *ref_agent,
                                              int item_index, 
                                              shared_ptr<SymbolExpression> a ) :
    ChildOperator( ref_agent, item_index, a )
{
}
    

SR::XLink SingularChildOperator::XLinkFromItem( SR::XLink parent_xlink, 
                                                Itemiser::Element *item ) const
{
    // Cast based on assumption that we'll be looking at a singular item
    TreePtrInterface *p_x_singular = dynamic_cast<TreePtrInterface *>(item);
    ASSERT( p_x_singular )("item_index didn't lead to a singular item");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>)
    return SR::XLink(parent_xlink.GetChildX(), p_x_singular);        
}


string SingularChildOperator::GetItemType() const
{
    return "sing";
}

