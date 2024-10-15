#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "walk.hpp"

#include "flatten.hpp"
#include "transformation.hpp"

// ---------------------- NavigationUtils ---------------------------

NavigationUtils::NavigationUtils( const NavigationUtilsImpl *impl_ ) :
	impl(impl_)
{
}	

bool NavigationUtils::IsRequireReports() const
{
	return impl->IsRequireReports();
}


set<NavigationUtils::LinkInfo> NavigationUtils::GetParents( TreePtr<Node> node ) const
{
	return impl->GetParents(node); // TODO convert to a set of AugTreePtr
}


set<NavigationUtils::LinkInfo> NavigationUtils::GetDeclarers( TreePtr<Node> node ) const
{
	return impl->GetDeclarers(node); // TODO convert to a set of AugTreePtr
} 


// ---------------------- Transformation ---------------------------

const TreePtrInterface *Transformation::GetPTreePtr( const AugTreePtrBase &atp )
{
	return atp.p_tree_ptr;
}

// ---------------------- ReferenceNavigationUtilsImpl ---------------------------

ReferenceNavigationUtilsImpl::ReferenceNavigationUtilsImpl( TreePtr<Node> root_ ) :
	root( root_ )
{
}

	
bool ReferenceNavigationUtilsImpl::IsRequireReports() const
{
    return false; // No we don't, we're just the reference one
}    


set<NavigationUtilsImpl::LinkInfo> ReferenceNavigationUtilsImpl::GetParents( TreePtr<Node> node ) const
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


set<NavigationUtilsImpl::LinkInfo> ReferenceNavigationUtilsImpl::GetDeclarers( TreePtr<Node> node ) const
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


AugTreePtr<Node> Transformation::operator()( TreePtr<Node> node, 
    		                                 TreePtr<Node> root	) const
{
    ReferenceNavigationUtilsImpl nav_impl(root);
    NavigationUtils nav(&nav_impl);
    TreeKit kit { &nav };
    return ApplyTransformation( kit, node );
}
