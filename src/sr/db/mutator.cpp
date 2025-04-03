#include "mutator.hpp"

#include "helpers/flatten.hpp"

using namespace SR;

// ------------------------- Mutator --------------------------    

shared_ptr<Mutator> Mutator::MakeRootMutator( TreePtrInterface *dest_tree_ptr )
{
	return shared_ptr<Mutator>(new Mutator(dest_tree_ptr));
}

										  		  
shared_ptr<Mutator> Mutator::MakeSingularMutator( TreePtr<Node> parent_node, 
										  		  TreePtrInterface *dest_tree_ptr )
{
	return shared_ptr<Mutator>(new Mutator(parent_node, dest_tree_ptr));
}

										  		  
shared_ptr<Mutator> Mutator::MakeContainerMutator( TreePtr<Node> parent_node, 
												   ContainerInterface *dest_container,
												   ContainerInterface::iterator it_dest )
{
	return shared_ptr<Mutator>(new Mutator(parent_node, dest_container, it_dest));
}


Mutator::Mutator( Mode mode_, TreePtr<Node> parent_node_ ) :
	mode( mode_ ),
    parent_node( parent_node_ ) 
{
}    


Mutator::Mutator( TreePtrInterface *dest_tree_ptr_ ) :
	mode( Mode::Root ),
    parent_node( nullptr ),
    dest_tree_ptr( dest_tree_ptr_ )
{
}


Mutator::Mutator( TreePtr<Node> parent_node, TreePtrInterface *dest_tree_ptr_ ) :
	mode( Mode::Singular ),
    parent_node( parent_node ),
    dest_tree_ptr( dest_tree_ptr_ )
{
}


TreePtr<Node> Mutator::ExchangeChild( TreePtr<Node> new_child, 
                                       list<shared_ptr<Mutator>> child_terminii )
{
    ASSERT( new_child ); // perhaps we tried to populate with an empty zone?
	

	switch( mode )
	{
		case Mode::Root:
		case Mode::Singular:
		{
			TreePtr<Node> old_child = (TreePtr<Node>)*dest_tree_ptr;
			ASSERT( !dynamic_cast<ContainerInterface *>(new_child.get()) )("Cannot accept wide here");
			*dest_tree_ptr = new_child;
			TRACE("Singular mutated ")(new_child)("\n");   
			return old_child;
		}
			
		case Mode::Container:
		{
			TreePtr<Node> old_child = (TreePtr<Node>)*it_dest;
			if( ContainerInterface *child_container = dynamic_cast<ContainerInterface *>(new_child.get()) )
			{                    
				// We don't need the placeholder any more
				ContainerInterface::iterator it_after = dest_container->erase( it_dest );
				
				// Child zone base has ContainerInterface, so it's a SubContainer. We get here due to         
				// FreeZones created by StarAgent. Expand it and populate into the destination, which is also a SubContainer.         
				for( ContainerInterface::iterator it_child = child_container->begin();
					 it_child != child_container->end();
					 ++it_child    )
				{
					TreePtr<Node> child_element = (TreePtr<Node>)*it_child; 
					ContainerInterface::iterator it_new = dest_container->insert( it_after, child_element ); // inserts before it_after

					if( child_element == MakePlaceholder() ) 
					{
						// If child_element is a placeholder, the child FZ terminates immediately at this element.
						// So child_element *IS* the placeholder of that FZ's Mutator instance and child_container IS
						// the FZ base container. We need to build a new terminus for the FZ that uses our dest 
						// container because that's what will be kept.
						// Note: Kept: our container, child terminii
						// Discarded: this terminus, child base, child container
						shared_ptr<Mutator> child_con_terminus = FindMatchingTerminus( child_container, it_child, child_terminii );                                                
						*child_con_terminus = *MakeContainerMutator(GetParentNode(), dest_container, it_new);                            
					}
				}                                    
			}
			else
			{
				// Populate terminus with singular-based zone. We tee into Mutate() in case our container
				// is not order-preserving i.e. std::set<>.        
				it_dest.Mutate(&new_child); 
			}    
			return old_child;
		}
		
		default: 
			ASSERTFAIL();
	}	    
}
    

const TreePtrInterface *Mutator::GetTreePtrInterface() const
{    
	switch( mode )
	{
		case Mode::Root:
		case Mode::Singular:
			ASSERT( dest_tree_ptr );
			return dest_tree_ptr;

		case Mode::Container:
		{
			// Mutating to a ContainerInterface can do this, and then it's ambiguous where
			// we should use: there can be zero or many candidates.
			ASSERT( it_dest != dest_container->end() );
			
			const TreePtrInterface *dest_tree_ptr = &*it_dest;
			ASSERT( dest_tree_ptr );    
			return dest_tree_ptr;
		}
		
		default: 
			ASSERTFAIL();
	}			
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
	ASSERT( mode != Mode::Root );
    
    XLink xlink(parent_node, GetTreePtrInterface() );
    ASSERT( xlink );
    return xlink;
}


TreePtr<Node> Mutator::MakePlaceholder()
{
    // There should be no child where we have a mutator
    return TreePtr<Node>(); // It's just a NULL tree ptr!
}
    
    
string Mutator::GetTrace() const
{
 	switch( mode )
	{
		case Mode::Root:
		case Mode::Singular:
			ASSERT( dest_tree_ptr );
			return "⌾"+dest_tree_ptr->GetTypeName();
		
		case Mode::Container:
		{
			int i=-1;
			for( ContainerInterface::iterator it=dest_container->begin(); it!=dest_container->end(); ++it )
			{
				i++;
				if( it == it_dest )
					break;
			}
			string si;
			if( i==-1 )
				si = "ERROR!";
			else
				si = to_string(i);
			return "⌾"+dest_container->GetTypeName()+"["+si+" of "+to_string(dest_container->size())+"]";			//...?
		}
		
		default:
			ASSERTFAIL();
	}	
}

// ------------------------- ContainerMutator --------------------------    
    
Mutator::Mutator( TreePtr<Node> parent_node_, 
                  ContainerInterface *dest_container_,
                  ContainerInterface::iterator it_dest_ ) :
	mode( Mode::Container ),
    parent_node( parent_node_ ),
    dest_container( dest_container_ ),
    it_dest( it_dest_ )
{
    Validate();
}


shared_ptr<Mutator> Mutator::FindMatchingTerminus( ContainerInterface *container,
                                                   ContainerInterface::iterator it_placeholder,
                                                   list<shared_ptr<Mutator>> &candidate_terminii )
{
    shared_ptr<Mutator> found_terminus;

    for( shared_ptr<Mutator> candidate_terminus : candidate_terminii )
    {
        if( candidate_terminus->mode == Mode::Container && 
            candidate_terminus->dest_container == container && 
            candidate_terminus->it_dest == it_placeholder )
        {
            ASSERTS( !found_terminus )("Found multiple matching terminii including ")(*found_terminus)(" and now ")(*candidate_terminus);
            found_terminus = candidate_terminus;
        }
    }
    
    ASSERTS( found_terminus );
    return found_terminus;
}                                  


void Mutator::Validate() const
{    
    // important invariant: placeholder iterator must point to a member in the destination container
    ASSERT( it_dest != dest_container->end() );
    bool found = false;
    for( ContainerInterface::iterator it=dest_container->begin(); it!=dest_container->end(); ++it )
    {
        if( it == it_dest )
            found = true;        
    }
    ASSERT( found );
}

