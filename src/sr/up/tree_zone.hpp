#ifndef TREE_ZONE_HPP
#define TREE_ZONE_HPP

#include "zone.hpp"
#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"
#include "updater.hpp"

#include <unordered_set>

namespace SR 
{
class XTreeDatabase;    

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
      
    XLink GetBaseXLink() const;
    vector<XLink> GetTerminusXLinks() const;
	int GetNumTerminii() const;
    XLink GetTerminusXLink(int ti) const;
    bool IsEmpty() const;
    void DBCheck( const XTreeDatabase *db ) const;
    //FreeZone Duplicate( TreeZone &zone, 

    static bool IsOverlap( const XTreeDatabase *db, const TreeZone &l, const TreeZone &r );
    
    string GetTrace() const;

private:
    XLink base;    
    vector<XLink> terminii;
};
 
 
}

#endif
