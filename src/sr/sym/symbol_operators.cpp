#include "symbol_operators.hpp"

#include "result.hpp"

#include "helpers/flatten.hpp"
#include "node/node.hpp"
#include "../db/x_tree_database.hpp"

using namespace SYM;

// ------------------------- SymbolConstant --------------------------

SymbolConstant::SymbolConstant( XValue xlink_ ) :
	node( nullptr ),
    xlink( xlink_ )
{
	//FTRACE(xlink)("\n");
}


SymbolConstant::SymbolConstant( TreePtr<Node> node_ ) :
	node( node_ ),
    xlink( XValue::CreateFrom(&node) )
{
	//FTRACE(xlink)("\n");
}


unique_ptr<SymbolicResult> SymbolConstant::Evaluate( const EvalKit &kit ) const
{
	(void)kit;
	INDENT("S");
    return make_unique<UniqueResult>( xlink );
}


unique_ptr<SymbolicResult> SymbolConstant::GetValue() const
{
	//FTRACE(xlink)("\n");
    return make_unique<UniqueResult>( xlink );
}


XValue SymbolConstant::GetOnlyXLink() const
{
	//FTRACE(xlink)("\n");
    return xlink;
}


Orderable::Diff SymbolConstant::OrderCompare3WayCovariant( const Orderable &right, 
                                                           OrderProperty order_property ) const 
{
	(void)order_property;
    auto &r = GET_THAT_REFERENCE(right);
	//FTRACE(xlink)("\n");

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


unique_ptr<SymbolicResult> SymbolVariable::Evaluate( const EvalKit &kit ) const
{
	INDENT("s");
    // This is an ERROR. You could perfectly easily have called GetRequiredVariables(),
    // done a quick set difference and KNOWN that it would come to this.
    ASSERT( kit.hypothesis_links->count(plink) > 0 );
    return make_unique<UniqueResult>( kit.hypothesis_links->at(plink) );
}


SR::PatternLink SymbolVariable::GetPatternLink() const
{
    return plink;
}


shared_ptr<SymbolExpression> SymbolVariable::TrySolveForToEqual( const SolveKit &kit, 
                                                                 shared_ptr<SymbolVariable> target, 
                                                                 shared_ptr<SymbolExpression> to_equal ) const
{
	(void)kit;
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


Orderable::Diff SymbolVariable::OrderCompare3WayCovariant( const Orderable &right, 
                                                           OrderProperty order_property ) const 
{
	(void)order_property;
    auto &r = GET_THAT_REFERENCE(right);

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
                                              vector< Itemiser::Element * >::size_type item_index_, 
                                              shared_ptr<SymbolExpression> a_ ) :
    archetype_node( archetype_node_ ),
    item_index( item_index_ ),
    a( a_ )
{
}    


list<shared_ptr<SymbolExpression>> ChildToSymbolOperator::GetSymbolOperands() const
{
    return {a};
}


unique_ptr<SymbolicResult> ChildToSymbolOperator::Evaluate( const EvalKit &kit,
                                                            list<unique_ptr<SymbolicResult>> &&op_results ) const
{
	(void)kit;
    ASSERT( op_results.size()==1 );

    // XLink must match our referee (i.e. be non-strict subtype)
    unique_ptr<SymbolicResult> ar = SoloElementOf(move(op_results));
    if( !ar->IsDefinedAndUnique() )
        return ar;

    if( !archetype_node->IsSubcategory( *(ar->GetOnlyXLink().GetChildTreePtr()) ) )
        return make_unique<EmptyResult>(); // Will not be able to itemise due incompatible type
    
    // Itemise the child node of the XLink we got, according to the "schema"
    // of the referee node (note: link number is only valid wrt referee)
    vector< Itemiser::Element * > keyer_items = archetype_node->Itemise( ar->GetOnlyXLink().GetChildTreePtr().get() );   
    ASSERT( item_index < keyer_items.size() );     
    
    // Extract the item indicated by item_index. 
    return EvalFromItem( ar->GetOnlyXLink(), keyer_items[item_index] );
}


Orderable::Diff ChildToSymbolOperator::OrderCompare3WayCovariant( const Orderable &right, 
                                                                  OrderProperty order_property ) const 
{
    auto &r = GET_THAT_REFERENCE(right);
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

unique_ptr<SymbolicResult> ChildSequenceFrontOperator::EvalFromItem( XValue parent_xlink, 
                                                                     Itemiser::Element *item ) const
{
	(void)parent_xlink;
	
    // Cast based on assumption that we'll be looking at a sequence
    auto p_x_seq = dynamic_cast<SequenceInterface *>(item);    
    ASSERT( p_x_seq )("item_index didn't lead to a sequence");
    
    XValue result_xlink;
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>) or OffEnd if container empty.
    if( p_x_seq->empty() )
        result_xlink = XValue::OffEnd; // OffEnd IS allowed in this case
    else
        result_xlink = XValue(&(p_x_seq->front()));        
        
    return make_unique<UniqueResult>( result_xlink );
}


string ChildSequenceFrontOperator::GetItemTypeName() const
{
    return "seq front";
}

// ------------------------- ChildSequenceBackOperator --------------------------

unique_ptr<SymbolicResult> ChildSequenceBackOperator::EvalFromItem( XValue parent_xlink, 
                                                                    Itemiser::Element *item ) const
{
    (void)parent_xlink;
    
    // Cast based on assumption that we'll be looking at a sequence
    auto p_x_seq = dynamic_cast<SequenceInterface *>(item);    
    ASSERT( p_x_seq )("item_index didn't lead to a sequence");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>).
    if( p_x_seq->empty() )
        return make_unique<EmptyResult>();
    
    auto result_xlink = XValue(&(p_x_seq->back()));        
    return make_unique<UniqueResult>( result_xlink );
}


string ChildSequenceBackOperator::GetItemTypeName() const
{
    return "seq back";
}

// ------------------------- ChildCollectionFrontOperator --------------------------

unique_ptr<SymbolicResult> ChildCollectionFrontOperator::EvalFromItem( XValue parent_xlink, 
                                                                       Itemiser::Element *item ) const
{
	(void)parent_xlink;
	
    // Cast based on assumption that we'll be looking at a collection
    auto p_x_col = dynamic_cast<CollectionInterface *>(item);    
    ASSERT( p_x_col )("item_index didn't lead to a collection");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>).
    if( p_x_col->empty() )
        return make_unique<EmptyResult>();
    
    auto result_xlink = XValue(&*(p_x_col->begin()));        
    return make_unique<UniqueResult>( result_xlink );
}


