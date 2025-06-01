#ifndef TREE_ZONE_HPP
#define TREE_ZONE_HPP

#include "zone.hpp"
#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"
#include "mutator.hpp"
#include "df_relation.hpp"
#include "duplicate.hpp"
#include "db_common.hpp"

#include <unordered_set>

namespace SR 
{
class XTreeDatabase;    
class FreeZone;

// ------------------------- TreeZone --------------------------

class TreeZone : public Zone
{
public:
	static const DBCommon::TreeOrdinal default_ordinal = DBCommon::TreeOrdinal::MAIN;
	explicit TreeZone(DBCommon::TreeOrdinal ordinal_ = default_ordinal);

    virtual XLink GetBaseXLink() const = 0;
    virtual vector<XLink> GetTerminusXLinks() const = 0;
    virtual XLink GetTerminusXLink(size_t index) const = 0;

    DBCommon::TreeOrdinal GetTreeOrdinal() const;

    unique_ptr<FreeZone> Duplicate() const;
    void Validate(const XTreeDatabase *db) const;

private:
	DBCommon::TreeOrdinal ordinal;
};

// ------------------------- XTreeZone --------------------------

// XTreeZone is for zones that are within the current x tree. All
// nodes in the tree have an XLink, including at the root, and we
// prefer to keep track of the XLink to the base node for precision
// and convenience. See #623.
class XTreeZone : public TreeZone
{ 
public:
    static XTreeZone CreateSubtree( XLink base, 
								    DBCommon::TreeOrdinal ordinal_ = default_ordinal );
    static XTreeZone CreateEmpty( XLink base );

    explicit XTreeZone( XLink base, 
                        vector<XLink> terminii, 
                        DBCommon::TreeOrdinal ordinal_ = default_ordinal );
      
    bool IsEmpty() const override;
    size_t GetNumTerminii() const override;
    TreePtr<Node> GetBaseNode() const override;

    XLink GetBaseXLink() const override;
    vector<XLink> GetTerminusXLinks() const override;
    XLink GetTerminusXLink(size_t index) const override;
    
    void SetBaseXLink(XLink new_base);

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
                              vector<Mutator> &&terminii_,
                              DBCommon::TreeOrdinal ordinal_ = default_ordinal );

    bool IsEmpty() const override;
    size_t GetNumTerminii() const override;
    TreePtr<Node> GetBaseNode() const override;

    XLink GetBaseXLink() const override;
    vector<XLink> GetTerminusXLinks() const override;
    XLink GetTerminusXLink(size_t index) const override;

    const Mutator &GetBaseMutator() const;
    void SetBaseMutator( const Mutator &new_base );
    const Mutator &GetTerminusMutator(size_t index) const;	
	XTreeZone GetXTreeZone() const;

	void Swap( TreeZone &tree_zone_r, vector<TreeZone *> fixups_l, vector<TreeZone *> fixups_r );
       
    string GetTrace() const override;
    
private:
    Mutator base;
    vector<Mutator> terminii;
};
 
 
class RAIISuspendForSwapBase
{
protected:
	RAIISuspendForSwapBase() = delete;
	RAIISuspendForSwapBase( TreeZone &zone1_, TreeZone &zone2_ );
	~RAIISuspendForSwapBase();
	
	const TreeZone &zone1;
	const TreeZone &zone2;
};
 
 
}

#endif
