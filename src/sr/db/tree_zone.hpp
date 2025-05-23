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
    virtual vector<XLink> GetTerminusXLinks() const = 0;
    virtual XLink GetTerminusXLink(size_t index) const = 0;

    unique_ptr<FreeZone> Duplicate() const;
    void Validate(const XTreeDatabase *db) const;
};

// ------------------------- XTreeZone --------------------------

// XTreeZone is for zones that are within the current x tree. All
// nodes in the tree have an XLink, including at the root, and we
// prefer to keep track of the XLink to the base node for precision
// and convenience. See #623.
class XTreeZone : public TreeZone
{ 
public:
    static XTreeZone CreateSubtree( XLink base );
    static XTreeZone CreateEmpty( XLink base );
    static XTreeZone CreateFromScaffold( XLink scaffold_xlink );

    explicit XTreeZone( XLink base, vector<XLink> terminii );
      
    bool IsEmpty() const override;
    size_t GetNumTerminii() const override;
    TreePtr<Node> GetBaseNode() const override;

    XLink GetBaseXLink() const override;
    vector<XLink> GetTerminusXLinks() const override;
    XLink GetTerminusXLink(size_t index) const override;
    
    string GetTrace() const override;

protected:
    XLink base;    
    vector<XLink> terminii;    
};

// ------------------------- MutableTreeZone --------------------------
 
class MutableTreeZone : public TreeZone
{
public:
    explicit MutableTreeZone( Mutator &&base_, 
                              vector<Mutator> &&terminii_ );

    bool IsEmpty() const override;
    size_t GetNumTerminii() const override;
    TreePtr<Node> GetBaseNode() const override;

    XLink GetBaseXLink() const override;
    vector<XLink> GetTerminusXLinks() const override;
    XLink GetTerminusXLink(size_t index) const override;

    const Mutator &GetBaseMutator() const;
    void SetBaseMutator( const Mutator &new_base );
    const Mutator &GetTerminusMutator(size_t index) const;	

	void Swap( TreeZone &tree_zone_r, vector<MutableTreeZone *> fixups_l, vector<MutableTreeZone *> fixups_r );
       
    string GetTrace() const override;
    
private:
    Mutator base;
    vector<Mutator> terminii;
};
 
}

#endif
