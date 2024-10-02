#ifndef TREE_ZONE_HPP
#define TREE_ZONE_HPP

#include "zone.hpp"
#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"
#include "terminus.hpp"
#include "../db/df_relation.hpp"

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
	typedef set<XLink, DepthFirstRelation>::iterator TerminusIterator;

    static TreeZone CreateSubtree( XTreeDatabase *db, XLink base );
    static TreeZone CreateEmpty( XTreeDatabase *db, XLink base );

    explicit TreeZone( XTreeDatabase *db_, XLink base, vector<XLink> terminii );
      
    bool IsEmpty() const override;
	int GetNumTerminii() const override;
    TreePtr<Node> GetBaseNode() const override;

    XLink GetBaseXLink() const;
    set<XLink, DepthFirstRelation> GetTerminusXLinks() const;

    FreeZone Duplicate() const;
    void Update( const FreeZone &free_zone ) const;
    
    TerminusIterator GetTerminiiBegin();
    TerminusIterator GetTerminiiEnd();

    string GetTrace() const;

private:
    void DBCheck() const;

    XTreeDatabase * const db;
	DepthFirstRelation df_rel;
    XLink base;    
    set<XLink, DepthFirstRelation> terminii;    
};
 
 
}

#endif
