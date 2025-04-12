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
    typedef list<shared_ptr<Patch>>::iterator ChildExpressionIterator;

protected:
    Patch( list<shared_ptr<Patch>> &&child_patches_ );
    Patch();
    
public:
    size_t GetNumChildExpressions() const;
    ChildExpressionIterator GetChildrenBegin();
    ChildExpressionIterator GetChildrenEnd();
    list<shared_ptr<Patch>> &GetChildExpressions();
    const list<shared_ptr<Patch>> &GetChildExpressions() const;
    list<shared_ptr<Patch>> &&MoveChildExpressions();
    
    string GetChildExpressionsTrace() const;

    static void ForChildren(shared_ptr<Patch> base,
                            function<void(shared_ptr<Patch> &patch)> func);

    static void ForDepthFirstWalk( shared_ptr<Patch> &base,
                                   function<void(shared_ptr<Patch> &patch)> func_in,
                                   function<void(shared_ptr<Patch> &patch)> func_out );

    void DepthFirstWalkImpl(function<void(shared_ptr<Patch> &patch)> func_in,
                            function<void(shared_ptr<Patch> &patch)> func_out);

    list<shared_ptr<Patch>> GetChildren() const;

private:
    list<shared_ptr<Patch>> child_patches;

};

// ------------------------- ZonePatch --------------------------

// Construct with any zone and optional marker M. On evaluate: populate the
// zone, apply marker and return the resulting FreeZone. 
class ZonePatch : public Patch
{
protected:
    ZonePatch( list<shared_ptr<Patch>> &&child_patches_ );
    ZonePatch();

public:
    void AddEmbeddedMarker( RequiresSubordinateSCREngine *new_marker );
    virtual void AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers ) = 0;
    virtual list<RequiresSubordinateSCREngine *> GetEmbeddedMarkers() const = 0;
    virtual void ClearEmbeddedMarkers() = 0;
    
    virtual Zone *GetZone() = 0;
    virtual const Zone *GetZone() const = 0;
};


// ------------------------- TreeZonePatch --------------------------

// Construct with tree zone and child patches for terminii. Markers can 
// then be added. On evaluate: duplicate into a free zone, apply markers,
// populate it immediately (rule #726), and return the resulting FreeZone. 
// Due to rule #726, we cannot provide a merge method (or we could add support 
// for markers in interior possibly not at base).
class TreeZonePatch : public ZonePatch
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
    
    static void ForChildren(shared_ptr<Patch> base,
                            function<void(shared_ptr<TreeZonePatch> &patch)> func);
    static void ForDepthFirstWalk( shared_ptr<Patch> &base,
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
class FreeZonePatch : public ZonePatch
{
public:
    FreeZonePatch( unique_ptr<FreeZone> zone_, list<shared_ptr<Patch>> &&child_patches );
    FreeZonePatch( unique_ptr<FreeZone> zone_ );

    void AddEmbeddedMarkers( list<RequiresSubordinateSCREngine *> &&new_markers ) final;
    list<RequiresSubordinateSCREngine *> GetEmbeddedMarkers() const final;
    void ClearEmbeddedMarkers() final;

    ChildExpressionIterator SpliceOver( ChildExpressionIterator it_child, 
                                        list<shared_ptr<Patch>> &&child_patches );

    FreeZone *GetZone() final;
    const FreeZone *GetZone() const final;
    
    static void ForChildren(shared_ptr<Patch> base,
                            function<void(shared_ptr<FreeZonePatch> &patch)> func);
    static void ForDepthFirstWalk( shared_ptr<Patch> &base,
                                   function<void(shared_ptr<FreeZonePatch> &patch)> func_in,
                                   function<void(shared_ptr<FreeZonePatch> &patch)> func_out );

    string GetTrace() const final;

private:
    unique_ptr<FreeZone> zone;
};

}

#endif
