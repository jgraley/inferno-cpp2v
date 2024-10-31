#include "transform_of_agent.hpp"
#include "../scr_engine.hpp"
#include "link.hpp"
#include "db/x_tree_database.hpp"
#include "../../tree/cpptree.hpp"

using namespace SR;

#define DEFER_POLICY

// ---------------------- TransformOfAgent::AugBE ---------------------------

TransformOfAgent::AugBE::AugBE( TreePtr<Node> generic_tree_ptr_, const TransUtils *utils_ ) :
	utils( utils_ ),	
	xlink(),
	generic_tree_ptr(generic_tree_ptr_),
	my_deps(make_shared<Dependencies>())
{
	ASSERT( utils );
}


TransformOfAgent::AugBE::AugBE( XLink xlink_, const TransUtils *utils_) :
	utils( utils_ ),
	xlink(xlink_),
    generic_tree_ptr(xlink.GetChildTreePtr()),
	my_deps(make_shared<Dependencies>())
{
	ASSERT( utils );
	ASSERT( xlink );

#ifdef DEFER_POLICY
	my_deps->AddDep( xlink );		
#else	
	utils->GetDeps()->AddDep( xlink );
#endif		
}


TransformOfAgent::AugBE::AugBE( const AugBE &other, TreePtr<Node> generic_tree_ptr_ ) :
	utils( other.utils ),
	xlink(),
	generic_tree_ptr(generic_tree_ptr_),
	my_deps( other.my_deps )
{
	ASSERT( utils );
}


TransformOfAgent::AugBE::AugBE( const AugBE &other, XLink xlink_ ) :
	utils( other.utils ),
	xlink(xlink_),
    generic_tree_ptr(xlink.GetChildTreePtr()),
	my_deps( other.my_deps )
{	
	ASSERT( utils );
	ASSERT( xlink );
	
#ifdef DEFER_POLICY
	my_deps->AddDep( xlink );
#else
	utils->GetDeps()->AddDep( xlink );	
#endif		
}


TransformOfAgent::AugBE *TransformOfAgent::AugBE::Clone() const
{
	return new TransformOfAgent::AugBE( *this );
}


TreePtr<Node> TransformOfAgent::AugBE::GetGenericTreePtr() const
{
	return generic_tree_ptr;
}


XLink TransformOfAgent::AugBE::GetXLink() const
{
	return xlink;
}


const TransformOfAgent::Dependencies &TransformOfAgent::AugBE::GetDeps() const
{
	return *my_deps;
}


TransformOfAgent::AugBE *TransformOfAgent::AugBE::OnGetChild( const TreePtrInterface *other_tree_ptr )
{
	// If we are Tree then construct+return Tree style, otherwise reduce to Free style. This
	// is to stop descendents of Free masquerading as Tree.	
	if( xlink )
	{
		// We're roaming the x tree
		ASSERT( !ON_STACK(other_tree_ptr) );
		// DEFER_POLICY: my_deps will be shared with the new node, which will add itself
		return new TransformOfAgent::AugBE(*this, utils->db->GetXLink(other_tree_ptr)); // tree
	}
	else
	{
		// We're moving through our free tree - not illegal
		// DEFER_POLICY: my_deps will be shared with the new node, which will add itself
		return new TransformOfAgent::AugBE(*this, (TreePtr<Node>)*other_tree_ptr); // free
	}
}


void TransformOfAgent::AugBE::OnSetChild( const TreePtrInterface *other_tree_ptr, AugBEInterface *new_val )
{
	ASSERT( utils );
    auto n = GET_THAT_POINTER(new_val);

	// We have to be free
	ASSERT( !xlink )("Transformation attempts to modify the tree!");
	
	if( n->xlink )
	{	
		// we've meandered into the x tree
#ifdef DEFER_POLICY
		// DEFER_POLICY: Meandering into the tree: parent indirects to child deps
		my_deps->AddInd( n->my_deps );
#endif
	}	
	else
	{
#ifdef DEFER_POLICY
		// we're building our free tree
		// DEFER_POLICY: Construction: parent indirects to child deps
		my_deps->AddInd( n->my_deps );
#endif
	}
}


void TransformOfAgent::AugBE::OnDepLeak()
{
	// DEFER_POLICY: Leap dumps our deps stright into dest
	// (dest is the resultant dep set for the whole transformation)
	ASSERT( utils );
#ifdef DEFER_POLICY
	utils->GetDeps()->AddAll( *my_deps );
#endif	
}


string TransformOfAgent::AugBE::GetTrace() const 
{ 
	return "TODO"; 
}

