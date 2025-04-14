#ifndef MISC_PASSES_HPP
#define MISC_PASSES_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"

namespace SR 
{

// ------------------------- ToMutablePass --------------------------

/**
 * All tree zones are converted to mutable tree zones whose boundaries
 * are made of Mutators (as opposed to XLinks). Mutators have handy
 * properties:
 * - Can mutate the tree, including deep mutation
 * - Can remain valid across exchanges 
 * - Are managed via shared_ptr to always alias, so all remain valid
 */ 
class ToMutablePass
{
public:    
    ToMutablePass(XTreeDatabase *db_);
    
    // Can change the supplied shared ptr
    void Run( shared_ptr<Patch> &layout );

private:
    XTreeDatabase * const db;
};

// ------------------------- ProtectDEPass --------------------------

/**
 * We must duplicate any tree zones that are in domain extension extra trees
 * because we want these to stick around. If we added them to the layout 
 * they'd be detected anyway, but this is simpler, and ensures we duplicate
 * into the main tree and leave the extra tree alone (less confusing if
 * DomainExtension is the only thing that changes them).
 */
class ProtectDEPass 
{
public:
    ProtectDEPass( const XTreeDatabase *db );
    void Run( shared_ptr<Patch> &layout );

private:
    const XTreeDatabase * const db;
};

// ------------------------- EmptyZonePass --------------------------

/**
 * Drop empty tree zones, because they break correspondance betweek 
 * base XLink and zone.
 */ 
class EmptyZonePass
{
public:    
    EmptyZonePass();
    
    // Can change the supplied shared ptr
    void Run( shared_ptr<Patch> &layout );
    
    // Just ASSERT no empty zones
    void Check( shared_ptr<Patch> &layout );
};

// ------------------------- InsertIntrinsicPass --------------------------

/**
 * All the free zones are added to the intrinsic tables in the DB.
 */
class InsertIntrinsicPass 
{
public:
    InsertIntrinsicPass( XTreeDatabase *db );
    void Run( shared_ptr<Patch> &layout );

private:
    XTreeDatabase * const db;
};

// ------------------------- MarkersPass --------------------------

/**
 * Call into agents/engines informing them where certain important
 * nodes are in the new tree. We have to have completed any duplication
 * before doing this, since we will refer to the actual nodes. And 
 * we should still posess all the zones for the updated tree.
 */ 
class MarkersPass 
{
public:
    MarkersPass( const XTreeDatabase *db );
    void Run( shared_ptr<Patch> &layout );
    void Check( shared_ptr<Patch> &layout );

private:
    const XTreeDatabase * const db;
};

// ------------------------- DuplicateAllPass --------------------------

/** 
 * Not used for main tree update, but is used by builder action. This is the
 * lazy algorithm that just makes a copy of everything in the layout. If
 * followed by a MergeFreesPass, the entire updated tree will be left in
 * a single (subtree) free zone.
 */ 
class DuplicateAllPass
{
public:    
    DuplicateAllPass();
    
    // Can change the supplied shared ptr
    void Run( shared_ptr<Patch> &layout );
    
    // Just ASSERT all free zones
    void Check( shared_ptr<Patch> &layout );
};

// ------------------------- ScaffoldChecker --------------------------

class ScaffoldChecker 
{
public:
    void Run( shared_ptr<Patch> layout );
    static void Check(shared_ptr<Patch> patch);
};
}

#endif
