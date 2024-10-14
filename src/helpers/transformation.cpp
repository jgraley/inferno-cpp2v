#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "walk.hpp"

#include "flatten.hpp"
#include "transformation.hpp"


NavigationUtils::~NavigationUtils()
{
}


const TreePtrInterface *Transformation::GetPTreePtr( const AugTreePtrBase &atp )
{
	return atp.p_tree_ptr;
}


ReferenceNavigationUtils::ReferenceNavigationUtils( TreePtr<Node> root_ ) :
	root( root_ )
{
}

	
bool ReferenceNavigationUtils::IsRequireReports() const
{
    return false; // No we don't, we're just the reference one
}    


set<NavigationUtils::LinkInfo> ReferenceNavigationUtils::GetParents( TreePtr<Node> node ) const
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


set<NavigationUtils::LinkInfo> ReferenceNavigationUtils::GetDeclarers( TreePtr<Node> node ) const
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
    ReferenceNavigationUtils nav(root);
    TreeKit kit { &nav };
    return ApplyTransformation( kit, node );
}
