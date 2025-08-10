#include "transform_of_agent.hpp"
#include "../scr_engine.hpp"
#include "link.hpp"
#include "db/x_tree_database.hpp"
#include "../../tree/cpptree.hpp"

#define THROW_ON_NULL

using namespace SR;

// ---------------------- TransformOfAgent::AugBECommon ---------------------------    

TransformOfAgent::AugBECommon::AugBECommon( const TransUtils *utils_ ) :
    utils( utils_ ),
    my_deps(make_shared<Dependencies>())    
{
}

const TransformOfAgent::TransUtils *TransformOfAgent::AugBECommon::GetUtils() const
{
    return utils;
}


TransformOfAgent::Dependencies &TransformOfAgent::AugBECommon::GetDeps()
{
    return *my_deps;
}


const TransformOfAgent::Dependencies &TransformOfAgent::AugBECommon::GetDeps() const 
{
    return *my_deps;
}


shared_ptr<TransformOfAgent::Dependencies> TransformOfAgent::AugBECommon::GetDepsPtr() 
{
    return my_deps;
}

void TransformOfAgent::AugBECommon::OnDepLeak()
{
    // Policy: Leak dumps our deps stright into dest
    // (dest is the resultant dep set for the whole transformation)
    ASSERT( utils );
    utils->GetDeps()->AddAllFrom( *my_deps );
}


string TransformOfAgent::AugBECommon::GetTrace() const 
{ 
    return "TODO"; 
}

// ---------------------- TransformOfAgent::AugBERoaming ---------------------------

TransformOfAgent::AugBERoaming::AugBERoaming( XLink xlink_, const TransUtils *utils_) :
    AugBECommon( utils_ ),
    xlink(xlink_)
{
    ASSERT( xlink );

    // When constructed from XLink, we can assume it came from the tree
    // Policy: when constructing from tree, depend on self
    GetDeps().AddDep( xlink );            
}


TransformOfAgent::AugBERoaming::AugBERoaming( const AugBECommon &other, XLink xlink_ ) :
    AugBECommon( other ),
    xlink(xlink_)
{    
    ASSERT( xlink );
    
    // Partial copy-construct with xlink, we can assume it came from tree
    // Policy: copy in the deps (copy constructor), and add in self as another dependency
    GetDeps().AddDep( xlink );    
}


TransformOfAgent::AugBERoaming *TransformOfAgent::AugBERoaming::Clone() const
{
    return new TransformOfAgent::AugBERoaming( *this );
}


XLink TransformOfAgent::AugBERoaming::GetXLink() const
{
    return xlink;
}


TransformOfAgent::AugBERoaming *TransformOfAgent::AugBERoaming::OnGetChild( const TreePtrInterface *other_tree_ptr )
{
#ifdef THROW_ON_NULL
    if( !(TreePtr<Node>)*other_tree_ptr )
		throw ReachedNullChiled();
#endif
		
    // We're roaming the x tree so construct+return Tree style
    // Policy: my_deps will be copied into with the new node, which will add itself
    return new TransformOfAgent::AugBERoaming(*this, GetUtils()->db->GetXLink(other_tree_ptr)); // tree
}


void TransformOfAgent::AugBERoaming::OnSetChild( const TreePtrInterface *other_tree_ptr, AugBEInterface *new_val )
{
    ASSERTFAIL(); // would modify the x-tree, not allowed
}


string TransformOfAgent::AugBERoaming::GetTrace() const 
{ 
    return "TODO"; 
}

// ---------------------- TransformOfAgent::AugBEMeandering ---------------------------

TransformOfAgent::AugBEMeandering::AugBEMeandering( TreePtr<Node> generic_tree_ptr_, const TransUtils *utils_ ) :
    AugBECommon( utils_ ),    
    generic_tree_ptr(generic_tree_ptr_)
{
    // When constructed from TreePtr, we can assume it's new and free
    // We have nothing we can depend on.
}


TransformOfAgent::AugBEMeandering::AugBEMeandering( const AugBECommon &other, TreePtr<Node> generic_tree_ptr_ ) :
    AugBECommon( other ),
    generic_tree_ptr(generic_tree_ptr_)
{
    // Partial copy-construct with TreePtr, we can assume it's new and free
    // Policy: copy in the deps (copy constructor), but we have nothing to add
}


