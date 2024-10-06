#ifndef EMPTY_ZONE_HPP
#define EMPTY_ZONE_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "free_zone.hpp"
#include "tree_zone.hpp"
#include "../link.hpp"
#include "duplicate.hpp"
#include "../scr_engine.hpp"
#include "commands.hpp"

namespace SR 
{

class EmptyZoneElider
{
public:	
	EmptyZoneElider();
	
	// Can change the supplied shared ptr
	void Run( shared_ptr<ZoneExpression> &root_expr );
	
	// Just ASSERT no empty zones
	void Check( shared_ptr<ZoneExpression> &root_expr );
};

}

#endif