string ChildCollectionFrontOperator::GetItemTypeName() const
{
    return "col front";
}

// ------------------------- SingularChildOperator --------------------------

unique_ptr<SymbolicResult> SingularChildOperator::EvalFromItem( XValue parent_xlink, 
                                                                Itemiser::Element *item ) const
{
	(void)parent_xlink;
	
    // Cast based on assumption that we'll be looking at a singular item
    TreePtrInterface *p_x_singular = dynamic_cast<TreePtrInterface *>(item);
    ASSERT( p_x_singular )("item_index didn't lead to a singular item");
    
    // Create the correct XLink (i.e. not just pointing to the correct child Node,
    // but also coming from the correct TreePtr<Node>)
    auto result_xlink = XValue(p_x_singular);        
    return make_unique<UniqueResult>( result_xlink );
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


unique_ptr<SymbolicResult> XTreeDbToSymbolOperator::Evaluate( const EvalKit &kit,
                                                              list<unique_ptr<SymbolicResult>> &&op_results ) const
{
    // Evaluate operand and ensure we got an XLink
    unique_ptr<SymbolicResult> ar = SoloElementOf(move(op_results));       

    if( !ar->IsDefinedAndUnique() )
        return ar; // TODO what if non-unique?
        
    // These DB operations only work on XLinks supported by the DB. Does not include eg MMAX.   
    if( !kit.x_tree_db->HasRow(ar->GetOnlyXLink()) )
        return make_unique<EmptyResult>();
        
    const SR::LinkTable::Row &row( kit.x_tree_db->GetRow(ar->GetOnlyXLink()) );   
    XValue result_xlink = EvalXLinkFromRow( kit, ar->GetOnlyXLink(), row );
    if( result_xlink ) 
        return make_unique<UniqueResult>( result_xlink );
    else
        return make_unique<EmptyResult>();
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
    
XValue ParentOperator::EvalXLinkFromRow( const EvalKit &kit,
                                         XValue xlink, 
                                         const SR::LinkTable::Row &row ) const
{
    (void)xlink;
    (void)row;
    return kit.x_tree_db->TryGetParentXLink(xlink);
}


shared_ptr<SymbolExpression> ParentOperator::TrySolveForToEqual( const SolveKit &kit, shared_ptr<SymbolVariable> target, 
                                                                 shared_ptr<SymbolExpression> to_equal ) const
{   
	(void)kit;
	
    // AllChildren and Parent are inverse of each other
    auto a_to_equal = make_shared<AllChildrenOperator>( to_equal );
    return a->TrySolveForToEqual( kit, target, a_to_equal );
}


string ParentOperator::GetRenderPrefix() const
{
    return "Parent";
}

// ------------------------- LastDescendantOperator --------------------------
    
XValue LastDescendantOperator::EvalXLinkFromRow( const EvalKit &kit,
                                                 XValue xlink, 
                                                 const SR::LinkTable::Row &row ) const
{
    (void)kit;
    (void)xlink;
    (void)row;
    return SR::XTreeDatabase::GetLastDescendantXLink(xlink);
}


string LastDescendantOperator::GetRenderPrefix() const
{
    return "LastDescendant";
}

// ------------------------- MyContainerFrontOperator --------------------------
    
XValue MyContainerFrontOperator::EvalXLinkFromRow( const EvalKit &kit,
                                                   XValue xlink, 
                                                   const SR::LinkTable::Row &row ) const
{
    (void)kit;
    (void)xlink;
    return row.container_front;
}


string MyContainerFrontOperator::GetRenderPrefix() const
{
    return "MyConFront";
}

// ------------------------- MyContainerBackOperator --------------------------

XValue MyContainerBackOperator::EvalXLinkFromRow( const EvalKit &kit,
                                                  XValue xlink, 
                                                  const SR::LinkTable::Row &row ) const
{
    (void)kit;
    (void)xlink;
    return row.container_back;
}


string MyContainerBackOperator::GetRenderPrefix() const
{
    return "MyConBack";
}

// ------------------------- MySequenceSuccessorOperator --------------------------

XValue MySequenceSuccessorOperator::EvalXLinkFromRow( const EvalKit &kit,
                                                      XValue xlink, 
                                                      const SR::LinkTable::Row &row ) const
{  
    (void)kit;
    (void)xlink;
    return row.sequence_successor;
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

XValue MySequencePredecessorOperator::EvalXLinkFromRow( const EvalKit &kit,
                                                        XValue xlink, 
                                                        const SR::LinkTable::Row &row ) const
{  
    (void)kit;
    (void)xlink;
    return row.sequence_predecessor;
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


unique_ptr<SymbolicResult> AllChildrenOperator::Evaluate( const EvalKit &kit,
                                                          list<unique_ptr<SymbolicResult>> &&op_results ) const
{
	(void)kit;
    ASSERT( op_results.size()==1 );

    // XLink must match our referee (i.e. be non-strict subtype)
    unique_ptr<SymbolicResult> ar = SoloElementOf(move(op_results));
    if( !ar->IsDefinedAndUnique() )
        return ar;

    TreePtr<Node> parent_node = ar->GetOnlyXLink().GetChildTreePtr();
    FlattenNode flat( parent_node );

    set<XValue> child_xlinks;
    for(const TreePtrInterface &child_node : flat )
    {
        XValue child_xlink( &child_node);
        child_xlinks.insert( child_xlink );
    }
    
    return make_unique<SubsetResult>( child_xlinks );
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

