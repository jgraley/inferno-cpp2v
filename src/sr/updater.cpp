#include "updater.hpp"

#include "helpers/flatten.hpp"

using namespace SR;

// ------------------------- SingularUpdater --------------------------    
    
SingularUpdater::SingularUpdater( TreePtrInterface *tree_ptr_ ) :
    tree_ptr( tree_ptr_ )
{
}


void SingularUpdater::Insert( TreePtr<Node> node ) const
{
    *tree_ptr = node;
}
    

string SingularUpdater::GetTrace() const
{
    return Trace(tree_ptr);
}
    
// ------------------------- ContainerUpdater --------------------------    
    
ContainerUpdater::ContainerUpdater( ContainerInterface *container_ ) :
    container( container_ )
{
}


void ContainerUpdater::Insert( TreePtr<Node> node ) const
{
    (void)container->insert( node ); 
}


string ContainerUpdater::GetTrace() const
{
    return Trace(container);
}
    