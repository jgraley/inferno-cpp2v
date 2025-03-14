#ifndef COMPLEMENT_HPP
#define COMPLEMENT_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

// Find Tree Zones that complement the Tree Zones in the layout.
class TreeZoneComplementer
{
public:
	TreeZoneComplementer( XTreeDatabase *db );
	
	void Run(XLink target_origin, shared_ptr<Patch> layout);
	
private:
	XTreeDatabase * const db;
	DepthFirstRelation df_rel;
    map<XLink, TreeZone, DepthFirstRelation> tree_zones_depth_first_by_base;
	set<TreeZone> complement;
};

}

#endif
