#ifndef MERGE_PASSES_HPP
#define MERGE_PASSES_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"
#include "patches.hpp"

namespace SR 
{

// ------------------------- FreeZoneMergeImpl --------------------------

class FreeZoneMergeImpl
{
public:    
	typedef function<bool (const FreeZone *zone, const FreeZone *child_zone)> PolicyFunction;
   
    // Can change the supplied shared ptr
    Patch::Assignments Run( shared_ptr<Patch> &layout, PolicyFunction decider );
    
    void Check( shared_ptr<Patch> &layout, PolicyFunction decider );
};

// ------------------------- MergeFreesPass --------------------------

/**
 * Merging of free zones permits Inversion to unambiguously determine the boundaries
 * for exchanges. It creates the invariant that every free zone is bounded by something
 * other than a free zone, which must be a tree zone or the origin of the update.
 * This resolves the "free zone ambiguity"
 */ 
class MergeFreesPass
{
public:       
    // Can change the supplied shared ptr
    Patch::Assignments Run( shared_ptr<Patch> &layout );
    
    void Check( shared_ptr<Patch> &layout );
    
private:    
    bool Policy(const FreeZone *zone, const FreeZone *child_zone) const;
    FreeZoneMergeImpl impl;
};

// ------------------------- MergeWidesPass --------------------------

/** 
 * Free Zones with collection bases (aka poor man's wide zones) lack flexibility
 * and eg can only be merged into another free zone, so we merge them here. The
 * Check() is stronger and will fail on any collection base, which constrains what
 * we can accept from GenReplaceLayout() etc.
 */ 
class MergeWidesPass
{
public:       
    // Can change the supplied shared ptr
    Patch::Assignments Run( shared_ptr<Patch> &layout );
    
    void Check( shared_ptr<Patch> &layout );

private:    
    bool Policy(const FreeZone *zone, const FreeZone *child_zone) const;
    FreeZoneMergeImpl impl;
};


}

#endif
