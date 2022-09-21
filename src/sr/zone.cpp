#include "zone.hpp"

using namespace SR;


Zone::Zone( XLink base_ ) :
    base( base_ )
{
    //ASSERT( base ); // Zone is not nullable
    //ASSERT( base.GetXPtr() ); // Cannot be empty
}


XLink Zone::GetBase() const
{
    return base;
}

      

    