// ---------------------- TransformOfAgent::TransUtils ---------------------------

TransformOfAgent::TransUtils::TransUtils( const XTreeDatabase *db_, Dependencies *deps_ ) :
	db(db_),
	deps(deps_)
{
}	


AugTreePtr<Node> TransformOfAgent::TransUtils::CreateAugTreePtr(XLink xlink) const
{
	return AugTreePtr<Node>(xlink.GetChildTreePtr(), 
	                        ValuePtr<TransformOfAgent::AugBE>::Make(xlink, this));
}	


ValuePtr<AugBEInterface> TransformOfAgent::TransUtils::CreateBE( TreePtr<Node> tp ) const 
{
	return ValuePtr<TransformOfAgent::AugBE>::Make(tp, this);
}


ValuePtr<TransformOfAgent::AugBE> TransformOfAgent::TransUtils::GetBE( const AugTreePtrBase &atp ) const
{
	return ValuePtr<TransformOfAgent::AugBE>::DynamicCast(atp.GetImpl());	
}


set<AugTreePtr<Node>> TransformOfAgent::TransUtils::GetDeclarers( AugTreePtr<Node> node ) const
{
    if( !db->HasNodeRow(node.GetTreePtr()) ) // not found
        throw NavigationInterface::UnknownNode();

    NodeTable::Row node_row = db->GetNodeRow( node.GetTreePtr() );  
    
    // Generate ATPs from declarers
	set<AugTreePtr<Node>> atp_declarers;	
    for( XLink declaring_xlink : node_row.declaring_xlinks )
    {   
		// We want the XLink that points to the declarer
		XLink declarer_xlink = db->TryGetParentXLink(declaring_xlink);
		atp_declarers.insert( CreateAugTreePtr(declarer_xlink) ); 
	}
	
	return atp_declarers;
}


TeleportAgent::Dependencies *TransformOfAgent::TransUtils::GetDeps() const
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


TeleportAgent::QueryReturnType TransformOfAgent::RunTeleportQuery( const XTreeDatabase *db, Dependencies *deps, XLink stimulus_xlink ) const
{
    // Transform the candidate expression, sharing the x_tree_db as a TransKit
    // so that implementations can use handy features without needing to search
    // the tree. Note that transformations work on nodes, not XLinks, so some
    // precision is lost.
    
    // Policy: Don't convert MMAX link to a node (will evaluate to EmptyResult)
    if( stimulus_xlink == XLink::MMAX_Link )
         return QueryReturnType(); 
         
	TransformOfAgent::TransUtils utils(db, deps);
    TransKit kit { &utils };

    try
    {
		AugTreePtr<Node> stimulus_x = utils.CreateAugTreePtr( stimulus_xlink );	
		
		AugTreePtr<Node> atp = transformation->ApplyTransformation( kit, stimulus_x );  
		
		ValuePtr<AugBE> be = utils.GetBE(atp);
		
#ifdef DEFER_POLICY
		// Grab the final deps stored in the ATP. Same as a dep leak, but explicit for clarity.
		deps->AddAll( be->GetDeps() );
#endif
		
		XLink xlink = be->GetXLink(); 
		TreePtr<Node> tp = be->GetGenericTreePtr();		
		ASSERT( tp->IsFinal() )(*this)(" computed non-final ")(tp)(" from ")(stimulus_x)("\n");                				
        if( xlink ) 
            return make_pair(xlink, nullptr);  // tree      
		else
            return make_pair(XLink(), tp);  // free 
	}
    catch( const ::Mismatch &e )
    {
		TRACE("Caught ")(e)("; query fails\n");
		return QueryReturnType(); // NULL
	}
}


Graphable::Block TransformOfAgent::GetGraphBlockInfo() const
{
    Block block;
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
    block.sub_blocks = { { "pattern", 
                           "", 
                           true,
                           { link } } };
    return block;
}


string TransformOfAgent::GetName() const
{
	return transformation->GetName() + GetSerialString();
}


bool TransformOfAgent::IsExtenderChannelLess( const Extender &r ) const
{
	// If comparing two TransformOfAgent, secondary onto the transformation object's type
	// TODO transformation object's state might matter, so should call into it
	if( auto rto = dynamic_cast<const TransformOfAgent *>(&r) )
		return typeid(*transformation).before(typeid(*rto->transformation));
	
	// Otherwise resort to the default compare
	return TeleportAgent::IsExtenderChannelLess(r);
}


int TransformOfAgent::GetExtenderChannelOrdinal() const
{
	return 1; // TODO class id as an ordinal?
}



