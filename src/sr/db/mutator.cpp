#include "mutator.hpp"

#include "helpers/flatten.hpp"

using namespace SR;

// ------------------------- Mutator --------------------------    

shared_ptr<Mutator> Mutator::MakeRootMutator( shared_ptr<TreePtr<Node>> sp_tp_root_node )
{
	return shared_ptr<Mutator>(new Mutator(Mode::Root, nullptr, nullptr, nullptr, ContainerInterface::iterator(), sp_tp_root_node));
}

										  		  
shared_ptr<Mutator> Mutator::MakeSingularMutator( TreePtr<Node> parent_node, 
										  		  TreePtrInterface *parent_singular )
{
	return shared_ptr<Mutator>(new Mutator(Mode::Singular, parent_node, parent_singular, nullptr, ContainerInterface::iterator(), nullptr));
}

										  		  
shared_ptr<Mutator> Mutator::MakeContainerMutator( TreePtr<Node> parent_node, 
												   ContainerInterface *parent_container,
												   ContainerInterface::iterator container_iterator )
{
	return shared_ptr<Mutator>(new Mutator(Mode::Container, parent_node, nullptr, parent_container, container_iterator, nullptr));
}


Mutator::Mutator( Mode mode_,
                  TreePtr<Node> parent_node_, 
                  TreePtrInterface *parent_singular_, 
                  ContainerInterface *parent_container_,
                  ContainerInterface::iterator container_iterator_,
                  shared_ptr<TreePtr<Node>> sp_tp_root_node_ ) :
	mode( mode_ ),
    parent_node( parent_node_ ),
    parent_singular( parent_singular_ ),
    parent_container( parent_container_ ),
    container_iterator( container_iterator_ ),
    sp_tp_root_node( sp_tp_root_node_ )
{
    Validate();
}


TreePtr<Node> Mutator::ExchangeChild( TreePtr<Node> new_child )
{	
	// Free zone merging should get rid of these, and things will change with general wide zone support
	ASSERT( !dynamic_cast<ContainerInterface *>(new_child.get()) )("Cannot accept wide here");

	switch( mode )
	{
		case Mode::Root:
		{
			TreePtr<Node> old_child = *sp_tp_root_node;
			*sp_tp_root_node = new_child;
			TRACE("Singular mutated ")(new_child)("\n");   
			return old_child;
		}

		case Mode::Singular:
		{
			TreePtr<Node> old_child = (TreePtr<Node>)*parent_singular;
			*parent_singular = new_child;
			TRACE("Singular mutated ")(new_child)("\n");   
			return old_child;
		}
			
		case Mode::Container:
		{
			TreePtr<Node> old_child = (TreePtr<Node>)*container_iterator;
			container_iterator.Mutate(&new_child); 
			TRACE("Container mutated ")(new_child)("\n");   
			return old_child;
		}
		
		default: 
			ASSERTFAIL();
	}	    
}
    
    
TreePtr<Node> Mutator::ExchangeContainer( ContainerInterface *child_container, 
                                          list<shared_ptr<Mutator>> zone_terminii )
{
	ASSERT( mode==Mode::Container );
	TreePtr<Node> old_child = (TreePtr<Node>)*container_iterator;
	
	// We don't need the placeholder any more
	ContainerInterface::iterator it_after = parent_container->erase( container_iterator );
	
	// Child zone base has ContainerInterface, so it's a SubContainer. We get here due to         
	// FreeZones created by StarAgent. Expand it and populate into the parent, which is also a SubContainer.         
	for( ContainerInterface::iterator it_child = child_container->begin();
		 it_child != child_container->end();
		 ++it_child    )
	{
		TreePtr<Node> child_element = (TreePtr<Node>)*it_child; 
		ContainerInterface::iterator it_new = parent_container->insert( it_after, child_element ); // inserts before it_after

		if( child_element == MakePlaceholder() ) 
		{
			// If child_element is a placeholder, some zone terminates immediately at this element.
			// So child_element *IS* the placeholder of one of that zone's Mutators and child_container IS
			// the zone's base container. We need to build a new terminus for the zone that uses our parent 
			// container because that's what will be kept.
			// Note: Kept: parent container, other child terminii
			// Discarded: child terminus, child base, child container
			shared_ptr<Mutator> child_terminus = FindMatchingTerminus( child_container, it_child, zone_terminii );                                                
			*child_terminus = *MakeContainerMutator(parent_node, parent_container, it_new);                            
		}
	}                   
	                     
	return old_child;	
}
    
    
void Mutator::ExchangeParent( Mutator &other_mut )
{
    TreePtr<Node> other_child_node = other_mut.GetXLink().GetChildTreePtr(); // outside the zone        
    TreePtr<Node> my_child_node =  ExchangeChild( other_child_node );    
    other_mut.ExchangeChild( my_child_node );    
        
    swap(*this, other_mut);
}
    
    

