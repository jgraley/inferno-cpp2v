#include "terminus.hpp"

#include "helpers/flatten.hpp"

using namespace SR;

// ------------------------- SingularTerminus --------------------------    
    
SingularTerminus::SingularTerminus( TreePtrInterface *tree_ptr_ ) :
    tree_ptr( tree_ptr_ )
{
}


void SingularTerminus::PopulateTerminus( TreePtr<Node> node )
{
	ASSERT( node ); // perhaps we tried to populate with an empty zone?
    *tree_ptr = node;
    TRACE("Singular joined ")(node)("\n");    
}
    

string SingularTerminus::GetTrace() const
{
    return "⌾"+tree_ptr->GetTypeName();
}
    
// ------------------------- ContainerTerminus --------------------------    
    
ContainerTerminus::ContainerTerminus( ContainerInterface *container_,
                                      ContainerInterface::iterator it_placeholder_ ) :
    container( container_ ),
    it_placeholder( it_placeholder_ )
{
    ASSERT( it_placeholder != container->end() );
	bool found = false;
    for( ContainerInterface::iterator it=container->begin(); it!=container->end(); ++it )
    {
		if( it == it_placeholder )
			found = true;		
	}
	ASSERT( found );
}


void ContainerTerminus::PopulateTerminus( TreePtr<Node> node )
{
	ASSERT( node ); // perhaps we tried to populate with an empty zone?
    ASSERT( !joined );
    joined = true;

	ContainerInterface::iterator it_after = it_placeholder;

    // C++ insert() has "insert before" semantics, see https://cplusplus.com/reference/list/list/insert/
    // (this is logical: there are n+1 possible insert positions and n+1 iterator values if you include end())
    // so "insert before" the end, in order to preserve ordering.
	++it_after; // Can be end(), I think this is OK.
    if( ContainerInterface *operand_sub_con = dynamic_cast<ContainerInterface *>(node.get()) )
    {            
        // Operand zone base has container interface, so it's a SubContainer. Expand it and populate.
        for( const TreePtrInterface &sub_elt : *operand_sub_con )
        {
            ASSERT( (TreePtr<Node>)sub_elt );
            container->insert( *it_after.GetUnderlyingIterator(), (TreePtr<Node>)sub_elt ); 
        }                                    
    }
    else
    {
        // Populate terminus with singular-based zone.
        ASSERT( node );
        container->insert( *it_after.GetUnderlyingIterator(), node ); 
    }
    
    // We don't need the placeholder any more
    container->erase( *it_placeholder.GetUnderlyingIterator() );  
    
    /**
     * Why all this complicated placeholder business then?
     * It's to permit multiple terminii to refer to the same Sequence
     * with a well-defined relative order. We place a null element in
     * for each terminus so that iterators relative to different
     * terminii have differnt values.
     * 
     * Note: we must not determine the actual insertion iterator
     * (it_after) during construct, because the container is still
     * being filled, and we'll get end() when what we want is the next 
     * element that will be there when we apply the update.
     */     
}


TreePtr<Node> ContainerTerminus::GetPlaceholder()
{
    return TreePtr<Node>(); // It's just a NULL tree ptr!
}


string ContainerTerminus::GetTrace() const
{
	int i=-1;
    for( ContainerInterface::iterator it=container->begin(); it!=container->end(); ++it )
    {
		i++;
		if( it == it_placeholder )
			break;
	}
    string si;
    if( i==-1 )
		si = "ERROR!";
	else
		si = to_string(i);
    return "⌾"+container->GetTypeName()+"["+si+"]";
}
    
