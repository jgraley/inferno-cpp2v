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
    
}

#endif
