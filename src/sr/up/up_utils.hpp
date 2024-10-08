#ifndef UP_UTILS_HPP
#define UP_UTILS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"
#include "zone_commands.hpp"

namespace SR 
{
// ------------------------- EmptyZoneElider --------------------------

class EmptyZoneElider
{
public:	
	EmptyZoneElider();
	
	// Can change the supplied shared ptr
	void Run( shared_ptr<ZoneExpression> &root_expr );
	
	// Just ASSERT no empty zones
	void Check( shared_ptr<ZoneExpression> &root_expr );
};

// ------------------------- ZoneMarkEnacter --------------------------

class ZoneMarkEnacter 
{
public:
	ZoneMarkEnacter( const XTreeDatabase *db );
	void Run( shared_ptr<ZoneExpression> &root_expr );
	void Check( shared_ptr<ZoneExpression> &root_expr );

private:
	const XTreeDatabase * const db;
};

}

#endif
