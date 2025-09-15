#include "zone.hpp"
#include "free_zone.hpp"

#include "scr_engine.hpp"


using namespace VN;

bool Zone::IsSubtree() const
{
	return GetNumTerminii() == 0;
}