TransformOfAgent::AugBEMeandering *TransformOfAgent::AugBEMeandering::Clone() const
{
    return new TransformOfAgent::AugBEMeandering( *this );
}


TreePtr<Node> TransformOfAgent::AugBEMeandering::GetGenericTreePtr() const
{
    return generic_tree_ptr;
}


AugBEInterface *TransformOfAgent::AugBEMeandering::OnGetChild( const TreePtrInterface *other_tree_ptr )
{
#ifdef THROW_ON_NULL
    if( !(TreePtr<Node>)*other_tree_ptr )
		throw ReachedNullChiled();
#endif		
    // We're moving through our free tree - not illegal. We get here if a free section of tree was
    // already created (eg using AugBEMeandering::OnSetChild()) and we're re-analysiing it, for 
    // example if a transformation has invoked a different transformation and must now pick a 
    // base node from within that other transformation's output.
    
    // Policy: my_deps will be copied into the new node 
    if( XLink xlink = GetUtils()->db->TryGetXLink(other_tree_ptr) )
        return new TransformOfAgent::AugBERoaming(*this, xlink); // meandered into X tree, now we're roaming
    else     
        return new TransformOfAgent::AugBEMeandering(*this, (TreePtr<Node>)*other_tree_ptr); // still meandering
}


void TransformOfAgent::AugBEMeandering::OnSetChild( const TreePtrInterface *other_tree_ptr, AugBEInterface *new_val )
{
    ASSERT( new_val );
    AugBECommon *n = dynamic_cast<AugBECommon *>(new_val);
    ASSERT( n );
            
    // We're building our free tree OR we're meandering into the x tree
    // Policy: parent indirects to child's deps 
    GetDeps().AddChainTo( n->GetDepsPtr() );
}


string TransformOfAgent::AugBEMeandering::GetTrace() const 
{ 
    return "TODO"; 
}

// ---------------------- TransformOfAgent::TransUtils ---------------------------

TransformOfAgent::TransUtils::TransUtils( const XTreeDatabase *db_, Dependencies *deps_ ) :
    db(db_),
    deps(deps_)
{
}    


AugTreePtr<Node> TransformOfAgent::TransUtils::CreateAugTreePtrRoaming(XLink xlink) const
{
    return AugTreePtr<Node>(xlink.GetChildTreePtr(), 
                            ValuePtr<TransformOfAgent::AugBERoaming>::Make(xlink, this));
}    


ValuePtr<AugBEInterface> TransformOfAgent::TransUtils::CreateBE( TreePtr<Node> tp ) const 
{
    return ValuePtr<TransformOfAgent::AugBEMeandering>::Make(tp, this);
}


ValuePtr<TransformOfAgent::AugBECommon> TransformOfAgent::TransUtils::GetBE( const AugTreePtrBase &atp ) const
{
    auto be = ValuePtr<AugBECommon>::DynamicCast(atp.GetImpl());    
    ASSERTS(be);
    return be;
}


set<AugTreePtr<Node>> TransformOfAgent::TransUtils::GetDeclarers( AugTreePtr<Node> node ) const
{
    auto be = ValuePtr<AugBERoaming>::DynamicCast(node.GetImpl());    
    if( !be ) // Not roaming
        throw TransUtilsInterface::UnknownNode();
        
    if( !db->HasNodeRow(be->GetXLink().GetChildTreePtr()) ) // not found
        throw TransUtilsInterface::UnknownNode();

    NodeTable::Row node_row = db->GetNodeRow( be->GetXLink().GetChildTreePtr() );  
    
    // Generate ATPs from declarers
    set<AugTreePtr<Node>> atp_declarers;    
    for( XLink declaring_xlink : node_row.declaring_xlinks )
    {   
        // We want the XLink that points to the declarer
        XLink declarer_xlink = db->TryGetParentXLink(declaring_xlink);
        atp_declarers.insert( CreateAugTreePtrRoaming(declarer_xlink) ); 
    }
    
    return atp_declarers;
}


