#include "zone.hpp"

using namespace SR;

// ------------------------- FreeZone --------------------------

FreeZone::FreeZone( TreePtr<Node> base_ ) :
    base( base_ )
{
    ASSERT( base ); // FreeZone is not nullable
}


TreePtr<Node> FreeZone::GetBase() const
{
    return base;
}

// ------------------------- TreeZone --------------------------

TreeZone::TreeZone( XLink base_ ) :
    base( base_ )
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

      

    

