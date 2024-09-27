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


FreeZone::FreeZone( TreePtr<Node> base_, list<shared_ptr<Terminus>> terminii_ ) : // TODO rvalue ref+move()
    base( base_ ),
    terminii( terminii_ )
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


void FreeZone::Populate( XTreeDatabase *x_tree_db, list<unique_ptr<FreeZone>> &&child_zones ) 
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
	     it_cz++, it_t++ )
	{	
		ASSERT( it_cz != child_zones.end() && it_t != terminii.end() ); // length mismatch
		unique_ptr<FreeZone> &child_zone = *it_cz;
		shared_ptr<Terminus> terminus = *it_t;
		
		if( child_zone->IsEmpty() )
		{
			// nothing happens to this terminus
			continue; 
		}		
		
		// TODO support inserting the child's terminii. Do we renumber, or
		// just assign new keys?
		ASSERT( child_zone->GetNumTerminii() == 0 );
		 
		// Populate terminus. Join() will expand SubContainers
		ASSERT( child_zone->GetBaseNode() );
		terminus->PopulateTerminus( child_zone->GetBaseNode() );
		
		//Validate()( child_zone->GetBaseNode() ); 
	}		
	
	terminii.clear();
		
	ASSERT( GetNumTerminii() == 0 );
}


vector<shared_ptr<Terminus>> FreeZone::GetTerminusUpdaters() const // TODO return a list
{
	ASSERT(!IsEmpty());
	vector<shared_ptr<Terminus>> v;
	for( shared_ptr<Terminus> t : terminii )
	{
		v.push_back( t );
	}
	
    return v;
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
