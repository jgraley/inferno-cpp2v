#include "transform_of_agent.hpp"
#include "../scr_engine.hpp"
#include "link.hpp"
#include "db/x_tree_database.hpp"

using namespace SR;

// ---------------------- TransformOfAgent::AugBE ---------------------------

TransformOfAgent::AugBE::AugBE() :
	generic_tree_ptr(nullptr),
	p_tree_ptr(nullptr),
	dest_deps( nullptr )	
{
}


TransformOfAgent::AugBE::AugBE( TreePtr<Node> generic_tree_ptr_ ) :
	generic_tree_ptr(generic_tree_ptr_),
	p_tree_ptr(nullptr),
	dest_deps( nullptr )	
{
}


TransformOfAgent::AugBE::AugBE( const TreePtrInterface *p_tree_ptr_, Dependencies *dest_deps_ ) :
    generic_tree_ptr(*p_tree_ptr_),
	p_tree_ptr(p_tree_ptr_),
	dest_deps( dest_deps_ )
{
/*	ASSERT( generic_tree_ptr );
	ASSERT( p_tree_ptr );
	ASSERT( *p_tree_ptr );
	// Not a local automatic please, we're going to hang on to it.
	ASSERT( !ON_STACK(p_tree_ptr_) );	
*/
    if( dest_deps )
		dest_deps->AddTreeNode( generic_tree_ptr );	
}


TransformOfAgent::AugBE *TransformOfAgent::AugBE::Clone() const
{
	return new TransformOfAgent::AugBE( *this );
}


TreePtr<Node> TransformOfAgent::AugBE::GetGenericTreePtr() const
{
	return generic_tree_ptr;
}


const TreePtrInterface *TransformOfAgent::AugBE::GetPTreePtr() const
{
	return p_tree_ptr;
}


TransformOfAgent::AugBE *TransformOfAgent::AugBE::OnGetChild( const TreePtrInterface *other_tree_ptr )
{
	// If we are Tree then construct+return Tree style, otherwise reduce to Free style. This
	// is to stop descendents of Free masquerading as Tree.	
	if( p_tree_ptr )
	{
		ASSERT( !ON_STACK(other_tree_ptr) );
		return new TransformOfAgent::AugBE(other_tree_ptr, dest_deps); // tree
	}
	else
	{
		return new TransformOfAgent::AugBE((TreePtr<Node>)*other_tree_ptr); // free
	}
}


void TransformOfAgent::AugBE::OnSetChild( const TreePtrInterface *other_tree_ptr, AugBEInterface *new_val )
{
    auto n = GET_THAT_POINTER(new_val);

	// If we are Tree then construct+return Tree style, otherwise reduce to Free style. This
	// is to stop descendents of Free masquerading as Tree.	
	if( p_tree_ptr )
	{
		ASSERT(n->p_tree_ptr); // can't have tree style -> free style: would modify tree
		ASSERT( !ON_STACK(other_tree_ptr) );
	}
	else if( n->p_tree_ptr )
	{
		// TODO add a terminus to free zone
	}
}

void TransformOfAgent::AugBE::OnDepLeak()
{
	// TODO report all our deps
}

// ---------------------- TransformOfAgent::TransUtils ---------------------------

TransformOfAgent::TransUtils::TransUtils( const NavigationInterface *nav_, Dependencies *deps_ ) :
	nav(nav_),
	deps(deps_)
{
}	


AugTreePtr<Node> TransformOfAgent::TransUtils::CreateAugTreePtr(const TreePtrInterface *p_tree_ptr) const
{
	return AugTreePtr<Node>((TreePtr<Node>)*p_tree_ptr, 
	                        ValuePtr<TransformOfAgent::AugBE>::Make(p_tree_ptr, deps));
}	


ValuePtr<AugBEInterface> TransformOfAgent::TransUtils::CreateBE( TreePtr<Node> tp ) const 
{
	return ValuePtr<TransformOfAgent::AugBE>::Make(tp);
}


const TreePtrInterface *TransformOfAgent::TransUtils::GetPTreePtr( const AugTreePtrBase &atp ) const
{
	auto be = ValuePtr<TransformOfAgent::AugBE>::DynamicCast(atp.GetImpl());
	return be->GetPTreePtr(); 
}


TreePtr<Node> TransformOfAgent::TransUtils::GetGenericTreePtr( const AugTreePtrBase &atp ) const
{
	auto be = ValuePtr<TransformOfAgent::AugBE>::DynamicCast(atp.GetImpl());
	return be->GetGenericTreePtr();
}


set<AugTreePtr<Node>> TransformOfAgent::TransUtils::GetDeclarers( AugTreePtr<Node> node ) const
{
    set<NavigationInterface::LinkInfo> declarer_infos = nav->GetDeclarers( node.GetTreePtr() );  
    
    // Generate ATPs from declarers
	set<AugTreePtr<Node>> atp_declarers;	
    for( NavigationInterface::LinkInfo declarer : declarer_infos )
    {   
		// To be able to report the declarer as a node in the tree, we
		// must find its parent link
		set<NavigationInterface::LinkInfo> parent_infos = nav->GetParents( declarer.first );
		if( parent_infos.empty() )
		{
			// No parent link found, so we have to assume this is a free subtree
			atp_declarers.insert( AugTreePtr<Node>(declarer.first) );
		}
		else
		{
			const TreePtrInterface *declarer_parent_link = OnlyElementOf( parent_infos ).second;

			// Report and return
			atp_declarers.insert( CreateAugTreePtr(declarer_parent_link) ); 
		}
	}
	
	return atp_declarers;
}


TeleportAgent::Dependencies *TransformOfAgent::TransUtils::GetDepRep() const
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
		AugTreePtr<Node> stimulus_x = utils.CreateAugTreePtr( stimulus_xlink.GetXPtr() );
		AugTreePtr<Node> atp = transformation->ApplyTransformation( kit, stimulus_x );  

		const TreePtrInterface *ptp = utils.GetPTreePtr(atp);
		TreePtr<Node> tp = utils.GetGenericTreePtr(atp);
		ASSERT( tp->IsFinal() )(*this)(" computed non-final ")(tp)(" from ")(stimulus_x)("\n");                
		
        if( ptp ) 
            return make_pair(db->GetXLink( ptp ), nullptr);  // parent was specified      
		else
            return make_pair(XLink(), tp);  // no parent specified 
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

