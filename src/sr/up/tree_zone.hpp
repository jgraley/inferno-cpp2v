#ifndef TREE_ZONE_HPP
#define TREE_ZONE_HPP

#include "zone.hpp"
#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"
#include "terminus.hpp"

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
    static TreeZone CreateSubtree( XLink base );
    static TreeZone CreateEmpty( XLink base );

    explicit TreeZone( XLink base, vector<XLink> terminii );
      
    bool IsEmpty() const override;
	int GetNumTerminii() const override;
    TreePtr<Node> GetBaseNode() const override;

    XLink GetBaseXLink() const;
    vector<XLink> GetTerminusXLinks() const;
    XLink GetTerminusXLink(int ti) const;
    void DBCheck( const XTreeDatabase *db ) const;

    FreeZone Duplicate( XTreeDatabase *x_tree_db ) const;
    void Update( XTreeDatabase *x_tree_db, const FreeZone &free_zone ) const;

    static bool IsOverlap( const XTreeDatabase *db, const TreeZone &l, const TreeZone &r );
    
    string GetTrace() const;

private:
    XLink base;    
    vector<XLink> terminii;
};
 
 
}

#endif
