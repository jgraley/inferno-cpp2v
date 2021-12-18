#include "symbol_operators.hpp"

#include "node/node.hpp"
#include "../agents/agent.hpp"

using namespace SYM;

// ------------------------- SingularChildOperator --------------------------

SingularChildOperator::SingularChildOperator( const SR::Agent *ref_agent_,
                                              int item_, 
                                              shared_ptr<SymbolExpression> a_ ) :
    ref_agent( ref_agent_ ),
    item( item_ ),
    a( a_ )
{
    ASSERT( item >= 0 );
}    


set<shared_ptr<Expression>> SingularChildOperator::GetOperands() const
{
    set<shared_ptr<Expression>> ops;
    ops.insert(a);
    return ops;
}


unique_ptr<SymbolResult> SingularChildOperator::Evaluate( const EvalKit &kit ) const
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
    
    // Extract the item indicated by item and cast as per singular. 
    ASSERT( item < keyer_itemised.size() );
    TreePtrInterface *p_x_singular = dynamic_cast<TreePtrInterface *>(keyer_itemised[item]);
    ASSERT( p_x_singular )("item didn't lead to a singular child pointer");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but coming from the correct TreePtr<Node>
    SR::XLink sing_xlink(ar->xlink.GetChildX(), p_x_singular);        
    
    return make_unique<SymbolResult>( sing_xlink );
}


string SingularChildOperator::Render() const
{
    string inner_typename = RemoveOuterTemplate( ref_agent->GetTypeName() );

    // Not using RenderForMe() because we always want () here
    return "Item<" + inner_typename + ", sing " + to_string(item) + ">(" + a->Render() + ")"; 
}


Expression::Precedence SingularChildOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}
