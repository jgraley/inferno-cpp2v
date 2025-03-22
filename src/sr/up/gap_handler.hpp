#ifndef GAP_FINDER_HPP
#define GAP_FINDER_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "patches.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

// Find non-contiguous gaps between tree zones. Insert empty free zones into these gaps.
class TreeZoneGapHandler
{
public:
	TreeZoneGapHandler( XTreeDatabase *db );
	
	void Run(shared_ptr<Patch> source_layout);
		
private:
	void CheckPatch(shared_ptr<TreeZonePatch> patch);

	XTreeDatabase * const db;
};

}

#endif
