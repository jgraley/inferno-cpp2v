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
 */ 
class InversionPass 
{
public:
    InversionPass( XTreeDatabase *db );
    
    void RunInversion(const Mutator &origin_mutator, shared_ptr<Patch> *source_layout_ptr);
    void RunDeleteIntrinsic();
    
private:
    // .first is NULL: base not known
    typedef pair<Mutator, shared_ptr<Patch> *> LocatedPatch;

    void WalkLocatedPatches(LocatedPatch lze_root);
    void Invert(LocatedPatch lzfe);

    XTreeDatabase * const db;
    
    vector<FreeZone> extracted_free_zones;
};

}

#endif
