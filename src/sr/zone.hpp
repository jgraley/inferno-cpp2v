#ifndef ZONE_HPP
#define ZONE_HPP

#include "link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"
#include "updater.hpp"

#include <unordered_set>

namespace SR 
{
    class XTreeDatabase;
    
// ------------------------- FreeZone --------------------------

// FreeZone is for zones that are not anywhere in the current
// x tree. They are assumed to be "orphaned" and so for minimality
// we express the base as a TreePtr<Node>. See #623.
class FreeZone : public Traceable
{ 
public:
    static FreeZone CreateSubtree( TreePtr<Node> base );
    static FreeZone CreateEmpty();

    explicit FreeZone( TreePtr<Node> base, vector<shared_ptr<Updater>> terminii );

    void AddTerminus(int ti, shared_ptr<Updater> terminus);
      
    TreePtr<Node> GetBase() const;
    vector<shared_ptr<Updater>> GetTerminii() const;
	int GetNumTerminii() const;
    shared_ptr<Updater> GetTerminus(int ti) const;
    void DropTerminus(int ti);
    bool IsEmpty() const;

    string GetTrace() const;
    
private:
    TreePtr<Node> base;
    map<int, shared_ptr<Updater>> terminii;
};

// ------------------------- TreeZone --------------------------

// TreeZone is for zones that are within the current x tree. All
// nodes in the tree have an XLink, including at the root, and we
// prefer to keep track of the XLink to the base node for precision
// and convenience. See #623.
class TreeZone : public Traceable
{ 
public:
    static TreeZone CreateSubtree( XLink base );
    static TreeZone CreateEmpty( XLink base );

    explicit TreeZone( XLink base, vector<XLink> terminii );
      
    XLink GetBase() const;
    vector<XLink> GetTerminii() const;
    XLink GetTerminus(int ti) const;
    bool IsEmpty() const;
    void DBCheck( const XTreeDatabase *db ) const;

    static bool IsOverlap( const XTreeDatabase *db, const TreeZone &l, const TreeZone &r );
    
    string GetTrace() const;

private:
    XLink base;    
    vector<XLink> terminii;
};
 
 
}

#endif
