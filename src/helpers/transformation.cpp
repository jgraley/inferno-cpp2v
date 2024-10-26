#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "walk.hpp"

#include "flatten.hpp"
#include "transformation.hpp"

// ---------------------- AugTreePtrImpl ---------------------------

AugTreePtrImpl::AugTreePtrImpl()
{
}


AugTreePtrImpl::AugTreePtrImpl( TreePtr<Node> generic_tree_ptr_ )
{
}


AugTreePtrImpl::AugTreePtrImpl( const TreePtrInterface *p_tree_ptr_, DependencyReporter *dep_rep_ )
{
}


AugTreePtrImpl::AugTreePtrImpl( const AugTreePtrImpl &other, TreePtr<Node> generic_tree_ptr_ )
{
}

// ---------------------- AugTreePtrBase ---------------------------

AugTreePtrBase::AugTreePtrBase() :
	impl(nullptr),
	generic_tree_ptr(nullptr),
	p_tree_ptr(nullptr),
	dep_rep( nullptr )	
{
}


AugTreePtrBase::AugTreePtrBase( TreePtr<Node> generic_tree_ptr_ ) :
	impl( new AugTreePtrImpl(generic_tree_ptr_) ),
	generic_tree_ptr(generic_tree_ptr_),
	p_tree_ptr(nullptr),
	dep_rep( nullptr )	
{
	ASSERT( generic_tree_ptr );
}


AugTreePtrBase::AugTreePtrBase(const TreePtrInterface *p_tree_ptr_, DependencyReporter *dep_rep_) : // tree
	impl( new AugTreePtrImpl(p_tree_ptr_, dep_rep_) ),
    generic_tree_ptr(*p_tree_ptr_),
	p_tree_ptr(p_tree_ptr_),
	dep_rep( dep_rep_ )
{
	ASSERT( generic_tree_ptr );
	ASSERT( p_tree_ptr );
	ASSERT( *p_tree_ptr );
	// Not a local automatic please, we're going to hang on to it.
	ASSERT( !ON_STACK(p_tree_ptr_) );	

    if( dep_rep )
		dep_rep->ReportTreeNode( generic_tree_ptr );	
}    


AugTreePtrBase::AugTreePtrBase( const AugTreePtrBase &other, TreePtr<Node> generic_tree_ptr_ ) :
	impl( new AugTreePtrImpl(*other.impl, generic_tree_ptr_) ),
    generic_tree_ptr(generic_tree_ptr_),
	p_tree_ptr(other.p_tree_ptr),
	dep_rep(other.dep_rep)
{	
}


AugTreePtrBase::AugTreePtrBase( const AugTreePtrBase &other ) :
	impl( new AugTreePtrImpl(*other.impl) ),
    generic_tree_ptr(other.generic_tree_ptr),
	p_tree_ptr(other.p_tree_ptr),
	dep_rep(other.dep_rep)
{	
}


AugTreePtrBase &AugTreePtrBase::operator=(const AugTreePtrBase &other)
{
	impl = make_unique<AugTreePtrImpl>(*other.impl);
    generic_tree_ptr = other.generic_tree_ptr;
	p_tree_ptr = other.p_tree_ptr;
	dep_rep = other.dep_rep;
	return *this;
}



TreePtr<Node> AugTreePtrBase::GetGenericTreePtr() const
{
	return generic_tree_ptr;
}

// ---------------------- TreeUtils ---------------------------

TreeUtils::TreeUtils( const NavigationInterface *nav_, DependencyReporter *dep_rep_ ) :
	nav(nav_),
	dep_rep(dep_rep_)
{
}	


AugTreePtr<Node> TreeUtils::CreateAugTreePtr(const TreePtrInterface *p_tree_ptr) const
{
	return AugTreePtr<Node>((TreePtr<Node>)*p_tree_ptr, AugTreePtrBase(p_tree_ptr, dep_rep));
}	


const TreePtrInterface *TreeUtils::GetPTreePtr( const AugTreePtrBase &atp ) const
{
	return atp.p_tree_ptr; // TODO eg atp.GetImpl()->GetPTreePtr()
	// but then later, transformation_agent can do eg atp.GetImpl()->GetXLink() etc
	// possibly with a dyncast to an API that knows about XLinks 
}


TreePtr<Node> TreeUtils::GetGenericTreePtr( const AugTreePtrBase &atp ) const
{
	return atp.generic_tree_ptr; // TODO eg atp.GetImpl()->GetGenericTreePtr()
}


bool TreeUtils::IsRequireReports() const
{
	return nav->IsRequireReports();
}


set<TreeUtils::LinkInfo> TreeUtils::GetParents( TreePtr<Node> node ) const
{
	return nav->GetParents(node); // TODO convert to a set of AugTreePtr
}


set<TreeUtils::LinkInfo> TreeUtils::GetDeclarers( TreePtr<Node> node ) const
{
	return nav->GetDeclarers(node); // TODO convert to a set of AugTreePtr
} 


set<AugTreePtr<Node>> TreeUtils::GetDeclarers( AugTreePtr<Node> node ) const
{
    set<NavigationInterface::LinkInfo> declarer_infos = GetDeclarers( node.GetTreePtr() );  
    
    // Generate ATPs from declarers
	set<AugTreePtr<Node>> atp_declarers;	
    for( NavigationInterface::LinkInfo declarer : declarer_infos )
    {   
		// To be able to report the declarer as a node in the tree, we
		// must find its parent link
		set<NavigationInterface::LinkInfo> parent_infos = GetParents( declarer.first );
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


DependencyReporter *TreeUtils::GetDepRep() const
{
	return dep_rep;
}

// ---------------------- Transformation ---------------------------

AugTreePtr<Node> Transformation::operator()( AugTreePtr<Node> atp, 
    		                                 TreePtr<Node> root	) const
{
    SimpleNavigation nav(root);
    TreeUtils utils(&nav);
    TreeKit kit { &utils };
    return ApplyTransformation( kit, atp );
}

// ---------------------- SimpleNavigation ---------------------------

SimpleNavigation::SimpleNavigation( TreePtr<Node> root_ ) :
	root( root_ )
{
}

	
bool SimpleNavigation::IsRequireReports() const
{
    return false; // No we don't, we're just the reference one
}    


set<NavigationInterface::LinkInfo> SimpleNavigation::GetParents( TreePtr<Node> node ) const
{
	set<LinkInfo> infos;
	
	Walk w(root, nullptr, nullptr);
	for( const TreePtrInterface &n : w )
	{
        FlattenNode flat( node );
        for(const TreePtrInterface &n : flat )
		{
            if( node == (TreePtr<Node>)( n ) )
            {
				LinkInfo info( (TreePtr<Node>)n, &n );
				infos.insert( info );
			}	            
		}
	}
	
	return infos;
}


set<NavigationInterface::LinkInfo> SimpleNavigation::GetDeclarers( TreePtr<Node> node ) const
{
	set<LinkInfo> infos;
	
	Walk w(root, nullptr, nullptr);
	for( const TreePtrInterface &n : w )
	{
		set<const TreePtrInterface *> declared = ((TreePtr<Node>)n)->GetDeclared();
		for( const TreePtrInterface *pd : declared )
		{
            if( node == (TreePtr<Node>)( *pd ) )
            {
				LinkInfo info( (TreePtr<Node>)n, pd );
				infos.insert( info );
			}	            
		}
	}
	
	return infos;
}
