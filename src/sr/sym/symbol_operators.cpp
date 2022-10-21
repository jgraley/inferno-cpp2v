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


SymbolConstant::SymbolConstant( TreePtr<Node> node ) :
    SymbolConstant( SR::XLink::CreateDistinct(node) )
{
}


unique_ptr<SymbolResultInterface> SymbolConstant::Evaluate( const EvalKit &kit ) const
{
    return make_unique<SymbolResult>( xlink );
}


unique_ptr<SymbolResultInterface> SymbolConstant::GetValue() const
{
    return make_unique<SymbolResult>( xlink );
}


SR::XLink SymbolConstant::GetOnlyXLink() const
{
    return xlink;
}


Orderable::Diff SymbolConstant::OrderCompare3WayLocal( const Orderable &right, 
                                                       OrderProperty order_property ) const 
{
    auto &r = *GET_THAT_POINTER(&right);

    if( xlink == r.xlink )
        return 0;
    else if( xlink < r.xlink )
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


unique_ptr<SymbolResultInterface> SymbolVariable::Evaluate( const EvalKit &kit ) const
{
    // This is an ERROR. You could perfectly easily have called GetRequiredVariables(),
    // done a quick set difference and KNOWN that it would come to this.
    ASSERT( kit.hypothesis_links->count(plink) > 0 );
    
    return make_unique<SymbolResult>( kit.hypothesis_links->at(plink) );
}


SR::PatternLink SymbolVariable::GetPatternLink() const
{
    return plink;
}


shared_ptr<SymbolExpression> SymbolVariable::TrySolveForToEqual( const SolveKit &kit, 
                                                                 shared_ptr<SymbolVariable> target, 
                                                                 shared_ptr<SymbolExpression> to_equal ) const
{
    // Trivial case terminates a recursive solve. This amounts to "what 
    // value should target have, so that this evaluates to to_equal?". 

    if( OrderCompare3Way( *this, *target ) != 0 )
        return nullptr; // This is not the target, so won't be able to solve

    // "what value should this have, so that this evaluates to to_equal?"
    
    if( OrderCompare3Way( *this, *to_equal ) == 0 )
        return nullptr; // This is to_equal, so any value will work

    if( !to_equal->IsIndependentOf( target ) )
        return nullptr; // This is somewhere in the to_equal expression; don't know how to solve

    return to_equal;
}                                                                                                                  


Orderable::Diff SymbolVariable::OrderCompare3WayLocal( const Orderable &right, 
                                                     OrderProperty order_property ) const 
{
    auto &r = *GET_THAT_POINTER(&right);

    if( plink == r.plink )
        return 0;
    else if( plink < r.plink )
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

// ------------------------- ChildToSymbolOperator --------------------------

ChildToSymbolOperator::ChildToSymbolOperator( TreePtr<Node> archetype_node_,
                                                  int item_index_, 
                                                  shared_ptr<SymbolExpression> a_ ) :
    archetype_node( archetype_node_ ),
    item_index( item_index_ ),
    a( a_ )
{
    ASSERT( item_index >= 0 );
}    


list<shared_ptr<SymbolExpression>> ChildToSymbolOperator::GetSymbolOperands() const
{
    return {a};
}


unique_ptr<SymbolResultInterface> ChildToSymbolOperator::Evaluate( const EvalKit &kit,
                                                            list<unique_ptr<SymbolResultInterface>> &&op_results ) const
{
    ASSERT( op_results.size()==1 );

    // XLink must match our referee (i.e. be non-strict subtype)
    unique_ptr<SymbolResultInterface> ar = OnlyElementOf(move(op_results));
    if( !ar->IsDefinedAndUnique() )
        return ar;

    if( !archetype_node->IsSubcategory( ar->GetOnlyXLink().GetChildX().get() ) )
        return make_unique<SymbolResult>(SymbolResult::NOT_A_SYMBOL); // Will not be able to itemise due incompatible type
    
    // Itemise the child node of the XLink we got, according to the "schema"
    // of the referee node (note: link number is only valid wrt referee)
    vector< Itemiser::Element * > keyer_items = archetype_node->Itemise( ar->GetOnlyXLink().GetChildX().get() );   
    ASSERT( item_index < keyer_items.size() );     
    
    // Extract the item indicated by item_index. 
    return EvalFromItem( ar->GetOnlyXLink(), keyer_items[item_index] );
}


Orderable::Diff ChildToSymbolOperator::OrderCompare3WayLocal( const Orderable &right, 
                                                              OrderProperty order_property ) const 
{
    auto &r = *GET_THAT_POINTER(&right);
    //FTRACE(Render())("\n");
    if( Diff d1 = OrderCompare3Way(*archetype_node, 
                                   *r.archetype_node, 
                                   order_property) )
        return d1;

    return item_index - r.item_index;
}  


string ChildToSymbolOperator::Render() const
{
    string name = archetype_node->GetTypeName();

    // Not using RenderForMe() because we always want () here
    return "Child<" + 
           name + 
           "@" + 
           to_string(item_index) + 
           ":" + 
           GetItemTypeName() + 
           ">" + 
           a->RenderWithParentheses(); 
}


Expression::Precedence ChildToSymbolOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}

// ------------------------- ChildOperator --------------------------

shared_ptr<SymbolExpression> ChildOperator::TrySolveForToEqual( const SolveKit &kit, shared_ptr<SymbolVariable> target, 
                                                                shared_ptr<SymbolExpression> to_equal ) const
{   
    // ParentOperator uniquely inverts all the ChildOperators
    auto a_to_equal = make_shared<ParentOperator>( to_equal );
    return a->TrySolveForToEqual( kit, target, a_to_equal );
}

// ------------------------- ChildSequenceFrontOperator --------------------------

unique_ptr<SymbolResultInterface> ChildSequenceFrontOperator::EvalFromItem( SR::XLink parent_xlink, 
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
        
    return make_unique<SymbolResult>( result_xlink );
}


string ChildSequenceFrontOperator::GetItemTypeName() const
{
    return "seq front";
}

// ------------------------- ChildSequenceBackOperator --------------------------

unique_ptr<SymbolResultInterface> ChildSequenceBackOperator::EvalFromItem( SR::XLink parent_xlink, 
                                                                  Itemiser::Element *item ) const
{
    // Cast based on assumption that we'll be looking at a sequence
    auto p_x_seq = dynamic_cast<SequenceInterface *>(item);    
    ASSERT( p_x_seq )("item_index didn't lead to a sequence");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>).
    if( p_x_seq->empty() )
        return make_unique<SymbolResult>( SymbolResult::NOT_A_SYMBOL );
    
    auto result_xlink = SR::XLink(parent_xlink.GetChildX(), &(p_x_seq->back()));        
    return make_unique<SymbolResult>( result_xlink );
}


