#ifndef PATCHES_HPP
#define PATCHES_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../db/duplicate.hpp"
#include "../scr_engine.hpp"
#include <functional>

namespace SR 
{
class XTreeDatabase;

class FreeZonePatch;


// ------------------------- Patch --------------------------

class Patch : public Traceable
{
public:
    typedef list<shared_ptr<Patch>>::iterator ChildPatchIterator;

protected:
    Patch( list<shared_ptr<Patch>> &&child_patches_ );
    Patch();
    
public:
    size_t GetNumChildren() const;
    ChildPatchIterator GetChildrenBegin();
    ChildPatchIterator GetChildrenEnd();
    list<shared_ptr<Patch>> &GetChildren();
    const list<shared_ptr<Patch>> &GetChildren() const;
    list<shared_ptr<Patch>> &&MoveChildren();
    
    void AddEmbeddedMarker( RequiresSubordinateSCREngine *new_marker );
    virtual void AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers ) = 0;
    virtual list<RequiresSubordinateSCREngine *> GetEmbeddedMarkers() const = 0;
    virtual void ClearEmbeddedMarkers() = 0;

    virtual Zone *GetZone() = 0;
    virtual const Zone *GetZone() const = 0;
            
    string GetChildrenTrace() const;

    static void ForChildren(shared_ptr<Patch> base,
                            function<void(shared_ptr<Patch> &patch)> func);

    static void ForDepthFirstWalk( shared_ptr<Patch> &base,
                                   function<void(shared_ptr<Patch> &patch)> func_in,
                                   function<void(shared_ptr<Patch> &patch)> func_out );

    void DepthFirstWalkImpl(function<void(shared_ptr<Patch> &patch)> func_in,
                            function<void(shared_ptr<Patch> &patch)> func_out);

private:
    list<shared_ptr<Patch>> child_patches;

};


// ------------------------- TreeZonePatch --------------------------

// Construct with tree zone and child patches for terminii. Markers can 
// then be added. On evaluate: duplicate into a free zone, apply markers,
// populate it immediately (rule #726), and return the resulting FreeZone. 
// Due to rule #726, we cannot provide a merge method (or we could add support 
// for markers in interior possibly not at base).
class TreeZonePatch : public Patch
{
public:
    TreeZonePatch( unique_ptr<TreeZone> zone_, list<shared_ptr<Patch>> &&child_patches );
    TreeZonePatch( unique_ptr<TreeZone> zone_ );
    
    void AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers ) final;
    list<RequiresSubordinateSCREngine *> GetEmbeddedMarkers() const final;
    void ClearEmbeddedMarkers() final;

    TreeZone *GetZone() override;
    const TreeZone *GetZone() const override;
    void SetZone( unique_ptr<TreeZone> &&new_zone );
    
    shared_ptr<FreeZonePatch> DuplicateToFree() const;
    
    static void ForTreeChildren( shared_ptr<Patch> base,
                                 function<void(shared_ptr<Patch> &patch)> func );
    static void ForTreeChildren( shared_ptr<Patch> base,
                                 function<void(shared_ptr<TreeZonePatch> &patch)> func );
    static void ForTreeDepthFirstWalk( shared_ptr<Patch> &base,
                                       function<void(shared_ptr<Patch> &patch)> func_in,
                                       function<void(shared_ptr<Patch> &patch)> func_out );
    static void ForTreeDepthFirstWalk( shared_ptr<Patch> &base,
                                       function<void(shared_ptr<TreeZonePatch> &patch)> func_in,
                                       function<void(shared_ptr<TreeZonePatch> &patch)> func_out );
    
    string GetTrace() const final;
    
private:
    unique_ptr<TreeZone> zone;
    list<RequiresSubordinateSCREngine *> embedded_markers;
};

// ------------------------- FreeZonePatch --------------------------

// Construct with free zone and child patches for terminii. Markers can 
// then be added. On evaluate: populate the zone, and return the resulting 
// FreeZone. Rule #726 means there can never be duplicate, clone, move etc, 
// because we mark for embedded immediately (but this means we can merge 
// without needing to represent markers in interior possibly not at base).
class FreeZonePatch : public Patch
{
public:
    FreeZonePatch( unique_ptr<FreeZone> zone_, list<shared_ptr<Patch>> &&child_patches );
    FreeZonePatch( unique_ptr<FreeZone> zone_ );

    void AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers ) final;
    list<RequiresSubordinateSCREngine *> GetEmbeddedMarkers() const final;
    void ClearEmbeddedMarkers() final;

    ChildPatchIterator SpliceOver( ChildPatchIterator it_child, 
                                   list<shared_ptr<Patch>> &&child_patches );

    FreeZone *GetZone() final;
    const FreeZone *GetZone() const final;
    
    static void ForFreeChildren(shared_ptr<Patch> base,
                                function<void(shared_ptr<Patch> &patch)> func);
    static void ForFreeChildren(shared_ptr<Patch> base,
                                function<void(shared_ptr<FreeZonePatch> &patch)> func);
    static void ForFreeDepthFirstWalk( shared_ptr<Patch> &base,
                                       function<void(shared_ptr<Patch> &patch)> func_in,
                                       function<void(shared_ptr<Patch> &patch)> func_out );
    static void ForFreeDepthFirstWalk( shared_ptr<Patch> &base,
                                       function<void(shared_ptr<FreeZonePatch> &patch)> func_in,
                                       function<void(shared_ptr<FreeZonePatch> &patch)> func_out );
    string GetTrace() const final;

private:
    unique_ptr<FreeZone> zone;   
};

// ------------------------- MovesMap --------------------------

struct MoveMap
{
	public:
	map<TreePtr<Node>, unique_ptr<FreeZone>> moves_map;
};


}

#endif
