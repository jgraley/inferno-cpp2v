#include "transform_of_agent.hpp"
#include "../scr_engine.hpp"
#include "link.hpp"
#include "db/x_tree_database.hpp"
#include "../../tree/cpptree.hpp"

using namespace SR;

// ---------------------- TransformOfAgent::AugBECommon ---------------------------	

TransformOfAgent::AugBECommon::AugBECommon( const TransUtils *utils_ ) :
	utils( utils_ ),
	my_deps(make_shared<Dependencies>())	
{
}

// ---------------------- TransformOfAgent::AugBERoaming ---------------------------

TransformOfAgent::AugBERoaming::AugBERoaming( XLink xlink_, const TransUtils *utils_) :
	AugBECommon( utils_ ),
	xlink(xlink_)
{
	ASSERT( utils );
	ASSERT( xlink );

	// When constructed from XLink, we can assume it came from the tree
	// Policy: when constructing from tree, depend on self
	my_deps->AddDep( xlink );			
}


TransformOfAgent::AugBERoaming::AugBERoaming( const AugBECommon &other, XLink xlink_ ) :
	AugBECommon( other.utils ),
	xlink(xlink_)
{	
	ASSERT( utils );
	ASSERT( xlink );
	
	// Partial copy-construct with xlink, we can assume it came from tree
	// Policy: copy in the deps, and add in self as another dependency
	my_deps->CopyAllFrom( *other.my_deps );
	my_deps->AddDep( xlink );	
}


TransformOfAgent::AugBERoaming *TransformOfAgent::AugBERoaming::Clone() const
{
	return new TransformOfAgent::AugBERoaming( *this );
}


TreePtr<Node> TransformOfAgent::AugBERoaming::GetGenericTreePtr() const
{
	return xlink.GetChildTreePtr();
}


XLink TransformOfAgent::AugBERoaming::GetXLink() const
{
	return xlink;
}


const TransformOfAgent::Dependencies &TransformOfAgent::AugBERoaming::GetDeps() const
{
	return *my_deps;
}


AugBEInterface *TransformOfAgent::AugBERoaming::OnGetChild( const TreePtrInterface *other_tree_ptr )
{
	// If we are Tree then construct+return Tree style, otherwise reduce to Free style. This
	// is to stop descendents of Free masquerading as Tree.	
	if( xlink )
	{
		// We're roaming the x tree
		ASSERT( !ON_STACK(other_tree_ptr) );
		// Policy: my_deps will be copied into with the new node, which will add itself
		return new TransformOfAgent::AugBERoaming(*this, utils->db->GetXLink(other_tree_ptr)); // tree
	}
	else
	{
		// We're moving through our free tree - not illegal
		// Policy: my_deps will be copied into the new node
		return new TransformOfAgent::AugBE(*this, (TreePtr<Node>)*other_tree_ptr); // free
	}
}


void TransformOfAgent::AugBERoaming::OnSetChild( const TreePtrInterface *other_tree_ptr, AugBEInterface *new_val )
{
	ASSERTFAIL(); // would modify the x-tree, not allowed
}


void TransformOfAgent::AugBERoaming::OnDepLeak()
{
	// Policy: Leap dumps our deps stright into dest
	// (dest is the resultant dep set for the whole transformation)
	ASSERT( utils );
	utils->GetDeps()->CopyAllFrom( *my_deps );
}


string TransformOfAgent::AugBERoaming::GetTrace() const 
{ 
	return "TODO"; 
}

// ---------------------- TransformOfAgent::AugBE ---------------------------

TransformOfAgent::AugBE::AugBE( TreePtr<Node> generic_tree_ptr_, const TransUtils *utils_ ) :
	AugBECommon( utils_ ),	
	xlink(),
	generic_tree_ptr(generic_tree_ptr_)
{
	// When constructed from TreePtr, we can assume it's new and free
	// We have nothing we can depend on.
	ASSERT( utils );
}


TransformOfAgent::AugBE::AugBE( XLink xlink_, const TransUtils *utils_) :
	AugBECommon( utils_ ),
	xlink(xlink_),
    generic_tree_ptr(xlink.GetChildTreePtr())
{
	ASSERT( utils );
	ASSERT( xlink );

	// When constructed from XLink, we can assume it came from the tree
	// Policy: when constructing from tree, depend on self
	my_deps->AddDep( xlink );			
}


TransformOfAgent::AugBE::AugBE( const AugBECommon &other, TreePtr<Node> generic_tree_ptr_ ) :
	AugBECommon( other.utils ),
	xlink(),
	generic_tree_ptr(generic_tree_ptr_)
{
	// Partial copy-construct with TreePtr, we can assume it's new and free
	// Policy: copy in the deps, but we have nothing to add
	my_deps->CopyAllFrom( *other.my_deps );

	ASSERT( utils );
}


