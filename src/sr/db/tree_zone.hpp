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

// TreeZone is for zones that are within the current x tree. All
// nodes in the tree have an XLink, including at the root, and we
// prefer to keep track of the XLink to the base node for precision
// and convenience. See #623.
class TreeZone : public Zone
{ 
public:
	static const DBCommon::TreeOrdinal default_ordinal = DBCommon::TreeOrdinal::MAIN;

    static TreeZone CreateSubtree( XLink base, 
								    DBCommon::TreeOrdinal ordinal_ = default_ordinal );
    static TreeZone CreateEmpty( XLink base );

    explicit TreeZone( XLink base, 
                        vector<XLink> terminii, 
                        DBCommon::TreeOrdinal ordinal_ = default_ordinal );
      
    bool IsEmpty() const override;
    size_t GetNumTerminii() const override;
    TreePtr<Node> GetBaseNode() const override;

    XLink GetBaseXLink() const;
    vector<XLink> GetTerminusXLinks() const;
    XLink GetTerminusXLink(size_t index) const;

    DBCommon::TreeOrdinal GetTreeOrdinal() const;
    
    void SetBaseXLink(XLink new_base);

    unique_ptr<FreeZone> Duplicate() const;
    void Validate(const XTreeDatabase *db) const;
    
    string GetTrace() const override;

protected:
    XLink base;    
    vector<XLink> terminii;    
   	DBCommon::TreeOrdinal ordinal;    
};

// ------------------------- MutableZone --------------------------
 
class MutableZone : public Traceable
{
public:
    explicit MutableZone( Mutator &&base_, 
                              vector<Mutator> &&terminii_,
                              DBCommon::TreeOrdinal ordinal_ );

    bool IsEmpty() const;

	TreeZone GetXTreeZone() const;

	static void Swap( MutableZone &tree_zone_l, vector<TreeZone *> fixups_l, 
	                  MutableZone &tree_zone_r, vector<TreeZone *> fixups_r );
       
    string GetTrace() const override;
    
private:
    Mutator base;
    vector<Mutator> terminii;
   	DBCommon::TreeOrdinal ordinal;
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
