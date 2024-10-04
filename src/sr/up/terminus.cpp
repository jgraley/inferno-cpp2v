#include "terminus.hpp"

#include "helpers/flatten.hpp"

using namespace SR;

// ------------------------- SingularTerminus --------------------------    
    
SingularTerminus::SingularTerminus( TreePtrInterface *tree_ptr_ ) :
    tree_ptr( tree_ptr_ )
{
}


void SingularTerminus::Populate( TreePtr<Node> child_base, 
                                 list<shared_ptr<Terminus>> child_terminii )
{
	ASSERT( child_base ); // perhaps we tried to populate with an empty zone?

	if( ContainerInterface *child_container = dynamic_cast<ContainerInterface *>(child_base.get()) )
	{
		ASSERTFAIL();
	}
	else
	{
		*tree_ptr = child_base;
	}
    TRACE("Singular joined ")(child_base)("\n");    
}
    

string SingularTerminus::GetTrace() const
{
    return "⌾"+tree_ptr->GetTypeName();
}
    
// ------------------------- ContainerTerminus --------------------------    
    
ContainerTerminus::ContainerTerminus( ContainerInterface *dest_container_,
                                      ContainerInterface::iterator it_dest_placeholder_ ) :
    dest_container( dest_container_ ),
    it_dest_placeholder( it_dest_placeholder_ )
{
    ASSERT( it_dest_placeholder != dest_container->end() );
	bool found = false;
    for( ContainerInterface::iterator it=dest_container->begin(); it!=dest_container->end(); ++it )
    {
		if( it == it_dest_placeholder )
			found = true;		
	}
	ASSERT( found );
}


void ContainerTerminus::Populate( TreePtr<Node> child_base, 
                                  list<shared_ptr<Terminus>> child_terminii )
{
	ASSERT( child_base ); // perhaps we tried to populate with an empty zone?
    ASSERT( !joined );
    joined = true;

	ContainerInterface::iterator it_after = it_dest_placeholder;

    // C++ insert() has "insert before" semantics, see https://cplusplus.com/reference/list/list/insert/
    // (this is logical: there are n+1 possible insert positions and n+1 iterator values if you include end())
    // so "insert before" the end, in order to preserve ordering.
	++it_after; // Can be end(), I think this is OK.
    if( ContainerInterface *child_container = dynamic_cast<ContainerInterface *>(child_base.get()) )
    {            
        // Child zone base has ContainerInterface, so it's a SubContainer. We get here due to 
        // FreeZones created by StarAgent. Expand it and populate into the destination, which is also a SubContainer. 
        for( const TreePtrInterface &child_element : *child_container )
        {
            ASSERT( (TreePtr<Node>)child_element )("UNTESTED NULL element in supplied subcontainer: ")(*child_container);
            // If it's non-NULL, the StarAgent's FZ was previously populated. If it has terminii, they
            // are deeper down in the tree, and can just be reused as-is. 
            // If it's NULL, nothing has acted on the StarAgent's FZ and so it terminates immediately.
            // That means it's the placeholder of some OTHER Terminus instance and operand_sub_con is
            // its container. We need to give that terminus our container and a new placeholder.
            dest_container->insert( *it_after.GetUnderlyingIterator(), (TreePtr<Node>)child_element ); 
        }                                    
    }
    else
    {
        // Populate terminus with singular-based zone.
        ASSERT( child_base );
        dest_container->insert( *it_after.GetUnderlyingIterator(), child_base ); 
    }
    
    // We don't need the placeholder any more
    dest_container->erase( *it_dest_placeholder.GetUnderlyingIterator() );  
    
    /**
     * Why all this complicated placeholder business then?
     * It's to permit multiple terminii to refer to the same Sequence
     * with a well-defined relative order. We place a null element in
     * for each terminus so that iterators relative to different
     * terminii have different values.
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
    for( ContainerInterface::iterator it=dest_container->begin(); it!=dest_container->end(); ++it )
    {
		i++;
		if( it == it_dest_placeholder )
			break;
	}
    string si;
    if( i==-1 )
		si = "ERROR!";
	else
		si = to_string(i);
    return "⌾"+dest_container->GetTypeName()+"["+si+" of "+to_string(dest_container->size())+" in dest]";
}
    
