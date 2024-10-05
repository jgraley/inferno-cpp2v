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

    // We don't need the placeholder any more
	ContainerInterface::iterator it_after = dest_container->erase2( it_dest_placeholder );
	
    if( ContainerInterface *child_container = dynamic_cast<ContainerInterface *>(child_base.get()) )
    {            		
        // Child zone base has ContainerInterface, so it's a SubContainer. We get here due to         
        // FreeZones created by StarAgent. Expand it and populate into the destination, which is also a SubContainer.         
        for( ContainerInterface::iterator it_child = child_container->begin();
			 it_child != child_container->end();
			 ++it_child	)
        {
			TreePtr<Node> child_element = (TreePtr<Node>)*it_child; 
			dest_container->insert( *it_after.GetUnderlyingIterator(), child_element ); // inserts before it_after

			if( !child_element ) 
			{
				// If child_element is NULL, the StarAgent's FZ terminates immediately at this element.
				// That means it's the placeholder of that FZ's Terminus instance and child_container is
				// the FZ base container. We need to build a new terminus for the FZ.
				shared_ptr<ContainerTerminus> child_con_terminus = FindMatchingTerminus( child_container, it_child, child_terminii );
												
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
}


TreePtr<Node> ContainerTerminus::MakePlaceholder()
{
    return TreePtr<Node>(); // It's just a NULL tree ptr!
}


shared_ptr<ContainerTerminus> ContainerTerminus::FindMatchingTerminus( ContainerInterface *container,
                                                                       ContainerInterface::iterator it_placeholder,
                                                                       list<shared_ptr<Terminus>> &candidate_terminii )
{
	shared_ptr<ContainerTerminus> found_terminus;

	for( shared_ptr<Terminus> candidate_terminus : candidate_terminii )
	{
		if( auto candidate_container_terminus = dynamic_pointer_cast<ContainerTerminus>( candidate_terminus ) ) 
		{						
			if( candidate_container_terminus->dest_container == container && 
				candidate_container_terminus->it_dest_placeholder == it_placeholder )
			{
				ASSERTS( !found_terminus )("Found multiple matching terminii including ")(*found_terminus)(" and now ")(*candidate_container_terminus);
				found_terminus = candidate_container_terminus;
			}
		}
	}
	
	ASSERTS( found_terminus );
	return found_terminus;
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
    
