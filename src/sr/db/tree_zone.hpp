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

// TreeZone is for zones that are within the current x tree. All
// nodes in the tree have an XLink, including at the root, and we
// prefer to keep track of the XLink to the base node for precision
// and convenience. See #623.
class TreeZone : public Zone
{ 
public:
    static TreeZone CreateSubtree( XLink base, 
								    DBCommon::TreeOrdinal ordinal_ = DBCommon::UnknownTree );
    static TreeZone CreateEmpty( XLink base );

    explicit TreeZone( XLink base, 
                        vector<XLink> terminii, 
                        DBCommon::TreeOrdinal ordinal_ = DBCommon::UnknownTree );
      
    bool IsEmpty() const override;
    size_t GetNumTerminii() const override;
    TreePtr<Node> GetBaseNode() const override;

    XLink GetBaseXLink() const;
    vector<XLink> GetTerminusXLinks() const;
    XLink GetTerminusXLink(size_t index) const;

    DBCommon::TreeOrdinal GetTreeOrdinal() const;
    void SetTreeOrdinal(DBCommon::TreeOrdinal ordinal_);
    
    void SetBaseXLink(XLink new_base);

    unique_ptr<FreeZone> Duplicate() const;
    void Validate(const XTreeDatabase *db) const;
    
    string GetTrace() const override;

protected:
    XLink base;    
    vector<XLink> terminii;    
   	DBCommon::TreeOrdinal ordinal;    
};

}

#endif
