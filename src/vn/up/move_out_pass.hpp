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
 * For tree patches with intent MOVABLE, perform the first of two passes 
 * required for moves (other is MoveInPass). This involves making TWO 
 * identical scaffold nodes that are type-compatible with the moving tree zone.
 * Call them A and B. Then:
 * - moving the tree zone content somewhere safe
 * - replacing it with scaffold A
 * - turning the patch into a free patch with scaffold B
 * - remembering the association in our moves map 
 * 
 * We hope that scaffold A will be eaten by inversion, while B will make 
 * it into the main tree. Then MoveInPass will find it using the moves 
 * map and put the moving tree zone there.
 */
class MoveOutPass
{
public:    
    MoveOutPass(XTreeDatabase *db_, class ScaffoldOps *sops_);
    
    // Can change the supplied shared ptr
    void Run( shared_ptr<Patch> &layout, class MovesMap &moves_map);
    
private:    
	void MoveTreeZoneOut( shared_ptr<Patch> *ooo_patch_ptr, shared_ptr<Patch> &layout, class MovesMap &moves_map );

    XTreeDatabase * const db;
    ScaffoldOps * const sops;
};

}

#endif
