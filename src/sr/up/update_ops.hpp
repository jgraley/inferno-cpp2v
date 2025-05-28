#ifndef UPDATE_OPS_HPP
#define UPDATE_OPS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../db/duplicate.hpp"
#include "../db/db_common.hpp"
#include <functional>

namespace SR 
{


class UpdateOps
{
public:	
	UpdateOps( XTreeDatabase *db );

	// Take a free zone and place it inside a new extra tree, plugging its terminii so the tree is legal.
	// Generate and return a tree zone in that new extra tree, with therminii present (not plugged).
	MutableTreeZone FreeZoneIntoExtraTree( FreeZone free_zone, const MutableTreeZone &reference_tree_zone );
	
	
	//MutableTreeZone TreeZoneAroundScaffold( ... );

private:
	XTreeDatabase * const db;
};

}

#endif
