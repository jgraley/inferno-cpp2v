#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "walk.hpp"

#include "flatten.hpp"
#include "transformation.hpp"

// ---------------------- AugTreePtrBase ---------------------------

AugTreePtrBase::AugTreePtrBase() :
	impl(nullptr)
{
}


AugTreePtrBase::AugTreePtrBase( ValuePtr<AugBEInterface> &&impl_ ) :
    impl( move(impl_) )
{
}


ValuePtr<AugBEInterface> AugTreePtrBase::GetImpl() const
{
	ASSERT( impl );
	return impl;
}


AugTreePtrBase AugTreePtrBase::OnGetChild( const TreePtrInterface *other_tree_ptr ) const
{
	if( impl )
	{
		ValuePtr<AugBEInterface> child_impl( impl->OnGetChild(other_tree_ptr) );
		return AugTreePtrBase(move(child_impl)); 
	}
	else
	{
		return AugTreePtrBase();
	}
}


void AugTreePtrBase::OnSetChild( const TreePtrInterface *other_tree_ptr, AugTreePtrBase new_val ) const
{
	if( impl )
		impl->OnSetChild(other_tree_ptr, new_val.impl.get());
}


void AugTreePtrBase::OnDepLeak() const
{
	if( impl )
		impl->OnDepLeak();	
}

// ---------------------- DefaultNavigation ---------------------------

DefaultNavigation::DefaultNavigation( TreePtr<Node> root_ ) :
	root( root_ )
{
}


set<NavigationInterface::LinkInfo> DefaultNavigation::GetParents( TreePtr<Node> node ) const
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


set<NavigationInterface::LinkInfo> DefaultNavigation::GetDeclarers( TreePtr<Node> node ) const
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

// ---------------------- DefaultTransUtils ---------------------------

DefaultTransUtils::DefaultTransUtils( const NavigationInterface *nav_ ) :
	nav(nav_)
{
}	


ValuePtr<AugBEInterface> DefaultTransUtils::CreateBE( TreePtr<Node> tp ) const 
{
	return nullptr;
}


set<AugTreePtr<Node>> DefaultTransUtils::GetDeclarers( AugTreePtr<Node> node ) const
{
    set<NavigationInterface::LinkInfo> declarer_infos = nav->GetDeclarers( node.GetTreePtr() );  
    
    // Generate ATPs from declarers
	set<AugTreePtr<Node>> atp_declarers;	
    for( NavigationInterface::LinkInfo declarer : declarer_infos )
		atp_declarers.insert( AugTreePtr<Node>(declarer.first) );
	
	return atp_declarers;
}

// ---------------------- Transformation ---------------------------

AugTreePtr<Node> Transformation::operator()( AugTreePtr<Node> atp, 
    		                                 TreePtr<Node> root	) const
{
    DefaultNavigation nav(root);
    DefaultTransUtils utils(&nav);
    TransKit kit { &utils };
    return ApplyTransformation( kit, atp );
}

