#ifndef FZ_MERGE_HPP
#define FZ_MERGE_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

// ------------------------- FreeZoneMergeImpl --------------------------

class FreeZoneMergeImpl
{
public:    
	typedef function<bool (const FreeZone *zone, const FreeZone *child_zone)> PolicyFunction;
   
    // Can change the supplied shared ptr
    void Run( shared_ptr<Patch> &layout, PolicyFunction decider );
    
    void Check( shared_ptr<Patch> &layout, PolicyFunction decider );
};

// ------------------------- FreeZoneMerger --------------------------

class FreeZoneMerger
{
public:       
    // Can change the supplied shared ptr
    void Run( shared_ptr<Patch> &layout );
    
    void Check( shared_ptr<Patch> &layout );
    
private:    
    bool Policy(const FreeZone *zone, const FreeZone *child_zone) const;
    FreeZoneMergeImpl impl;
};

// ------------------------- FreeZoneMergeCollectionBases --------------------------

class FreeZoneMergeCollectionBases
{
public:       
    // Can change the supplied shared ptr
    void Run( shared_ptr<Patch> &layout );
    
    void Check( shared_ptr<Patch> &layout );

private:    
    bool Policy(const FreeZone *zone, const FreeZone *child_zone) const;
    FreeZoneMergeImpl impl;
};


}

#endif