RelocatingAgent::Dependencies *TransformOfAgent::TransUtils::GetDeps() const
{
    return deps;
}

// ---------------------- TransformOfAgent ---------------------------

shared_ptr<PatternQuery> TransformOfAgent::GetPatternQuery() const
{
    auto pq = make_shared<PatternQuery>();
    pq->RegisterNormalLink( PatternLink(this, &pattern) );
    return pq;
}


RelocatingAgent::RelocatingQueryResult TransformOfAgent::RunRelocatingQuery( const XTreeDatabase *db, XLink stimulus_xlink ) const
{
    // Transform the candidate expression, sharing the x_tree_db as a TransKit
    // so that implementations can use handy features without needing to search
    // the tree. Note that transformations work on nodes, not XLinks, so some
    // precision is lost.
    
    // Policy: Don't convert MMAX link to a node (will evaluate to EmptyResult)
    if( stimulus_xlink == XLink::MMAX )
         return RelocatingQueryResult(); 
    
    Dependencies deps;
    TransformOfAgent::TransUtils utils(db, &deps);
    TransKit kit { &utils };

    try
    {
        // We always begin by roaming because stimulus XLinks are in the X tree.
        AugTreePtr<Node> stimulus_x = utils.CreateAugTreePtrRoaming( stimulus_xlink );    
        
        AugTreePtr<Node> base_atp = transformation->ApplyTransformation( kit, stimulus_x );  
        
        ValuePtr<AugBECommon> base_be = utils.GetBE(base_atp);        
        // Grab the final deps stored in the ATP. Same as a dep leak, but explicit for clarity.
        deps.AddAllFrom( base_be->GetDeps() );
                
        if( auto base_bem = ValuePtr<AugBEMeandering>::DynamicCast(move(base_be)) ) 
        {            
            // Base is outside the X tree, so domain extension will be required                        
            TreePtr<Node> tp = base_bem->GetGenericTreePtr();        
            return RelocatingQueryResult( tp, deps );  // free 
        }
        else if( auto base_ber = ValuePtr<AugBERoaming>::DynamicCast(move(base_be)) ) 
        {
            // Base is inside the X tree, so domain extension will not be required
            XLink xlink = base_ber->GetXLink(); 
            return RelocatingQueryResult( xlink );  // tree      
        }
        else
        {
            ASSERTFAIL(); // Unknown BE class
        }
    }
    catch( const ::Mismatch &e )
    {
        TRACE("Caught ")(e)("; query fails\n");
        return RelocatingQueryResult(); // NULL
    }
}


Graphable::NodeBlock TransformOfAgent::GetGraphBlockInfo() const
{
    NodeBlock block;
    // The TransformOf node appears as a slightly flattened octagon, with the name of the specified 
    // kind of Transformation class inside it.
    block.bold = true;
    block.title = transformation->GetName();
    block.shape = "octagon";
    block.block_type = Graphable::NODE_SHAPED;
    block.node = GetPatternPtr();
    auto link = make_shared<Graphable::Link>( dynamic_cast<Graphable *>(pattern.get()),
              list<string>{},
              list<string>{},
              phase,
              &pattern );
    block.item_blocks = { { "pattern", 
                           "", 
                           true,
                           { link } } };
    return block;
}


string TransformOfAgent::GetName() const
{
    return transformation->GetName() + GetSerialString();
}


string TransformOfAgent::GetTrace() const
{
    return TransformOfAgent::GetName(); // No v-call, use our one
}


bool TransformOfAgent::IsExtenderChannelLess( const Extender &r ) const
{
    // If comparing two TransformOfAgent, secondary onto the transformation object's type
    // TODO transformation object's state might matter, so should call into it
    if( auto rto = dynamic_cast<const TransformOfAgent *>(&r) )
        return typeid(*transformation).before(typeid(*rto->transformation));
    
    // Otherwise resort to the default compare
    return RelocatingAgent::IsExtenderChannelLess(r);
}


int TransformOfAgent::GetExtenderChannelOrdinal() const
{
    return 1; // TODO class id as an ordinal?
}



