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
	// Not a local automatic please, we're going to hang on to it.
	ASSERT( !ON_STACK(other_tree_ptr) );
	
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

// ---------------------- DefaultTransUtils ---------------------------

DefaultTransUtils::DefaultTransUtils( TreePtr<Node> root_ ) :
	root( root_ )
{
}	


ValuePtr<AugBEInterface> DefaultTransUtils::CreateBE( TreePtr<Node> tp ) const 
{
	return nullptr;
}


set<AugTreePtr<Node>> DefaultTransUtils::GetDeclarers( AugTreePtr<Node> node ) const
{
    set<DefaultTransUtils::LinkInfo> declarer_infos = GetDeclarers( node.GetTreePtr() );  
    
    // Generate ATPs from declarers
	set<AugTreePtr<Node>> atp_declarers;	
    for( LinkInfo declarer : declarer_infos )
		atp_declarers.insert( AugTreePtr<Node>(declarer.first) );
	
	return atp_declarers;
}


set<DefaultTransUtils::LinkInfo> DefaultTransUtils::GetParents( TreePtr<Node> node ) const
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


set<DefaultTransUtils::LinkInfo> DefaultTransUtils::GetDeclarers( TreePtr<Node> node ) const
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


// ---------------------- Transformation ---------------------------

AugTreePtr<Node> Transformation::operator()( AugTreePtr<Node> atp, 
    		                                 TreePtr<Node> root	) const
{
    DefaultTransUtils utils(root);
    TransKit kit { &utils };
    return TryApplyTransformation( kit, atp );
}


AugTreePtr<Node> Transformation::ApplyTransformation( const TransKit &kit, // Handy functions
                                                      AugTreePtr<Node> node ) const    // Root of the subtree we want to modify    		                          
{
	AugTreePtr<Node> n = TryApplyTransformation(kit, node);
	if( !n )
		throw TransformationFailedMismatch();
	else
		return n;
}                                      