TransformOfAgent::AugBE::AugBE( const AugBECommon &other, XLink xlink_ ) :
	AugBECommon( other.utils ),
	xlink(xlink_),
    generic_tree_ptr(xlink.GetChildTreePtr())
{	
	ASSERT( utils );
	ASSERT( xlink );
	
	// Partial copy-construct with xlink, we can assume it came from tree
	// Policy: copy in the deps, and add in self as another dependency
	my_deps->CopyAllFrom( *other.my_deps );
	my_deps->AddDep( xlink );	
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
		// Policy: my_deps will be copied into with the new node, which will add itself
		return new TransformOfAgent::AugBE(*this, utils->db->GetXLink(other_tree_ptr)); // tree
	}
	else
	{
		// We're moving through our free tree - not illegal
		// Policy: my_deps will be copied into the new node
		return new TransformOfAgent::AugBE(*this, (TreePtr<Node>)*other_tree_ptr); // free
	}
}


void TransformOfAgent::AugBE::OnSetChild( const TreePtrInterface *other_tree_ptr, AugBEInterface *new_val )
{
	ASSERT( utils );
	ASSERT( new_val );
    AugBECommon *n = dynamic_cast<AugBECommon *>(new_val);
    ASSERT( n );
			
	// We're building our free tree OR we're meandering into the x tree
	// Policy: parent indirects to child's deps 
	my_deps->AddChainTo( n->my_deps );
}


void TransformOfAgent::AugBE::OnDepLeak()
{
	// Policy: Leap dumps our deps stright into dest
	// (dest is the resultant dep set for the whole transformation)
	ASSERT( utils );
	utils->GetDeps()->CopyAllFrom( *my_deps );
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
	                        ValuePtr<TransformOfAgent::AugBERoaming>::Make(xlink, this));
}	


ValuePtr<AugBEInterface> TransformOfAgent::TransUtils::CreateBE( TreePtr<Node> tp ) const 
{
	return ValuePtr<TransformOfAgent::AugBE>::Make(tp, this);
}


ValuePtr<TransformOfAgent::AugBE> TransformOfAgent::TransUtils::TryGetBE( const AugTreePtrBase &atp ) const
{
	return ValuePtr<TransformOfAgent::AugBE>::DynamicCast(atp.GetImpl());	
}


ValuePtr<TransformOfAgent::AugBERoaming> TransformOfAgent::TransUtils::TryGetBERoaming( const AugTreePtrBase &atp ) const
{
	return ValuePtr<TransformOfAgent::AugBERoaming>::DynamicCast(atp.GetImpl());	
}


set<AugTreePtr<Node>> TransformOfAgent::TransUtils::GetDeclarers( AugTreePtr<Node> node ) const
{
    if( !db->HasNodeRow(node.GetTreePtr()) ) // not found
        throw TransUtilsInterface::UnknownNode();

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
    if( stimulus_xlink == XLink::MMAX_Link )
         return RelocatingQueryResult(); 
	
	Dependencies deps;
	TransformOfAgent::TransUtils utils(db, &deps);
    TransKit kit { &utils };

    try
    {
		AugTreePtr<Node> stimulus_x = utils.CreateAugTreePtr( stimulus_xlink );	
		
		AugTreePtr<Node> atp = transformation->ApplyTransformation( kit, stimulus_x );  
		
		if( ValuePtr<AugBE> be = utils.TryGetBE(atp) ) // TODO: get common (must succeed), do the deps, then if-else with casts
		{
			// Grab the final deps stored in the ATP. Same as a dep leak, but explicit for clarity.
			deps.CopyAllFrom( be->GetDeps() );
			
			XLink xlink = be->GetXLink(); 
			TreePtr<Node> tp = be->GetGenericTreePtr();		
			ASSERT( tp->IsFinal() )(*this)(" computed non-final ")(tp)(" from ")(stimulus_x)("\n");                				
			if( xlink ) 
				return RelocatingQueryResult( xlink );  // tree      
			else
				return RelocatingQueryResult( tp, deps );  // free 
         }
         else if( ValuePtr<AugBERoaming> be = utils.TryGetBERoaming(atp) )
         {
			// Grab the final deps stored in the ATP. Same as a dep leak, but explicit for clarity.
			deps.CopyAllFrom( be->GetDeps() );
			
			XLink xlink = be->GetXLink(); 
			ASSERT( xlink );
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
	return RelocatingAgent::IsExtenderChannelLess(r);
}


int TransformOfAgent::GetExtenderChannelOrdinal() const
{
	return 1; // TODO class id as an ordinal?
}



