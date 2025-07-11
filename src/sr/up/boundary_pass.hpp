#ifndef BOUNDARY_PASS_HPP
#define BOUNDARY_PASS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "patches.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

/**
 * Make boundaries of tree zones globally consistent by ensuring that
 * no boundary of any tree zone is in the interior of another. This ensures
 * that if two tree zones are not equal, then they don't overlap. Among
 * other things, this makes their base XLinks good for ordering checks.
 * It also makes move and duplicate safe. After this pass we can say that
 * a tree zone appears a natural number N of times in the layout without
 * ambiguity. Wwe must duplicate N-1 appearances, and the remaining one 
 * will be move or leave-alone.
 **/ 
class BoundaryPass
{
public:
	BoundaryPass( XTreeDatabase *db );
	
	void Run(shared_ptr<Patch> &layout);
		
private:
    void GatherBoundaries(TreeZone *tree_zone);
	void CheckTreeZoneAtBoundaries( shared_ptr<Patch> *patch_ptr );
	XLink TryGetBoundaryInRange( XLink lower, bool lower_incl, XLink upper, bool upper_incl );
	void SplitTreeZoneAtXLink( shared_ptr<Patch> *patch_ptr, XLink split_point );

	XTreeDatabase * const db;
    SR::DepthFirstRelation dfr;                          
	set<XLink, DepthFirstRelation> boundaries;
	queue<shared_ptr<Patch> *> check_queue;
};

}

#endif
