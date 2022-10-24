#ifndef ZONE_HPP
#define ZONE_HPP

#include "link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"

#include <unordered_set>

namespace SR 
{

// ------------------------- FreeZone --------------------------

// FreeZone is for zones that are not anywhere in the current
// x tree. They are assumed to be "orphaned" and so for minimality
// we express the base as a TreePtr<Node>. See #623.
class FreeZone
{ 
public:
    explicit FreeZone( TreePtr<Node> base );
      
    TreePtr<Node> GetBase() const;
    
private:
    TreePtr<Node> base;
};

// ------------------------- TreeZone --------------------------

// TreeZone is for zones that are within the current x tree. All
// nodes in the tree have an XLink, including at the root, and we
// prefer to keep track of the XLink to the base node for precision
// and convenience. See #623.
class TreeZone
{ 
public:
    explicit TreeZone( XLink base );
    TreeZone( XLink base, const FreeZone &free_zone );
      
    XLink GetBase() const;
    
private:
    XLink base;
    set<XLink> terminii;
};
 
 
TreeZone MakeZoneFromExclusions( XLink base, unordered_set<XLink> exclusions );
    
}

#endif
