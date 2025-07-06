#ifndef MOVE_IN_PASS_HPP
#define MOVE_IN_PASS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "patches.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

/**
 * Make tree zones for all the scaffolds in the supplied moves_map,
 * and exchange in the corresponding free zones. This completes the
 * journey of move-patches. We only need the moves map for this, 
 * the layout was effectively consumed by the inversion pass.
 **/ 
class MoveInPass
{
public:
	MoveInPass( XTreeDatabase *db, class ScaffoldOps *sops_ );
	
	void Run(class MovesMap &moves_map, ReplaceAssignments *assignments);

private:
	XTreeDatabase * const db;
    ScaffoldOps * const sops;
};

}

#endif