string ChildSequenceBackOperator::GetItemTypeName() const
{
    return "seq back";
}

// ------------------------- ChildCollectionFrontOperator --------------------------

unique_ptr<SymbolResultInterface> ChildCollectionFrontOperator::EvalFromItem( SR::XLink parent_xlink, 
                                                                     Itemiser::Element *item ) const
{
    // Cast based on assumption that we'll be looking at a collection
    auto p_x_col = dynamic_cast<CollectionInterface *>(item);    
    ASSERT( p_x_col )("item_index didn't lead to a collection");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>).
    if( p_x_col->empty() )
        return make_unique<SymbolResult>( SymbolResult::NOT_A_SYMBOL );
    
    auto result_xlink = SR::XLink(parent_xlink.GetChildX(), &*(p_x_col->begin()));        
    return make_unique<SymbolResult>( result_xlink );
}


string ChildCollectionFrontOperator::GetItemTypeName() const
{
    return "col front";
}

// ------------------------- SingularChildOperator --------------------------

unique_ptr<SymbolResultInterface> SingularChildOperator::EvalFromItem( SR::XLink parent_xlink, 
                                                              Itemiser::Element *item ) const
{
    // Cast based on assumption that we'll be looking at a singular item
    TreePtrInterface *p_x_singular = dynamic_cast<TreePtrInterface *>(item);
    ASSERT( p_x_singular )("item_index didn't lead to a singular item");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>)
    auto result_xlink = SR::XLink(parent_xlink.GetChildX(), p_x_singular);        
    return make_unique<SymbolResult>( result_xlink );
}


string SingularChildOperator::GetItemTypeName() const
{
    return "sing";
}

// ------------------------- XTreeDbToSymbolOperator --------------------------

XTreeDbToSymbolOperator::XTreeDbToSymbolOperator( shared_ptr<SymbolExpression> a_ ) :
    a( a_ )
{
    ASSERT(a);
}    


Expression::VariablesRequiringRows XTreeDbToSymbolOperator::GetVariablesRequiringRows() const
{
    return GetRequiredVariables();
}


list<shared_ptr<SymbolExpression>> XTreeDbToSymbolOperator::GetSymbolOperands() const
{
    return {a};
}


unique_ptr<SymbolResultInterface> XTreeDbToSymbolOperator::Evaluate( const EvalKit &kit,
                                                                       list<unique_ptr<SymbolResultInterface>> &&op_results ) const
{
    // Evaluate operand and ensure we got an XLink
    unique_ptr<SymbolResultInterface> ar = OnlyElementOf(move(op_results));       

    if( !ar->IsDefinedAndUnique() )
        return ar;
        
    const SR::LinkTable::Row &row( kit.x_tree_db->GetRow(ar->GetOnlyXLink()) );   
    SR::XLink result_xlink = EvalXLinkFromRow( kit, ar->GetOnlyXLink(), row );
    if( result_xlink ) 
        return make_unique<SymbolResult>( result_xlink );
    else
        return make_unique<SymbolResult>( SymbolResult::NOT_A_SYMBOL );
}


string XTreeDbToSymbolOperator::Render() const
{
    return GetRenderPrefix() + a->RenderWithParentheses(); 
}


Expression::Precedence XTreeDbToSymbolOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}

// ------------------------- ParentOperator --------------------------
    
SR::XLink ParentOperator::EvalXLinkFromRow( const EvalKit &kit,
                                            SR::XLink xlink, 
                                            const SR::LinkTable::Row &row ) const
{
  
    return kit.x_tree_db->TryGetParentXLink(xlink);
}


