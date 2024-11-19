#ifndef TREE_ZONE_HPP
#define TREE_ZONE_HPP

#include "zone.hpp"
#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"
#include "free_terminus.hpp"
#include "df_relation.hpp"
#include "duplicate.hpp"

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
	typedef vector<XLink>::const_iterator TerminusIterator;

    static TreeZone CreateSubtree( XLink base );
    static TreeZone CreateEmpty( XLink base );

    explicit TreeZone( XLink base, vector<XLink> terminii );
    TreeZone( const TreeZone &other );
      
    bool IsEmpty() const override;
	int GetNumTerminii() const override;
    TreePtr<Node> GetBaseNode() const override;

    XLink GetBaseXLink() const;
    XLink &GetBaseXLink();
    vector<XLink> GetTerminusXLinks() const;

    FreeZone Duplicate() const;
    
    TerminusIterator GetTerminiiBegin() const;
    TerminusIterator GetTerminiiEnd() const;
    
    void Patch( FreeZone &&free_zone );

    string GetTrace() const;

    void DBCheck(const XTreeDatabase *db) const;

private:

    XLink base;    
    vector<XLink> terminii;    
};
 
}

#endif
