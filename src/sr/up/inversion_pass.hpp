#ifndef INVERSION_PASS_HPP
#define INVERSION_PASS_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

/** 
 * Installs all the free zones into the tree. 
 * Determines target tree zones for free patches by examining surrounding tree 
 * patches and the origin of the update. Exchanges the free patches' zones over
 * these target tree zones. 
 * 
 * As a post-pass (RunDeleteIntrinsic()) we delete all the intrisic DB info,
 * by using the free zones released by our Exchange calls, which correspond
 * to the target tree zones identified by inversion. The key insight is that
 * this will not delete info corresponding to moves, because at this point
 * all we see of them is the scaffold nodes.
 */ 
class InversionPass 
{
public:
    InversionPass( XTreeDatabase *db, class ScaffoldOps *sops_ );
    
    void RunInversion(const Mutator &origin_mutator, shared_ptr<Patch> *source_layout_ptr);
    
private:
    // .first is NULL: base not known
    typedef pair<Mutator, shared_ptr<Patch> *> LocatedPatch;

    void WalkLocatedPatches(LocatedPatch lze_root);
    void Invert(LocatedPatch lzfe);

    XTreeDatabase * const db;
    ScaffoldOps * const sops;
};

}

#endif
