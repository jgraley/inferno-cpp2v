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
                     { shared_ptr<Mutator>() } ); // One element, NULL
}


FreeZone FreeZone::CreateScaffold(const TreePtrInterface *tpi_base, int num_terminii)
{
    // Create the scaffolding, of type that matches the supplied TreePtr<>
    auto pair = tpi_base->MakeScaffold();        
    
    // Set the base as the scaffolding node
    FreeZone zone( pair.first, {} );
    
    // Set the terminii as the scaffolding node's child pointers
    for( int i=0; i<num_terminii; i++ )
    {
        ContainerInterface::iterator it = pair.second->insert( Mutator::MakePlaceholder() );
        zone.AddTerminus( Mutator::MakeContainerMutator(pair.first, pair.second, it) );     
    }
    
    //FTRACES("Created scaffold with %d terminii\n", num_terminii)("\n");

    return zone;
}


FreeZone::FreeZone()
{
}


FreeZone::FreeZone( TreePtr<Node> base_, list<shared_ptr<Mutator>> &&terminii_ ) : 
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


ContainerInterface *FreeZone::TryGetContainerBase() const
{
	return dynamic_cast<ContainerInterface *>(base.get());
}



int FreeZone::GetNumTerminii() const
{
    return terminii.size();
}


void FreeZone::AddTerminus(shared_ptr<Mutator> terminus)
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


void FreeZone::SetBase( TreePtr<Node> base_ )
{
	ASSERT(base_); // No trying to make an empty zone this way
	base = base_;
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
    ASSERT( child_zone.get() != this ); 

    if( child_zone->IsEmpty() )
    {
        // Nothing happens to this terminus. If we're empty, we'll stay empty.
        return ++it_t; 
    }    
    
    if( IsEmpty() )
    {        
        // Child zone overwrites us
        base = child_zone->base;        
    }    
    else     
    {
		if( auto child_base_container = child_zone->TryGetContainerBase() )
		    (*it_t)->ExchangeContainer( child_base_container, child_zone->terminii );
		else
			(*it_t)->ExchangeChild( child_zone->base );
        // Populate terminus. This will expand SubContainers. Remember that
        // terminii are reference-like and so it's fine that we erase it.        
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