TreePtr<Node> Mutator::GetParentNode() const
{
	ASSERT( mode != Mode::Root );
    return parent_node;
}


bool Mutator::IsAtRoot() const
{
	return mode == Mode::Root;
}


XLink Mutator::GetXLink() const
{
	XLink xlink;
	if( mode == Mode::Root )
		xlink = XLink::CreateFrom(sp_tp_root_node);
	else
	    xlink = XLink(parent_node, GetTreePtrInterface() );
    ASSERT( xlink );
    return xlink;
}


const TreePtrInterface *Mutator::GetTreePtrInterface() const
{    
	switch( mode )
	{
		case Mode::Root:
			ASSERT( sp_tp_root_node );
			return sp_tp_root_node.get();
		
		case Mode::Singular:
			ASSERT( parent_singular );
			return parent_singular;

		case Mode::Container:
		{
			// Mutating to a ContainerInterface can do this, and then it's ambiguous where
			// we should use: there can be zero or many candidates.
			ASSERT( container_iterator != parent_container->end() );
			
			const TreePtrInterface *parent_singular = &*container_iterator;
			ASSERT( parent_singular );    
			return parent_singular;
		}
		
		default: 
			ASSERTFAIL();
	}			
}  


TreePtr<Node> Mutator::MakePlaceholder()
{
    // There should be no child where we have a mutator
    return TreePtr<Node>(); // It's just a NULL tree ptr!
}
    
    
shared_ptr<Mutator> Mutator::FindMatchingTerminus( ContainerInterface *container,
                                                   ContainerInterface::iterator it_placeholder,
                                                   list<shared_ptr<Mutator>> &candidate_terminii )
{
    shared_ptr<Mutator> found_terminus;

    for( shared_ptr<Mutator> candidate_terminus : candidate_terminii )
    {
        if( candidate_terminus->mode == Mode::Container && 
            candidate_terminus->parent_container == container && 
            candidate_terminus->container_iterator == it_placeholder )
        {
            ASSERTS( !found_terminus )("Found multiple matching terminii including ")(*found_terminus)(" and now ")(*candidate_terminus);
            found_terminus = candidate_terminus;
        }
    }
    
    ASSERTS( found_terminus );
    return found_terminus;
}                                  


void Mutator::Validate() const // TODO call this more
{    
 	switch( mode )
	{
		case Mode::Root:
			ASSERT( sp_tp_root_node );
			break;
			
		case Mode::Singular:
			ASSERT( parent_singular );
			// TODO but is parent_singular inside parent_node????
			break;
			
		case Mode::Container:
		{
			// important invariant: placeholder iterator must point to a member in the parent container
			// TODO but is parent_container inside parent_node????
			ASSERT( container_iterator != parent_container->end() );
			bool found = false;
			for( ContainerInterface::iterator it=parent_container->begin(); it!=parent_container->end(); ++it )
			{
				if( it == container_iterator )
					found = true;        
			}
			ASSERT( found );
			break;
		}
	}
}


string Mutator::GetTrace() const
{
 	switch( mode )
	{
		case Mode::Root:
			ASSERT( sp_tp_root_node );
			return "⌻";
		
		case Mode::Singular:
			ASSERT( parent_singular );
			return "⌾"+parent_singular->GetTypeName();
		
		case Mode::Container:
		{
			int i=-1;
			for( ContainerInterface::iterator it=parent_container->begin(); it!=parent_container->end(); ++it )
			{
				i++;
				if( it == container_iterator )
					break;
			}
			string si;
			if( i==-1 )
				si = "ERROR!";
			else
				si = to_string(i);
			return "⍟"+parent_container->GetTypeName()+"["+si+" of "+to_string(parent_container->size())+"]";			//...?
		}
		
		default:
			ASSERTFAIL();
	}	
}


