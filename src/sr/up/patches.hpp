#ifndef PATCHES_HPP
#define PATCHES_HPP

#include "common/common.hpp"
#include "node/tree_ptr.hpp"
#include "../db/free_zone.hpp"
#include "../db/tree_zone.hpp"
#include "../link.hpp"
#include "../db/duplicate.hpp"
#include "../scr_engine.hpp"
#include "../db/db_common.hpp"
#include "up_common.hpp"

#include <functional>

#define USE_SWAPS

namespace SR 
{
class XTreeDatabase;

class FreePatch;

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
    
    void AddOriginators( const Originators &new_originators );
    const Originators &GetOriginators() const;
    void ClearOriginators();

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

    string GetTrace() const override;

private:
    list<shared_ptr<Patch>> child_patches;
    Originators originators;    
};

// ------------------------- TreePatch --------------------------

// Construct with tree zone and child patches for terminii. Markers can 
// then be added. On evaluate: duplicate into a free zone, apply markers,
// populate it immediately (rule #726), and return the resulting FreeZone. 
// Due to rule #726, we cannot provide a merge method (or we could add support 
// for markers in interior possibly not at base).
class TreePatch : public Patch
{
public:
	enum class Intent
	{
		DEFAULT,
		MOVEABLE,
		COPYABLE
	};

    TreePatch( const TreeZone &zone_, list<shared_ptr<Patch>> &&child_patches );
    TreePatch( const TreeZone &zone_ );

    TreeZone *GetZone() override;
    const TreeZone *GetZone() const override;
    
    TreeZone GetXTreeZone() const;
    shared_ptr<FreePatch> DuplicateToFree() const;
    
    static void ForTreeChildren( shared_ptr<Patch> base,
                                 function<void(shared_ptr<Patch> &patch)> func );
    static void ForTreeChildren( shared_ptr<Patch> base,
                                 function<void(shared_ptr<TreePatch> &patch)> func );
    static void ForTreeDepthFirstWalk( shared_ptr<Patch> &base,
                                       function<void(shared_ptr<Patch> &patch)> func_in,
                                       function<void(shared_ptr<Patch> &patch)> func_out );
    static void ForTreeDepthFirstWalk( shared_ptr<Patch> &base,
                                       function<void(shared_ptr<TreePatch> &patch)> func_in,
                                       function<void(shared_ptr<TreePatch> &patch)> func_out );
    
    void SetIntent(Intent in);
    Intent GetIntent() const;
    
    string GetTrace() const final;
    
private:
    TreeZone zone;
    Intent intent = Intent::DEFAULT;
};

// ------------------------- FreePatch --------------------------

// Construct with free zone and child patches for terminii. Markers can 
// then be added. On evaluate: populate the zone, and return the resulting 
// FreeZone. Rule #726 means there can never be duplicate, clone, move etc, 
// because we mark for embedded immediately (but this means we can merge 
// without needing to represent markers in interior possibly not at base).
class FreePatch : public Patch
{
public:
    FreePatch( const FreeZone &zone_, list<shared_ptr<Patch>> &&child_patches );
    FreePatch( const FreeZone &zone_ );

    ChildPatchIterator SpliceOver( ChildPatchIterator it_child, 
                                   list<shared_ptr<Patch>> &&child_patches );

    FreeZone *GetZone() final;
    const FreeZone *GetZone() const final;
   	shared_ptr<TreePatch> ConvertToTree(const TreeZone &tree_zone) const;

    static void ForFreeChildren(shared_ptr<Patch> base,
                                function<void(shared_ptr<Patch> &patch)> func);
    static void ForFreeChildren(shared_ptr<Patch> base,
                                function<void(shared_ptr<FreePatch> &patch)> func);
    static void ForFreeDepthFirstWalk( shared_ptr<Patch> &base,
                                       function<void(shared_ptr<Patch> &patch)> func_in,
                                       function<void(shared_ptr<Patch> &patch)> func_out );
    static void ForFreeDepthFirstWalk( shared_ptr<Patch> &base,
                                       function<void(shared_ptr<FreePatch> &patch)> func_in,
                                       function<void(shared_ptr<FreePatch> &patch)> func_out );
    string GetTrace() const final;

private:
    FreeZone zone;   
};

}

#endif
