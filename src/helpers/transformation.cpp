#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "walk.hpp"

#include "transformation.hpp"

ReferenceTreeKit::ReferenceTreeKit( TreePtr<Node> context_ ) :
	context( context_ )
{
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
