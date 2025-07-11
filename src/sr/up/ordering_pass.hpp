#ifndef ORDERING_PASS_HPP
#define ORDERING_PASS_HPP

#include "patches.hpp"
#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{
	
class DFPatchIndexRelation;	

/**
 * Detect where the ordering of tree zones implied by the layout differs from that 
 * found in the current tree. A tree zone is OOO (out of order) when:
 *  - A patch descends from another patch, but its TZ does not descend from the other patch's TZ
 *  - A patch's TZ is not in depth-first order wrt siblings
 * The ordering check is strict, so where we see neigbouring duplicates, all but one are
 * found to be out of order. We set the intent of the out-of-order tree patches to MOVABLE.
 */ 
class OrderingPass
{
public:    
    typedef list<shared_ptr<Patch> *> PatchPtrList;

    OrderingPass(XTreeDatabase *db_);
    
    void Run( shared_ptr<Patch> &layout );

    struct PatchRecord
    {
        shared_ptr<Patch> *patch_ptr; // Double pointer to patch node so we can mutate
        bool out_of_order; 
    };
    
private:    
    typedef vector<PatchRecord> PatchRecords;
    typedef set<size_t, DFPatchIndexRelation> PatchIndicesDFO;

	// On use of ancestor, front_ancestor, back_ancestor: when doing the 
	// exchanges on the fly, we may actually move the back end of the
	// current range out of the tree (it sometimes comes from 
	// GetLastDescendant and could therefore be at leaf level). So, at
	// least across exchanges, we keep ancestors of the bounds that
	// are at a safe level. FindOutOfOrderTreePatches() converts these.
    void ConstrainAnyPatchToDescendants( shared_ptr<Patch> &start_patch, 
								         XLink base );
    void ConstrainChildrenToTerminii( shared_ptr<TreePatch> &tree_patch );
    void AppendNextDescendantTreePatches( shared_ptr<Patch> &patch, 
                                      PatchRecords &patch_records );
    void FindOutOfOrderTreePatches( PatchRecords &patch_records, 
									XLink base );

	void MaximalIncreasingSubsequence( PatchIndicesDFO &indices_dfo );

    shared_ptr<TreePatch> GetTreePatch(const PatchRecord &patch_record) const;
    XLink GetBaseXLink(const PatchRecord &patch_record) const;
                          
    XTreeDatabase * const db;
    SR::DepthFirstRelation dfr;          
    set<XLink> in_order_bases;
};


class DFPatchIndexRelation
{
public: 
	typedef size_t KeyType;

    explicit DFPatchIndexRelation(const XTreeDatabase *db, const vector<OrderingPass::PatchRecord> &patch_records);

    /// Less operator: for use with set, map etc
    bool operator()( KeyType l_key, KeyType r_key ) const;
    Orderable::Diff Compare3Way( KeyType l_key, KeyType r_key ) const;
    pair<Orderable::Diff, DepthFirstRelation::RelType> CompareHierarchical( KeyType l_key, KeyType r_key ) const;
    
private:
    DepthFirstRelation df;
    const vector<OrderingPass::PatchRecord> &patch_records;
}; 

}

#endif
