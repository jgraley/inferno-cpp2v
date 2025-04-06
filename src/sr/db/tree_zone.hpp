#ifndef TREE_ZONE_HPP
#define TREE_ZONE_HPP

#include "zone.hpp"
#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"
#include "mutator.hpp"
#include "df_relation.hpp"
#include "duplicate.hpp"

#include <unordered_set>

namespace SR 
{
class XTreeDatabase;    
class FreeZone;

// ------------------------- TreeZone --------------------------

class TreeZone : public Zone
{
public:
    virtual XLink GetBaseXLink() const = 0;
    virtual XLink &GetBaseXLink() = 0;
    virtual vector<XLink> GetTerminusXLinks() const = 0;
    virtual XLink GetTerminusXLink(size_t index) const = 0;

    virtual unique_ptr<FreeZone> Duplicate() const = 0;
    virtual unique_ptr<FreeZone> MakeFreeZone(const XTreeDatabase *db) const = 0;	
};

// ------------------------- XTreeZone --------------------------

// XTreeZone is for zones that are within the current x tree. All
// nodes in the tree have an XLink, including at the root, and we
// prefer to keep track of the XLink to the base node for precision
// and convenience. See #623.
class XTreeZone : public TreeZone
{ 
public:
    static unique_ptr<XTreeZone> CreateSubtree( XLink base );
    static unique_ptr<XTreeZone> CreateEmpty( XLink base );

    explicit XTreeZone( XLink base, vector<XLink> terminii );
      
    bool IsEmpty() const override;
    size_t GetNumTerminii() const override;
    TreePtr<Node> GetBaseNode() const override;

    XLink GetBaseXLink() const override;
    XLink &GetBaseXLink() override;
    vector<XLink> GetTerminusXLinks() const override;
    XLink GetTerminusXLink(size_t index) const override;

    unique_ptr<FreeZone> Duplicate() const;
    unique_ptr<FreeZone> MakeFreeZone(const XTreeDatabase *db) const;
    
    string GetTrace() const;

    void DBCheck(const XTreeDatabase *db) const;

protected:
    XLink base;    
    vector<XLink> terminii;    
};

// ------------------------- MutableTreeZone --------------------------
 
class MutableTreeZone : public XTreeZone
{
public:
    explicit MutableTreeZone( XTreeZone *tz, 
                              shared_ptr<Mutator> &&base_mutator_, 
                              vector<shared_ptr<Mutator>> &&terminii_mutators_ );

    void Exchange( FreeZone &free_zone );
    
private:
    shared_ptr<Mutator> base_mutator;
    vector<shared_ptr<Mutator>> terminii_mutators;
};
 
}

#endif
