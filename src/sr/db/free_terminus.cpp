#include "free_terminus.hpp"

#include "helpers/flatten.hpp"

using namespace SR;

// ------------------------- FreeTerminus --------------------------    

FreeTerminus::FreeTerminus( TreePtr<Node> parent_node_ ) :
	parent_node( parent_node_ )
{
	ASSERT( parent_node );
}	


TreePtr<Node> FreeTerminus::GetParentNode() const
{
	return parent_node;
}

// ------------------------- FreeSingularTerminus --------------------------    
    
FreeSingularTerminus::FreeSingularTerminus( TreePtr<Node> parent_node, TreePtrInterface *dest_tree_ptr_ ) :
    FreeTerminus( parent_node ),
    dest_tree_ptr( dest_tree_ptr_ )
{
}


void FreeSingularTerminus::Populate( TreePtr<Node> child_base, 
                                 list<shared_ptr<FreeTerminus>> child_terminii )
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
    

const TreePtrInterface *FreeSingularTerminus::GetTreePtrInterface() const
{	
	ASSERT( dest_tree_ptr );
	return dest_tree_ptr;
}  


string FreeSingularTerminus::GetTrace() const
{
    return "⌾"+dest_tree_ptr->GetTypeName();
}
    
// ------------------------- FreeContainerTerminus --------------------------    
    
FreeContainerTerminus::FreeContainerTerminus( TreePtr<Node> parent_node, 
                                      ContainerInterface *dest_container_,
                                      ContainerInterface::iterator it_dest_placeholder_ ) :
    FreeTerminus( parent_node ),
    dest_container( dest_container_ ),
    it_dest_placeholder( it_dest_placeholder_ ),
    it_dest_populated( dest_container->end() )
{
    Validate();
}


FreeContainerTerminus &FreeContainerTerminus::operator=( const FreeContainerTerminus &other )
{
	dest_container = other.dest_container;
	it_dest_placeholder = other.it_dest_placeholder;
	Validate();
	return *this;
}


void FreeContainerTerminus::Populate( TreePtr<Node> child_base, 
                                  list<shared_ptr<FreeTerminus>> child_terminii )
{
	ASSERT( child_base ); // perhaps we tried to populate with an empty zone?
    ASSERT( !populated );
    populated = true;

    // We don't need the placeholder any more
	ContainerInterface::iterator it_after = dest_container->erase( it_dest_placeholder );
	
    if( ContainerInterface *child_container = dynamic_cast<ContainerInterface *>(child_base.get()) )
    {            		
        // Child zone base has ContainerInterface, so it's a SubContainer. We get here due to         
        // FreeZones created by StarAgent. Expand it and populate into the destination, which is also a SubContainer.         
        for( ContainerInterface::iterator it_child = child_container->begin();
			 it_child != child_container->end();
			 ++it_child	)
        {
			TreePtr<Node> child_element = (TreePtr<Node>)*it_child; 
			ContainerInterface::iterator it_new = dest_container->insert( it_after, child_element ); // inserts before it_after

			if( child_element == MakePlaceholder() ) 
			{
				// If child_element is a placeholder, the child FZ terminates immediately at this element.
				// So child_element *IS* the placeholder of that FZ's FreeTerminus instance and child_container IS
				// the FZ base container. We need to build a new terminus for the FZ that uses our dest 
				// container because that's what will be kept.
				// Note: Kept: our container, child terminii
				// Discarded: this terminus, child base, child container
				shared_ptr<FreeContainerTerminus> child_con_terminus = FindMatchingTerminus( child_container, it_child, child_terminii );												
				*child_con_terminus = FreeContainerTerminus(GetParentNode(), dest_container, it_new);							
			}
        }                                    
    }
    else
    {
        // Populate terminus with singular-based zone.
        ASSERT( child_base );
        dest_container->insert( it_after, child_base ); // inserts before it_after
        it_dest_populated = it_after;
        --it_dest_populated;
    }    
}


TreePtr<Node> FreeContainerTerminus::MakePlaceholder()
{
    return TreePtr<Node>(); // It's just a NULL tree ptr!
}


shared_ptr<FreeContainerTerminus> FreeContainerTerminus::FindMatchingTerminus( ContainerInterface *container,
                                                                       ContainerInterface::iterator it_placeholder,
                                                                       list<shared_ptr<FreeTerminus>> &candidate_terminii )
{
	shared_ptr<FreeContainerTerminus> found_terminus;

	for( shared_ptr<FreeTerminus> candidate_terminus : candidate_terminii )
	{
		if( auto candidate_container_terminus = dynamic_pointer_cast<FreeContainerTerminus>( candidate_terminus ) ) 
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


const TreePtrInterface *FreeContainerTerminus::GetTreePtrInterface() const
{	
	// Must have populated, and not with a SubContainer
	ASSERT( it_dest_populated != dest_container->end() ); 
	
	const TreePtrInterface *dest_tree_ptr = &*it_dest_populated;
	ASSERT( dest_tree_ptr );	
	return dest_tree_ptr;
}                                     


void FreeContainerTerminus::Validate() const
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


string FreeContainerTerminus::GetTrace() const
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
    return "⌾"+dest_container->GetTypeName()+"["+si+" of "+to_string(dest_container->size())+"]";
}
    
