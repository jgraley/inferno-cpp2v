#ifndef INVERSION_HPP
#define INVERSION_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

// Convert free patches into targetted patches by matching them up against 
// surrounding tree patches and the supplied overall tree zone. TODO 
// in fact we only need the base XLink.
class TreeZoneInverter 
{
public:
    TreeZoneInverter( XTreeDatabase *db );
    
    void Run(shared_ptr<Mutator> origin_mutator, shared_ptr<Patch> *source_layout_ptr);
    
private:
    // .first is NULL: base not known
    typedef pair<shared_ptr<Mutator>, shared_ptr<Patch> *> LocatedPatch;

    void WalkLocatedPatches(LocatedPatch lze_root);
    void Invert(LocatedPatch lzfe);

    XTreeDatabase * const db;
};

}

#endif
