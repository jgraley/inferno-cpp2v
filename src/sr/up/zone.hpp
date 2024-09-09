#ifndef ZONE_HPP
#define ZONE_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"
#include "updater.hpp"

#include <unordered_set>

namespace SR 
{
	    
// ------------------------- Zone --------------------------
class Zone : public Traceable
{
public:
	virtual ~Zone() = default;
};
    
 
}

#endif
