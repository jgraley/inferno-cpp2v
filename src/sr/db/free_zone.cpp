#include "free_zone.hpp"

#include "helpers/flatten.hpp"
#include "df_relation.hpp"
#include "x_tree_database.hpp"
#include "tree_zone.hpp"

using namespace SR;

// ------------------------- FreeZone --------------------------

FreeZone FreeZone::CreateSubtree( TreePtr<Node> base )
{
    return FreeZone( base, {} );
}


FreeZone FreeZone::CreateEmpty()
{
    return FreeZone( TreePtr<Node>(), // NULL
                     { shared_ptr<Terminus>() } ); // One element, NULL
}


FreeZone::FreeZone()
{
}


FreeZone::FreeZone( TreePtr<Node> base_, list<shared_ptr<Terminus>> &&terminii_ ) : 
    base( base_ ),
    terminii( move(terminii_) )
{
    // An empty free zone is indicated by a NULL base and exactly one
    // terminus, which should also be NULL.
    if( !base )
    {
        ASSERT( terminii.size() == 1 );
        ASSERT( !terminii.front() );
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


int FreeZone::GetNumTerminii() const
{
    return terminii.size();
}


void FreeZone::AddTerminus(shared_ptr<Terminus> terminus)
{
	// Can't use this to make an empty zone
	ASSERT( base );
	ASSERT( terminus );
	
	terminii.push_back(terminus);
}


TreePtr<Node> FreeZone::GetBaseNode() const
{
	ASSERT(!IsEmpty());
    return base;
}


void FreeZone::MergeAll( list<unique_ptr<FreeZone>> &&child_zones ) 
{
	ASSERT( terminii.size() == child_zones.size() );
	
	if( IsEmpty() )
	{		
		// child zone overwrites us
		operator=(*(child_zones.front()));
		return;
	}	
	
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
                                                       unique_ptr<FreeZone> &&child_zone ) 
{
	ASSERT( !IsEmpty() );
	
	if( child_zone->IsEmpty() )
	{
		// nothing happens to this terminus
		return ++it_t; 
	}		
		 
	ASSERT( child_zone.get() != this ); 
		
	// Populate terminus. This will expand SubContainers. Remember that
	// terminii are reference-like and so it's fine that we erase it.
	(*it_t)->Populate( child_zone->base, child_zone->terminii );
	
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


string FreeZone::GetTrace() const
{
    string arrow, rhs;
    if( IsEmpty() )
    {
        rhs = " ↯ "; // Indicates zone is empty due to a terminus at base
    }
    else
    {
        if( terminii.empty() )
            rhs = " → "; // Indicates the zone goes all the way to leaves i.e. subtree
        else
            rhs = " ⇥ " + Trace(terminii); // Indicates the zone terminates            
    }
        
    return "FreeZone(" + Trace(base) + rhs +")";
}
