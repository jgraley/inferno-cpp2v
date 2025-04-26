#include "mutator.hpp"

#include "helpers/flatten.hpp"

using namespace SR;

// ------------------------- Mutator --------------------------    

Mutator Mutator::CreateFreeSingular( TreePtr<Node> parent_node, 
							   	     TreePtrInterface *parent_singular )
{
	ASSERTS( parent_node )("For tree zone mutator, parent must be a valid node");
	ASSERTS( !(TreePtr<Node>)*parent_singular )("For tree zone mutator, child must be placeholder");
	return Mutator(Mode::Singular, parent_node, parent_singular, nullptr, ContainerInterface::iterator(), nullptr);
}

										  		  
Mutator Mutator::CreateFreeContainer( TreePtr<Node> parent_node, 
									  ContainerInterface *parent_container,
									  ContainerInterface::iterator container_iterator )
{
	ASSERTS( parent_node )("For tree zone mutator, parent must be a valid node");
	ASSERTS( parent_container )("For tree zone mutator, parent must be a valid node");
	ASSERTS( !(TreePtr<Node>)*container_iterator )("For tree zone mutator, child must be a placeholder");
	return Mutator(Mode::Container, parent_node, nullptr, parent_container, container_iterator, nullptr);
}


Mutator Mutator::CreateTreeRoot( shared_ptr<TreePtr<Node>> sp_tp_root_node )
{
	ASSERTS( (TreePtr<Node>)*sp_tp_root_node )("For tree zone mutator, child must be a valid node");
	return Mutator(Mode::Root, nullptr, nullptr, nullptr, ContainerInterface::iterator(), sp_tp_root_node);
}


Mutator Mutator::CreateTreeSingular( TreePtr<Node> parent_node, 
									 TreePtrInterface *parent_singular )
{
	ASSERTS( parent_node )("For tree zone mutator, parent must be a valid node");
	ASSERTS( (TreePtr<Node>)*parent_singular )("For tree zone mutator, child must be a valid node");
	return Mutator(Mode::Singular, parent_node, parent_singular, nullptr, ContainerInterface::iterator(), nullptr);
}

										  		  
Mutator Mutator::CreateTreeContainer( TreePtr<Node> parent_node, 
									  ContainerInterface *parent_container,
									  ContainerInterface::iterator container_iterator )
{
	ASSERTS( parent_node )("For tree zone mutator, parent must be a valid node");
	ASSERTS( parent_container )("For tree zone mutator, parent must be a valid node");
	ASSERTS( (TreePtr<Node>)*container_iterator )("For tree zone mutator, child must be a valid node");
	return Mutator(Mode::Container, parent_node, nullptr, parent_container, container_iterator, nullptr);
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


Mutator::Mutator() :
	mode( Mode::Null )
{
}


bool Mutator::operator<(const Mutator &right) const
{
	return GetTreePtrInterface() < right.GetTreePtrInterface();
}


bool Mutator::operator==( const Mutator &right ) const
{
	return GetTreePtrInterface() == right.GetTreePtrInterface();
}


bool Mutator::operator!=( const Mutator &right ) const
{
	return GetTreePtrInterface() != right.GetTreePtrInterface();
}


Mutator::operator bool() const
{
	return mode != Mode::Null;
}


TreePtr<Node> Mutator::ExchangeChild( TreePtr<Node> free_child ) const
{	
	// Free zone merging should get rid of these, and things will change with general wide zone support
	ASSERT( !dynamic_cast<ContainerInterface *>(free_child.get()) )("Cannot accept wide here");
	TreePtr<Node> old_child = GetChildTreePtr();
	switch( mode )
	{
		case Mode::Root:
		{
			*sp_tp_root_node = free_child;
			TRACE("Singular mutated ")(old_child)(" into ")(free_child)("\n");   
			break;
		}

		case Mode::Singular:
		{
			*parent_singular = free_child;
			TRACE("Singular mutated ")(old_child)(" into ")(free_child)("\n");   
			break;
		}
			
		case Mode::Container:
		{
			container_iterator.Mutate(&free_child); 
			TRACE("Container mutated ")(old_child)(" into ")(free_child)("\n");   
			break;
		}
		
		case Mode::Null:
		default: 
			ASSERTFAIL();
	}	  
	
	return old_child;
}
    
    
TreePtr<Node> Mutator::ExchangeContainer( ContainerInterface *free_child_container, 
                                          list<Mutator> &free_zone_terminii )
{
	ASSERT( mode==Mode::Container );
	TreePtr<Node> old_child = (TreePtr<Node>)*container_iterator;
	
	// We don't need the placeholder any more
	ContainerInterface::iterator it_after = parent_container->erase( container_iterator );
	
	// Child zone base has ContainerInterface, so it's a SubContainer. We get here due to         
	// FreeZones created by StarAgent. Expand it and populate into the parent, which is also a SubContainer.         
	for( ContainerInterface::iterator it_child = free_child_container->begin();
		 it_child != free_child_container->end();
		 ++it_child    )
	{
		TreePtr<Node> child_element = (TreePtr<Node>)*it_child; 
		ContainerInterface::iterator it_new = parent_container->insert( it_after, child_element ); // inserts before it_after

		if( !child_element ) // Child element is a place-holder 
		{
			// If child_element is a placeholder, some zone terminates immediately at this element
			// (the zone is "shallow"). So child_element *IS* the placeholder of one of that zone's 
			// Mutators and free_child_container IS the zone's base container. We need to in-place modify 
			// the supplied zone terminus using our parent container because that's what will be kept.
			// Note: Kept: parent container, other child terminii
			// Discarded: child terminus, child base, child container
			Mutator *child_terminus = FindMatchingTerminus( free_child_container, it_child, free_zone_terminii );                                                
			child_terminus->mode = Mode::Container;
			child_terminus->parent_node = parent_node;
			child_terminus->parent_container = parent_container;
			child_terminus->container_iterator = it_new;
		}
	}                   
	                     
	return old_child;	
}
    
    
void Mutator::ExchangeParent( Mutator &free_mut )
{
    TreePtr<Node> free_child = free_mut.GetChildTreePtr(); // outside the zone        
	TreePtr<Node> old_child = GetChildTreePtr();
    
    ExchangeChild( free_child );            
    swap(*this, free_mut);
    ExchangeChild( old_child );        
}   
        

// This one will not change the original child, so it's safe if that
// aliases as in the case of exchanging on an empty TZ. However, it does
// not update free_mut (in the case of exchanging on an empty TZ, just
// return an empty FZ).
void Mutator::SetParent( const Mutator &free_mut )
{
	TreePtr<Node> old_child = GetChildTreePtr();
    *this = free_mut;    
    ExchangeChild( old_child );           
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
		
		case Mode::Null:
		default: 
			ASSERTFAIL();
	}			
}  


