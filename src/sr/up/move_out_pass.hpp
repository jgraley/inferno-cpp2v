#ifndef MOVE_OUT_PASS_HPP
#define MOVE_OUT_PASS_HPP

#include "patches.hpp"
#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{
	
/**
 * For tree zone patches with intent MOVABLE, perform the first of two passes 
 * required for moves (other in MoveInPass). This involves 
 * - moving the tree zone content somewhere safe
 * - replacing it with scaffold A
 * - turning the patch into a free zone patch with scaffold B
 * We hope that scaffold A will be eaten by inversion, while B will make 
 * it into the main tree. Then MoveInPass will find it and put
 * the content there.
 */
class MoveOutPass
{
public:    
    MoveOutPass(XTreeDatabase *db_, class ScaffoldOps *sops_);
    
    // Can change the supplied shared ptr
    void Run( shared_ptr<Patch> &layout, MovesMap &moves_map);
    
private:    
	void MoveTreeZoneOut( shared_ptr<Patch> *ooo_patch_ptr, shared_ptr<Patch> &layout, MovesMap &moves_map );

    XTreeDatabase * const db;
    ScaffoldOps * const sops;
};

}

#endif
