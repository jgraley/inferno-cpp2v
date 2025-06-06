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
    void Run( shared_ptr<Patch> &layout );
    void Check( shared_ptr<Patch> &layout );
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

// ------------------------- ValidateTreeZones --------------------------

class ValidateTreeZones 
{
public:
    ValidateTreeZones( const XTreeDatabase *db );
    void Run( shared_ptr<Patch> layout );

private:
    const XTreeDatabase * const db;
};

}


#endif
