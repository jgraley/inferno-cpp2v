#include "zone.hpp"
#include "free_zone.hpp"

#include "scr_engine.hpp"


using namespace SR;

bool Zone::IsSubtree() const
{
	return GetNumTerminii() == 0;
}
