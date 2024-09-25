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


FreeZone::FreeZone( TreePtr<Node> base_, vector<shared_ptr<Terminus>> terminii_ ) :
    base( base_ )
{
	// Fill the map
	for( int ti=0; ti<terminii_.size(); ti++ )
	{
		terminii[ti] = terminii_.at(ti);
		if( base )
			ASSERT( terminii.at(ti) );
	}
	
    // An empty free zone is indicated by a NULL base and exactly one
    // terminus, which should also be NULL.
    if( !base )
    {
        ASSERT( terminii.size() == 1 );
        ASSERT( !terminii.at(0) );
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
        ASSERT( !terminii.at(0) );
        return true;
    }
    return false;
}


int FreeZone::GetNumTerminii() const
{
    return terminii.size();
}


void FreeZone::AddTerminus(int ti, shared_ptr<Terminus> terminus)
{
	// Can't use this to make an empty zone
	ASSERT( base );
	ASSERT( terminus );
	
	terminii[ti] = terminus;
}


TreePtr<Node> FreeZone::GetBaseNode() const
{
	ASSERT(!IsEmpty());
    return base;
}


void FreeZone::Populate( XTreeDatabase *x_tree_db, vector<unique_ptr<FreeZone>> &&child_zones ) 
{
	ASSERT( terminii.size() == child_zones.size() );
	int ti=0;
	for( unique_ptr<FreeZone> &child_zone : child_zones )
	{
		Join(move(child_zone), ti++);
	}		
		
	ASSERT( GetNumTerminii() == 0 );
}


vector<shared_ptr<Terminus>> FreeZone::GetTerminusUpdaters() const
{
	ASSERT(!IsEmpty());
	vector<shared_ptr<Terminus>> v;
	for( int ti=0; ti<terminii.size(); ti++ )
	{
		ASSERT( terminii.count(ti) > 0 );
		v.push_back( terminii.at(ti) );
	}
	
    return v;
}


shared_ptr<Terminus> FreeZone::GetTerminus(int ti) const
{
	ASSERT(!IsEmpty());
	ASSERT( ti >= 0 );
	ASSERT( terminii.count(ti) > 0 );
    return terminii.at(ti);
}


void FreeZone::DropTerminus(int ti)
{
	ASSERT(!IsEmpty());
	ASSERT( ti >= 0 );
	ASSERT( terminii.count(ti) > 0 );
	int ne = terminii.erase(ti);
	ASSERT( ne==1 );
}


void FreeZone::Join( unique_ptr<FreeZone> &&child_zone, int ti )
{
	if( child_zone->IsEmpty() )
	{
		// nothing happens	
		return; 
	}
	else if( IsEmpty() )
	{		
		// child zone overwrites us
		ASSERT(ti == 0);		
		operator=(*child_zone);
		return;
	}
	
	// TODO support inserting the child's terminii. Do we renumber, or
	// just assign new keys?
	ASSERT( child_zone->GetNumTerminii() == 0 );
	 
    shared_ptr<Terminus> terminus = GetTerminus(ti);
    DropTerminus(ti);
    
    // Populate terminus. Join() will expand SubContainers
    ASSERT( child_zone->GetBaseNode() );
    terminus->Join( child_zone->GetBaseNode() );
    
    //Validate()( child_zone->GetBaseNode() );     
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


