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
    virtual void Populate( TreePtr<Node> node ) = 0;
};    
    
// ------------------------- SingularTerminus --------------------------    
    
class SingularTerminus : public Terminus
{
public:
    explicit SingularTerminus( TreePtrInterface *tree_ptr );
    void Populate( TreePtr<Node> node ) final;
    
    string GetTrace() const;

private:
    TreePtrInterface * const tree_ptr;
};    
    
// ------------------------- ContainerTerminus --------------------------    
    
class ContainerTerminus : public Terminus
{
public:
    explicit ContainerTerminus( ContainerInterface *container,
                               ContainerInterface::iterator it_placeholder_ );             
    void Populate( TreePtr<Node> base ) final;
    
    static TreePtr<Node> GetPlaceholder();
    string GetTrace() const;

private:
    ContainerInterface * const container;
    ContainerInterface::iterator it_placeholder;
    
    bool joined = false;
};    
    
}

#endif
