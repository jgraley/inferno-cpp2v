#include "terminus.hpp"

#include "helpers/flatten.hpp"

using namespace SR;

// ------------------------- SingularTerminus --------------------------    
    
SingularTerminus::SingularTerminus( TreePtrInterface *dest_tree_ptr_ ) :
    dest_tree_ptr( dest_tree_ptr_ )
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
		*dest_tree_ptr = child_base;
	}
    TRACE("Singular joined ")(child_base)("\n");    
}
    

string SingularTerminus::GetTrace() const
{
    return "⌾"+dest_tree_ptr->GetTypeName();
}
    
// ------------------------- ContainerTerminus --------------------------    
    
ContainerTerminus::ContainerTerminus( ContainerInterface *dest_container_,
                                      ContainerInterface::iterator it_dest_placeholder_ ) :
    dest_container( dest_container_ ),
    it_dest_placeholder( it_dest_placeholder_ )
{
    Validate();
}


ContainerTerminus &ContainerTerminus::operator=( const ContainerTerminus &other )
{
	dest_container = other.dest_container;
	it_dest_placeholder = other.it_dest_placeholder;
	Validate();
	return *this;
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
        
        for( ContainerInterface::iterator it_child = child_container->begin();
			 it_child != child_container->end();
			 ++it_child	)
        {
			TreePtr<Node> child_element = (TreePtr<Node>)*it_child; 
			if( child_element ) 
			{
				// If it's non-NULL, the StarAgent's FZ was previously populated. If it has terminii, they
				// are deeper down in the tree, and can just be reused as-is. 
				dest_container->insert( *it_after.GetUnderlyingIterator(), child_element ); 
			}
			else
			{
				// If it's NULL, nothing has acted on the StarAgent's FZ and so it terminates immediately.
				// That means it's the placeholder of that FZ's Terminus instance and child_container is
				// its container. We need to give that terminus our container and a new placeholder.
				shared_ptr<ContainerTerminus> child_con_terminus;
				for( shared_ptr<Terminus> child_terminus : child_terminii )
				{
					if( auto cct = dynamic_pointer_cast<ContainerTerminus>( child_terminus ) ) 
					{						
						if( cct->dest_container == child_container && 
						    cct->it_dest_placeholder == it_child )
						{
							ASSERT( !child_con_terminus )("Found multiple matching terminii including ")(child_con_terminus)(" and now ")(cct);
							child_con_terminus = cct;
						}
					}
				}
				ASSERT( child_con_terminus );
								
				dest_container->insert( *it_after.GetUnderlyingIterator(), GetPlaceholder() ); 
				
				ContainerInterface::iterator it_new_placeholder = it_after;
				--it_new_placeholder; // back up to the newly inserted placeholder
				*child_con_terminus = ContainerTerminus(dest_container, it_new_placeholder);							
			}
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


void ContainerTerminus::Validate() const
{	
    // important invariant: placeholder iterator must point to a member in the destination container
    ASSERT( it_dest_placeholder != dest_container->end() );
    bool found = false;
    for( ContainerInterface::iterator it=dest_container->begin(); it!=dest_container->end(); ++it )
    {
		if( it == it_dest_placeholder )
			found = true;		
	}
	ASSERT( found );
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
    
