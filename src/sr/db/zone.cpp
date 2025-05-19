#include "zone.hpp"
#include "free_zone.hpp"

#include "scr_engine.hpp"


using namespace SR;

bool Zone::IsSubtree() const
{
	return GetNumTerminii() == 0;
}


void Zone::MarkOriginForEmbedded(RequiresSubordinateSCREngine *embedded_agent)
{
    ASSERT( !IsEmpty() );
    embedded_agent->MarkOriginForEmbedded( GetBaseNode() );   
    // Note: SCREngine will tell us to take a hike if we execute this more than once}
}


FreeZone Zone::CreateSimilarScaffoldZone() const
{
	return FreeZone::CreateScaffoldToSpec(GetBaseNode(), GetNumTerminii());
}
