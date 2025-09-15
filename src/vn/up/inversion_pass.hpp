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
 * Installs all the free zones into the main tree. 
 * Determines inverted tree zones for free patches by examining surrounding tree 
 * patches and the origin of the update. We build an extra tree with the free 
 * zone contents (adding it to all geometric and intrinsic db assets) and 
 * swap it with the inverted tree zone. The extra tree now contains that which 
 * we wish to discard, so we tear it down and this also removes from all assets.
 */ 
class InversionPass 
{
public:
    InversionPass( XTreeDatabase *db, class ScaffoldOps *sops_ );
    
    void RunInversion(XLink origin, shared_ptr<Patch> *source_layout_ptr);
    
private:
    // .first is NULL: base not known
    typedef pair<XLink, shared_ptr<Patch> *> LocatedPatch;

    void WalkLocatedPatches(LocatedPatch lze_root);
    void Invert(LocatedPatch lzfe);

    XTreeDatabase * const db;
    ScaffoldOps * const sops;
};

}

#endif
