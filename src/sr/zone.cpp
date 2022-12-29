#include "zone.hpp"

#include "helpers/flatten.hpp"

using namespace SR;

// ------------------------- FreeZone --------------------------

FreeZone FreeZone::CreateEmpty()
{
    return FreeZone( TreePtr<Node>(), // NULL
                     { shared_ptr<Updater>() } ); // One element, NULL
}


FreeZone::FreeZone( TreePtr<Node> base_, list<shared_ptr<Updater>> terminii_ ) :
    base( base_ ),
    terminii( move(terminii_) )
{
    // An empty free zone is indicated by a NULL base and exactly one
    // terminus, which should also be NULL.
    if( !base )
    {
        ASSERT( terminii.size() == 1 );
        ASSERT( !OnlyElementOf(terminii) );
    }

    // Checks all terminii are distinct
    (void)ToSetSolo(terminii);
}


TreePtr<Node> FreeZone::GetBase() const
{
    return base;
}


const list<shared_ptr<Updater>> &FreeZone::GetTerminii() const
{
    return terminii;
}


bool FreeZone::IsEmpty() const
{
    // No base indicates an empty zone
    if( !base )
    {
        ASSERT( terminii.size() == 1 );
        ASSERT( !OnlyElementOf(terminii) );
        return true;
    }
    return false;
}


string FreeZone::GetTrace() const
{
    list<string> elts;
    for( const shared_ptr<Updater> &p : terminii )
        elts.push_back( Trace(p) );
    
    string arrow;
    if( IsEmpty() )
    {
        arrow = " ↯ "; // Indicates zone is empty due to a terminus at base
    }
    else
    {
        if( terminii.empty() )
            arrow = " →"; // Indicates the zone goes all the way to leaves i.e. subtree
        else
            arrow = " ⇥ "; // Indicates the zone terminates
    }
        
    return "FreeZone(" + Trace(base) + arrow + Join(elts, ", ") +")";
}


// ------------------------- TreeZone --------------------------

TreeZone TreeZone::CreateEmpty( XLink base )
{
    ASSERTS( base );
    return TreeZone( base, 
                     { base } ); // One element, same as base
}


TreeZone::TreeZone( XLink base_, list<XLink> terminii_ ) :
    base( base_ ),
    terminii( terminii_ )
{
    ASSERT( base ); // TreeZone is not nullable
    ASSERT( base.GetChildX() ); // Cannot be empty
}


TreeZone::TreeZone( XLink base, const FreeZone &free_zone ) :
    TreeZone( base )
{
    ASSERT( base.GetChildX() == free_zone.GetBase() )
          ( "Making TreeZone from FreeZone but bases do not match:\n")
          (base)("\n")
          (free_zone.GetBase())("\n");
    // TODO copy terminii out of free_zone
}    


XLink TreeZone::GetBase() const
{
    return base;
}


list<XLink> TreeZone::GetTerminii() const
{
    return terminii;
}


bool TreeZone::IsEmpty() const
{
    // There must be a base, so the only way to be empty is to terminate at the base
    return terminii.size()==1 && OnlyElementOf(terminii)==base;
}


string TreeZone::GetTrace() const
{
    list<string> elts;
    for( const XLink &p : terminii )
        elts.push_back( Trace(p) );

    string rhs;
    if( IsEmpty() )
    {
        rhs = " ↯"; // Indicates zone is empty due to a terminus at base
                    // (we still give the base, for info)
    }
    else
    {
        string arrow;
        if( terminii.empty() )
            arrow = " →"; // Indicates the zone goes all the way to leaves i.e. subtree
        else
            arrow = " ⇥ "; // Indicates the zone terminates
            
        rhs = arrow + Join(elts, ", ");
    }
    
    return "TreeZone(" + Trace(base) + rhs +")";
}
