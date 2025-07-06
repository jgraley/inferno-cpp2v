#ifndef MISC_PASSES_HPP
#define MISC_PASSES_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../scr_engine.hpp"
#include "patches.hpp"

namespace SR 
{

// ------------------------- ProtectDEPass --------------------------

/**
 * Any tree zones that are in domain extension extra trees must be
 * protected from move operations because they are "managed" by domain
 * extension. Thus, we mark them as COPYABLE so that in due course
 * they will be duplicated into free zones, which can go into the main
 * tree safely.
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
 * Drop empty tree zones, because they break correspondance between
 * base XLink and zone - the same XLink becomes the base of more than 
 * one zone.
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

// ------------------------- GetTreePatchAssignmentsPass --------------------------

/**
 * Extract assignments for tree patches. Combines the originator info 
 * with the exentual locations in the new X tree.
 */ 
class GetTreePatchAssignmentsPass 
{
public:
    Assignments Run( shared_ptr<Patch> &layout );
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

// ------------------------- SetTreeOrdinals --------------------------

/**
 * Tree Zones arrive from the engine/agents with an invalid tree ordinal.
 * They could be in main tree or DE extra trees. Determine and set 
 * correctly using the database.
 */ 
class SetTreeOrdinals 
{
public:
    SetTreeOrdinals( const XTreeDatabase *db );
    void Run( shared_ptr<Patch> layout );

private:
    const XTreeDatabase * const db;
};

}


#endif