shared_ptr<SymbolExpression> ParentOperator::TrySolveForToEqual( const SolveKit &kit, shared_ptr<SymbolVariable> target, 
                                                             shared_ptr<SymbolExpression> to_equal ) const
{   
    // AllChildren and Parent are inverse of each other
    auto a_to_equal = make_shared<AllChildrenOperator>( to_equal );
    return a->TrySolveForToEqual( kit, target, a_to_equal );
}


string ParentOperator::GetRenderPrefix() const
{
    return "Parent";
}

// ------------------------- LastDescendantOperator --------------------------
    
SR::XLink LastDescendantOperator::EvalXLinkFromRow( const EvalKit &kit,
                                                    SR::XLink xlink, 
                                                    const SR::LinkTable::Row &row ) const
{
  
    return kit.x_tree_db->GetLastDescendant(xlink);
}


string LastDescendantOperator::GetRenderPrefix() const
{
    return "LastDescendant";
}

// ------------------------- MyContainerFrontOperator --------------------------
    
SR::XLink MyContainerFrontOperator::EvalXLinkFromRow( const EvalKit &kit,
                                                      SR::XLink xlink, 
                                                      const SR::LinkTable::Row &row ) const
{
  
    return row.my_container_front;
}


string MyContainerFrontOperator::GetRenderPrefix() const
{
    return "MyConFront";
}

// ------------------------- MyContainerBackOperator --------------------------

SR::XLink MyContainerBackOperator::EvalXLinkFromRow( const EvalKit &kit,
                                                     SR::XLink xlink, 
                                                     const SR::LinkTable::Row &row ) const
{
    return row.my_container_back;
}


string MyContainerBackOperator::GetRenderPrefix() const
{
    return "MyConBack";
}

// ------------------------- MySequenceSuccessorOperator --------------------------

SR::XLink MySequenceSuccessorOperator::EvalXLinkFromRow( const EvalKit &kit,
                                                         SR::XLink xlink, 
                                                         const SR::LinkTable::Row &row ) const
{  
    return row.my_sequence_successor;
}


shared_ptr<SymbolExpression> MySequenceSuccessorOperator::TrySolveForToEqual( const SolveKit &kit, shared_ptr<SymbolVariable> target, 
                                                                          shared_ptr<SymbolExpression> to_equal ) const
{   
    // Predecessor and successors are inverse of each other
    auto a_to_equal = make_shared<MySequencePredecessorOperator>( to_equal );
    return a->TrySolveForToEqual( kit, target, a_to_equal );
}


string MySequenceSuccessorOperator::GetRenderPrefix() const
{
    return "MySeqSuccessor";
}

// ------------------------- MySequencePredecessorOperator --------------------------

SR::XLink MySequencePredecessorOperator::EvalXLinkFromRow( const EvalKit &kit,
                                                           SR::XLink xlink, 
                                                           const SR::LinkTable::Row &row ) const
{  
    return row.my_sequence_predecessor;
}


shared_ptr<SymbolExpression> MySequencePredecessorOperator::TrySolveForToEqual( const SolveKit &kit, shared_ptr<SymbolVariable> target, 
                                                                            shared_ptr<SymbolExpression> to_equal ) const
{   
    // Predecessor and successors are inverse of each other
    auto a_to_equal = make_shared<MySequenceSuccessorOperator>( to_equal );
    return a->TrySolveForToEqual( kit, target, a_to_equal );
}


string MySequencePredecessorOperator::GetRenderPrefix() const
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


unique_ptr<SymbolResultInterface> AllChildrenOperator::Evaluate( const EvalKit &kit,
                                                            list<unique_ptr<SymbolResultInterface>> &&op_results ) const
{
    ASSERT( op_results.size()==1 );

    // XLink must match our referee (i.e. be non-strict subtype)
    unique_ptr<SymbolResultInterface> ar = OnlyElementOf(move(op_results));
    if( !ar->IsDefinedAndUnique() )
        return ar;

    TreePtr<Node> parent_node = ar->GetOnlyXLink().GetChildX();
    FlattenNode flat( parent_node );

    set<SR::XLink> child_xlinks;
    for(const TreePtrInterface &child_node : flat )
    {
        SR::XLink child_xlink( parent_node, &child_node);
        child_xlinks.insert( child_xlink );
    }
    
    return make_unique<SetResult>( child_xlinks );
}


shared_ptr<SymbolExpression> AllChildrenOperator::TrySolveForToEqual( const SolveKit &kit, shared_ptr<SymbolVariable> target, 
                                                                      shared_ptr<SymbolExpression> to_equal ) const
{   
    // AllChildren and Parent are inverse of each other
    auto a_to_equal = make_shared<ParentOperator>( to_equal );
    return a->TrySolveForToEqual( kit, target, a_to_equal );
}


string AllChildrenOperator::Render() const
{
    return "AllChildren" + a->RenderWithParentheses(); 
}


Expression::Precedence AllChildrenOperator::GetPrecedence() const
{
    return Precedence::PREFIX;
}

