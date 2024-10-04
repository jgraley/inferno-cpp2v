#ifndef UPDATER_HPP
#define UPDATER_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"

#include <unordered_set>

namespace SR 
{

// ------------------------- Terminus --------------------------    
    
class Terminus : public Traceable
{
public:
    virtual void Populate( TreePtr<Node> child_base,                               
                           list<shared_ptr<Terminus>> child_terminii = {} ) = 0;
};    
    
// ------------------------- SingularTerminus --------------------------    
    
class SingularTerminus : public Terminus
{
public:
    explicit SingularTerminus( TreePtrInterface *tree_ptr );
    void Populate( TreePtr<Node> child_base,                               
                   list<shared_ptr<Terminus>> child_terminii = {} ) final;
    
    string GetTrace() const;

private:
    TreePtrInterface * const tree_ptr;
};    
    
// ------------------------- ContainerTerminus --------------------------    
    
class ContainerTerminus : public Terminus
{
public:
    explicit ContainerTerminus( ContainerInterface *dest_container_,
                                ContainerInterface::iterator it_dest_placeholder_ );             
    void Populate( TreePtr<Node> child_base, 
                   list<shared_ptr<Terminus>> child_terminii = {} ) final;
    
    static TreePtr<Node> GetPlaceholder();
    string GetTrace() const;

private:
    ContainerInterface * const dest_container;
    ContainerInterface::iterator it_dest_placeholder;
    
    bool joined = false;
};    
    
}

#endif
