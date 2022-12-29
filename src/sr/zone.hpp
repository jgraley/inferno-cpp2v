#ifndef ZONE_HPP
#define ZONE_HPP

#include "link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"

#include <unordered_set>

namespace SR 
{

// ------------------------- Updater --------------------------    
    
class Updater : public Traceable
{
public:
    virtual void Insert( TreePtr<Node> node ) const = 0;
};    
    
// ------------------------- SingularUpdater --------------------------    
    
class SingularUpdater : public Updater
{
public:
    explicit SingularUpdater( TreePtrInterface *tree_ptr );
    void Insert( TreePtr<Node> node ) const final;
    
    string GetTrace() const;

private:
    TreePtrInterface *tree_ptr;
};    
    
// ------------------------- ContainerUpdater --------------------------    
    
class ContainerUpdater : public Updater
{
public:
    explicit ContainerUpdater( ContainerInterface *container );
    void Insert( TreePtr<Node> node ) const final;
    
    string GetTrace() const;

private:
    ContainerInterface *container;
};    
    
// ------------------------- FreeZone --------------------------

// FreeZone is for zones that are not anywhere in the current
// x tree. They are assumed to be "orphaned" and so for minimality
// we express the base as a TreePtr<Node>. See #623.
class FreeZone : public Traceable
{ 
public:
    static FreeZone CreateEmpty();

    explicit FreeZone( TreePtr<Node> base, list<shared_ptr<Updater>> terminii = {} );
      
    TreePtr<Node> GetBase() const;
    const list<shared_ptr<Updater>> &GetTerminii() const;
    bool IsEmpty() const;

    string GetTrace() const;
    
private:
    TreePtr<Node> base;
    list<shared_ptr<Updater>> terminii;
};

// ------------------------- TreeZone --------------------------

// TreeZone is for zones that are within the current x tree. All
// nodes in the tree have an XLink, including at the root, and we
// prefer to keep track of the XLink to the base node for precision
// and convenience. See #623.
class TreeZone : public Traceable
{ 
public:
    static TreeZone CreateEmpty( XLink base );

    explicit TreeZone( XLink base, list<XLink> terminii = {} );
    TreeZone( XLink base, const FreeZone &free_zone );
      
    XLink GetBase() const;
    list<XLink> GetTerminii() const;
    bool IsEmpty() const;
    
    string GetTrace() const;

private:
    XLink base;    
    list<XLink> terminii;
};
 
 
}

#endif
