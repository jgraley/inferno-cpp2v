#include "zone.hpp"

#include "scr_engine.hpp"


using namespace SR;

bool Zone::IsSubtree() const
{
	return GetNumTerminii() == 0;
}


void Zone::MarkBaseForEmbedded(RequiresSubordinateSCREngine *embedded_agent)
{
    ASSERT( !IsEmpty() );
    embedded_agent->MarkBaseForEmbedded( GetBaseNode() );   
    // Note: SCREngine will tell us to take a hike if we execute this more than once}
}
