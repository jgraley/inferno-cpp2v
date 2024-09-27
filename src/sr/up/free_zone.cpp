#include "free_zone.hpp"

#include "helpers/flatten.hpp"
#include "db/df_relation.hpp"
#include "db/x_tree_database.hpp"
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


void FreeZone::PopulateAll( list<unique_ptr<FreeZone>> &&child_zones ) 
{
	ASSERT( terminii.size() == child_zones.size() );
	
	if( IsEmpty() )
	{		
		// child zone overwrites us
		operator=(*(child_zones.front()));
		return;
	}	
	
	list<unique_ptr<FreeZone>>::iterator it_cz;
	list<shared_ptr<Terminus>>::iterator it_t;
	for( it_cz = child_zones.begin(), it_t = terminii.begin();
	     it_cz != child_zones.end() || it_t != terminii.end();
	     it_cz++ )
	{	
		ASSERT( it_cz != child_zones.end() && it_t != terminii.end() ); // length mismatch
		unique_ptr<FreeZone> child_zone = move(*it_cz);
		shared_ptr<Terminus> terminus = *it_t;
		
		if( child_zone->IsEmpty() )
		{
			// nothing happens to this terminus
			continue; 
		}		
		
		// TODO support inserting the child's terminii. Do we renumber, or
		// just assign new keys?
		ASSERT( child_zone->GetNumTerminii() == 0 );
		 
		// Populate terminus. This will expand SubContainers
		terminus->PopulateTerminus( child_zone->GetBaseNode() );
		
		// it_t updated to the next terminus after the one we erased
		it_t = terminii.erase( it_t );		
	}		
		
	ASSERT( GetNumTerminii() == 0 );
}


list<shared_ptr<Terminus>>::iterator FreeZone::PopulateTerminus( list<shared_ptr<Terminus>>::iterator it_t, 
                                                                 unique_ptr<FreeZone> &&child_zone ) 
{
	if( child_zone->IsEmpty() )
	{
		// nothing happens to this terminus
		return ++it_t; 
	}		
	
	shared_ptr<Terminus> terminus = *it_t;

	// TODO support inserting the child's terminii. Do we renumber, or
	// just assign new keys?
	ASSERT( child_zone->GetNumTerminii() == 0 );
	 
	// Populate terminus. This will expand SubContainers
	terminus->PopulateTerminus( child_zone->GetBaseNode() );
	
	// it_t updated to the next terminus after the one we erased, or end()
	return terminii.erase( it_t );		
}


list<shared_ptr<Terminus>> &FreeZone::GetTerminii() 
{
	ASSERT(!IsEmpty());
    return terminii;
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
