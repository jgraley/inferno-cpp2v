#ifndef UPDATER_HPP
#define UPDATER_HPP

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
    virtual void Apply( TreePtr<Node> node ) = 0;
};    
    
// ------------------------- SingularUpdater --------------------------    
    
class SingularUpdater : public Updater
{
public:
    explicit SingularUpdater( TreePtrInterface *tree_ptr );
    void Apply( TreePtr<Node> node ) final;
    
    string GetTrace() const;

private:
    TreePtrInterface * const tree_ptr;
};    
    
// ------------------------- ContainerUpdater --------------------------    
    
class ContainerUpdater : public Updater
{
public:
    explicit ContainerUpdater( ContainerInterface *container,
                               ContainerInterface::iterator it_ );             
    void Apply( TreePtr<Node> node ) final;
    
    static TreePtr<Node> GetPlaceholder();
    string GetTrace() const;

private:
    ContainerInterface * const container;
    ContainerInterface::iterator it_begin;
    ContainerInterface::iterator it_end;
    
    bool dirty = false;
};    
    
}

#endif
