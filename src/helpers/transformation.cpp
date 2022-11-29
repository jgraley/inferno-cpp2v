#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "walk.hpp"

#include "flatten.hpp"
#include "transformation.hpp"

TreeKit::~TreeKit()
{
}


ReferenceTreeKit::ReferenceTreeKit( TreePtr<Node> context_ ) :
	context( context_ )
{
}

	
bool ReferenceTreeKit::IsRequireReports() const
{
    return false; // No we don't, we're just the reference one
}    


set<ReferenceTreeKit::LinkInfo> ReferenceTreeKit::GetParents( TreePtr<Node> node ) const
{
	set<LinkInfo> infos;
	
	Walk w(context, nullptr, nullptr);
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


set<ReferenceTreeKit::LinkInfo> ReferenceTreeKit::GetDeclarers( TreePtr<Node> node ) const
{
	set<LinkInfo> infos;
	
	Walk w(context, nullptr, nullptr);
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
