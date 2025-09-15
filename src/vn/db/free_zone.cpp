#include "free_zone.hpp"

#include "helpers/flatten.hpp"
#include "df_relation.hpp"
#include "x_tree_database.hpp"
#include "tree_zone.hpp"

using namespace VN;

// ------------------------- FreeZone --------------------------

FreeZone FreeZone::CreateSubtree( TreePtr<Node> base )
{
    return FreeZone( base, list<Mutator>{} );
}


FreeZone FreeZone::CreateEmpty()
{
    return FreeZone( TreePtr<Node>(), // NULL
                     list<Mutator>{ Mutator() } ); // One element, NULL
}


FreeZone::FreeZone()
{
}


FreeZone::FreeZone( TreePtr<Node> base_, list<Mutator> &&terminii_ ) : 
    base( base_ )
{
    // An empty free zone is indicated by a NULL base and exactly one
    // terminus, which should also be NULL.
    if( !base )
    {
        ASSERT( terminii.empty() );
    }

    for( Mutator &terminus : terminii_ )
    {
		ASSERT( !terminus.GetChildTreePtr() );
		terminii.push_back(move(terminus));
	}	

    // Checks all terminii are distinct
    (void)ToSetSolo(terminii);
}


FreeZone &FreeZone::operator=( const FreeZone &other )
{
    base = other.base;
    terminii = other.terminii;
    return *this;
}


bool FreeZone::IsEmpty() const
{
    // No base indicates an empty zone
    if( !base )
    {
        ASSERT( terminii.size() == 1 );
        ASSERT( !terminii.front() );
        return true;
    }
    return false;
}


ContainerInterface *FreeZone::TryGetContainerBase() const
{
	return dynamic_cast<ContainerInterface *>(base.get());
}


size_t FreeZone::GetNumTerminii() const
{
    return terminii.size();
}


const Mutator &FreeZone::GetTerminusMutator(size_t index) const
{
	list<Mutator>::const_iterator it = terminii.begin();
	for( size_t i=0; i<index; i++ )
		it++;
	return *it;
}


void FreeZone::AddTerminus(Mutator &&terminus)
{
    // Can't use this to make an empty zone
    ASSERT( base );
    
    terminii.push_back(move(terminus));
}


TreePtr<Node> FreeZone::GetBaseNode() const
{
    ASSERT(!IsEmpty());
    return base;
}


void FreeZone::SetBase( TreePtr<Node> base_ )
{
	ASSERT(base_); // No trying to make an empty zone this way
	base = base_;
}


void FreeZone::MergeAll( list<unique_ptr<FreeZone>> &&child_zones ) 
{    
    if( IsEmpty() )
    {        
        // child zone overwrites us
        operator=(*(SoloElementOf(child_zones)));
        return;
    }    
    
    ASSERT( terminii.size() == child_zones.size() );
    
    TerminusIterator it_t = GetTerminiiBegin();
    for( unique_ptr<FreeZone> &child_zone : child_zones )
    {    
        ASSERT( it_t != GetTerminiiEnd() ); // length mismatch        
        it_t = MergeTerminus( it_t, move(child_zone) );        
    }        
        
    ASSERT( it_t == GetTerminiiEnd() ); // length mismatch        
    ASSERT( GetNumTerminii() == 0 );
}


FreeZone::TerminusIterator FreeZone::MergeTerminus( TerminusIterator it_t, 
                                                    unique_ptr<FreeZone> &&child_zone,
                                                    XLink *resulting_xlink ) 
{
    ASSERT( child_zone.get() != this ); 
	if( resulting_xlink )
		*resulting_xlink = XLink();

    if( child_zone->IsEmpty() )
    {
        // Nothing happens to this terminus. If we're empty, we'll stay empty.
        if( resulting_xlink )
        {
			*resulting_xlink = it_t->GetXLink();
		}
        return ++it_t; 
    }    
    
    if( IsEmpty() )
    {        
        // Child zone overwrites us
        base = child_zone->base;        
    }    
    else if( auto child_base_container = child_zone->TryGetContainerBase() )
	{
		// Child is a subcontainer-rooted free zone. Populate, but don't provide 
		// an XLink because it would be ambiguous.
		// TODO separate function for this case.
		it_t->ExchangeContainer( child_base_container, child_zone->terminii );
	}
	else
	{
		// Child is a regular free zone with an unambiguous root.
		it_t->ExchangeChild( child_zone->base );
        
		if( resulting_xlink )
		{
			*resulting_xlink = it_t->GetXLink();
 		}
    }
    
    // it_t updated to the next terminus after the one we erased, or end()
    it_t = terminii.erase( it_t );        
    
    // Insert the child zone's terminii before it_t, i.e. where we just
    // erase()d from. I assume it_t now points after the inserted 
    // terminii, i.e. at the same element it did after the erase()
    terminii.splice( it_t, move(child_zone->terminii) );
    	
    return it_t;
}


FreeZone::TerminusIterator FreeZone::GetTerminiiBegin()
{
    return terminii.begin();
}


FreeZone::TerminusIterator FreeZone::GetTerminiiEnd()
{
    return terminii.end();
}


FreeZone::TerminusConstIterator FreeZone::GetTerminiiBegin() const
{
    return terminii.begin();
}


FreeZone::TerminusConstIterator FreeZone::GetTerminiiEnd() const
{
    return terminii.end();
}


string FreeZone::GetTrace() const
{
    string arrow, s;
    if( IsEmpty() )
    {
        s = " ↯ "; // Indicates zone is empty due to a terminus at base
    }
    else
    {
        s = Trace(base);
        if( terminii.empty() )
            s += " → "; // Indicates the zone goes all the way to leaves i.e. subtree
        else
            s += " ⇥ " + Trace(terminii); // Indicates the zone terminates            
    }
        
    return "FreeZone(" + s + ")";
}
