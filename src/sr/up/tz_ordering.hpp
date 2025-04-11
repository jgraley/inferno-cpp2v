#ifndef TZ_ORDERING_HPP
#define TZ_ORDERING_HPP

#include "patches.hpp"
#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

// ------------------------- TreeZoneOrderingHandler --------------------------

class TreeZoneOrderingHandler
{
public:    
    typedef list<shared_ptr<Patch> *> PatchPtrList;

    TreeZoneOrderingHandler(XTreeDatabase *db_);
    
    // Can change the supplied shared ptr
    void Run( shared_ptr<Patch> &layout );
    
    // Just ASSERT no empty zones
    void Check( shared_ptr<Patch> &layout );

private:    
    struct PatchRecord
    {
        shared_ptr<Patch> *patch_ptr; // Double pointer to patch node so we can mutate
        bool out_of_order; 
    };
    typedef vector<PatchRecord> PatchRecords;

	void MoveTreeZoneToFreePatch( shared_ptr<Patch> *target_patch );
    void ConstrainAnyPatchToDescendants( shared_ptr<Patch> &start_patch, 
								         shared_ptr<Mutator> ancestor,
								         bool just_check );
    void ConstrainTreePatchesToRange( PatchRecords &patch_records, 
                                      shared_ptr<Mutator> front_ancestor,
                                      shared_ptr<Mutator> back_ancestor,
                                      bool just_check );
    void ConstrainChildrenToTerminii( shared_ptr<TreeZonePatch> &tree_patch, 
                                      bool just_check );
    void AppendNextDescendantTreePatches( shared_ptr<Patch> &patch, 
                                      PatchRecords &patch_records );
    void FindOutOfOrderTreePatches( PatchRecords &patch_records, 
									XLink front_ancestor,
									XLink back_ancestor,
									bool just_check );
    shared_ptr<TreeZonePatch> GetTreePatch(const PatchRecord &patch_record) const;
    XLink GetBaseXLink(const PatchRecord &patch_record) const;
    shared_ptr<Mutator> GetBaseMutator(const PatchRecord &patch_record) const;
                          
    XTreeDatabase * const db;
    SR::DepthFirstRelation dfr;        
    PatchPtrList out_of_order_patch_ptrs;                  
};

// ------------------------- AltTreeZoneOrderingChecker --------------------------

class AltTreeZoneOrderingChecker
{
public:
    AltTreeZoneOrderingChecker(const XTreeDatabase *db_);
    
    void Check( shared_ptr<Patch> layout );

private:
    void Worker( shared_ptr<Patch> patch, bool base_equal_ok );
    void CheckXlink( XLink x, bool equal_ok );
        
    const XTreeDatabase * const db;
    SR::DepthFirstRelation dfr;                          
    XLink prev_xlink;
};

}

#endif
