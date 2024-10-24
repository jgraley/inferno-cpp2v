#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "walk.hpp"

#include "flatten.hpp"
#include "transformation.hpp"

// ---------------------- AugTreePtrBase ---------------------------

AugTreePtrBase::AugTreePtrBase() :
	tree_ptr(nullptr),
	p_tree_ptr(nullptr),
	dep_rep( nullptr )	
{
}


AugTreePtrBase::AugTreePtrBase( TreePtr<Node> tree_ptr_ ) :
	tree_ptr(tree_ptr_),
	p_tree_ptr(nullptr),
	dep_rep( nullptr )	
{
	ASSERT( tree_ptr );
}


AugTreePtrBase::AugTreePtrBase(const TreePtrInterface *p_tree_ptr_, DependencyReporter *dep_rep_) : // tree
    tree_ptr(*p_tree_ptr_),
	p_tree_ptr(p_tree_ptr_),
	dep_rep( dep_rep_ )
{
	ASSERT( tree_ptr );
	ASSERT( p_tree_ptr );
	ASSERT( *p_tree_ptr );
	// Not a local automatic please, we're going to hang on to it.
	ASSERT( !ON_STACK(p_tree_ptr_) );	

    if( dep_rep )
		dep_rep->ReportTreeNode( tree_ptr );	
}    


TreePtr<Node> AugTreePtrBase::GetTreePtr() const
{
	return tree_ptr;
}


void AugTreePtrBase::SetTreePtr(TreePtr<Node> tree_ptr_)
{
	tree_ptr = tree_ptr_;
}


AugTreePtrBase::operator bool()
{
    return GetTreePtr(); 
}


// ---------------------- TreeUtils ---------------------------

TreeUtils::TreeUtils( const NavigationInterface *nav_, DependencyReporter *dep_rep_ ) :
	nav(nav_),
	dep_rep(dep_rep_)
{
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

/*
set<AugTreePtr<Node>> TreeUtils::GetParents( AugTreePtr<Node> atp ) const
{
	ASSERTFAIL(); // TODO
	set<TreeUtils::LinkInfo> li_set = GetParents( atp.tree_ptr );
}


set<AugTreePtr<Node>> TreeUtils::GetDeclarers( AugTreePtr<Node> node ) const
{
	ASSERTFAIL(); // TODO
}
*/

const TreePtrInterface *TreeUtils::GetPTreePtr( const AugTreePtrBase &atp ) const
{
	return atp.p_tree_ptr;
}


TreePtr<Node> TreeUtils::GetTreePtr( const AugTreePtrBase &atp ) const
{
	return atp.tree_ptr;
}


DependencyReporter *TreeUtils::GetDepRep() const
{
	return dep_rep;
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


AugTreePtr<Node> Transformation::operator()( AugTreePtr<Node> atp, 
    		                                 TreePtr<Node> root	) const
{
    SimpleNavigation nav(root);
    TreeUtils utils(&nav);
    TreeKit kit { &utils };
    return ApplyTransformation( kit, atp );
}
