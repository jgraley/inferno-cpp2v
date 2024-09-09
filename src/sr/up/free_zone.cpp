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
                     { shared_ptr<Updater>() } ); // One element, NULL
}


FreeZone::FreeZone()
{
}


FreeZone::FreeZone( TreePtr<Node> base_, vector<shared_ptr<Updater>> terminii_ ) :
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


void FreeZone::AddTerminus(int ti, shared_ptr<Updater> terminus)
{
	// Can't use this to make an empty zone
	ASSERT( base );
	ASSERT( terminus );
	
	terminii[ti] = terminus;
}


TreePtr<Node> FreeZone::GetBaseNode() const
{
    return base;
}


vector<shared_ptr<Updater>> FreeZone::GetTerminusUpdaters() const
{
	vector<shared_ptr<Updater>> v;
	for( int ti=0; ti<terminii.size(); ti++ )
	{
		ASSERT( terminii.count(ti) > 0 );
		v.push_back( terminii.at(ti) );
	}
	
    return v;
}



shared_ptr<Updater> FreeZone::GetTerminusUpdater(int ti) const
{
	ASSERT( ti >= 0 );
	ASSERT( terminii.count(ti) > 0 );
    return terminii.at(ti);
}


void FreeZone::DropTerminus(int ti)
{
	ASSERT( ti >= 0 );
	ASSERT( terminii.count(ti) > 0 );
	int ne = terminii.erase(ti);
	ASSERT( ne==1 );
}


void FreeZone::Join( FreeZone &child_zone, int ti )
{
    ASSERT( !child_zone.IsEmpty() );
    ASSERT( !IsEmpty() ); // Need to elide empty zones before executing	    

    shared_ptr<Updater> terminus_upd = GetTerminusUpdater(ti);
    DropTerminus(ti);
    
    // Populate terminus. Apply() will expand SubContainers
    ASSERT( child_zone.GetBaseNode() );
    terminus_upd->Apply( child_zone.GetBaseNode() );
    
    //Validate()( zone->GetBaseNode() );     
}


string FreeZone::GetTrace() const
{
    list<string> elts;
    for( auto p : terminii )
        elts.push_back( Trace(p.first)+":"+Trace(p.second) );
    
    string arrow, rhs;
    if( IsEmpty() )
    {
        rhs = " ↯ "; // Indicates zone is empty due to a terminus at base
    }
    else
    {
        if( terminii.empty() )
            arrow = " → "; // Indicates the zone goes all the way to leaves i.e. subtree
        else
            arrow = " ⇥ "; // Indicates the zone terminates

        rhs = arrow + ::Join(elts, ", ");
    }
        
    return "FreeZone(" + Trace(base) + rhs +")";
}