TreePtr<Node> Mutator::GetChildTreePtr() const
{
	return (TreePtr<Node>)*(GetTreePtrInterface());
}


TreePtr<Node> Mutator::MakePlaceholder()
{
    // There should be no child where we have a mutator
    return TreePtr<Node>(); // It's just a NULL tree ptr!
}
    
    
Mutator *Mutator::FindMatchingTerminus( ContainerInterface *container,
                                        ContainerInterface::iterator it_placeholder,
                                        list<Mutator> &candidate_terminii )
{
    Mutator *found_terminus = nullptr;

    for( Mutator &candidate_terminus : candidate_terminii )
    {
        if( candidate_terminus.mode == Mode::Container && 
            candidate_terminus.parent_container == container && 
            candidate_terminus.container_iterator == it_placeholder )
        {
            ASSERTS( !found_terminus )("Found multiple matching terminii including ")(*found_terminus)(" and now ")(candidate_terminus);
            found_terminus = &candidate_terminus;
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
		
		case Mode::Null:
		default:
			ASSERTFAIL();
	}
}


string Mutator::GetTrace() const
{
	string s;
 	switch( mode )
	{
		case Mode::Root:
			s = "⌻";
			break;
		
		case Mode::Singular:
			s = "⌾";
			if( parent_singular )
				s += parent_singular->GetTypeName();
			break;
		
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
			s = "⍟"+parent_container->GetTypeName()+"["+si+" of "+to_string(parent_container->size())+"]";			//...?
			break;
		}
		
		case Mode::Null:
			return "NULL";
			
		default:
			break;
	}	
	
	const TreePtrInterface *tpi = GetTreePtrInterface();
	s += "=" + tpi->GetName();
		
	return s;
}


