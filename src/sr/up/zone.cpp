#include "zone.hpp"

#include "scr_engine.hpp"


using namespace SR;

bool Zone::IsSubtree() const
{
	return GetNumTerminii() == 0;
}


void Zone::MarkBaseForEmbedded(const SCREngine *scr_engine, RequiresSubordinateSCREngine *embedded_agent)
{
    ASSERT( !IsEmpty() );
    scr_engine->MarkBaseForEmbedded( embedded_agent, GetBaseNode() );   
    // Note: SCREngine will tell us to take a hike if we execute this more than once}
}
