#include "subcontainers.hpp"

#include "common/common.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "node/node.hpp"    
    
using namespace SR;    
    


SubSequenceRange::SubSequenceRange( TreePtr<Node> parent_x, const iterator &b, const iterator &e ) : 
    SubContainerRange( parent_x ),
    my_begin(b.Clone()), 
    my_end(e.Clone())
{           
    if( !(*my_begin == *my_end) )
        ASSERT_NOT_ON_STACK( &*(*my_begin) )( *this ); 
}


const SubSequenceRange::iterator_interface &SubSequenceRange::begin() 
{ 
    return *my_begin; 
}


const SubSequenceRange::iterator_interface &SubSequenceRange::end()
{
    return *my_end; 
}
