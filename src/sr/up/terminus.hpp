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
    explicit SingularTerminus( TreePtrInterface *dest_tree_ptr_ );
    void Populate( TreePtr<Node> child_base,                               
                   list<shared_ptr<Terminus>> child_terminii = {} ) final;
    
    string GetTrace() const;

private:
    TreePtrInterface * const dest_tree_ptr;
};    
    
// ------------------------- ContainerTerminus --------------------------    
    
class ContainerTerminus : public Terminus
{
    /**
     * Why all this complicated placeholder business then?
     * It's to permit multiple terminii to refer to the same Sequence
     * with a well-defined relative order. We place a null element in
     * for each terminus so that iterators relative to different
     * terminii have different values.
     * 
     * Note: we must not determine the actual insertion iterator
     * (it_after) during construct, because the container is still
     * being filled, and we'll get end() when what we want is the next 
     * element that will be there when we apply the update.
     */  
     	
public:
    explicit ContainerTerminus( ContainerInterface *dest_container_,
                                ContainerInterface::iterator it_dest_placeholder_ );             

	ContainerTerminus &operator=( const ContainerTerminus &other );

    void Populate( TreePtr<Node> child_base, 
                   list<shared_ptr<Terminus>> child_terminii = {} ) final;
    
    static TreePtr<Node> MakePlaceholder();
    
    static shared_ptr<ContainerTerminus> FindMatchingTerminus( ContainerInterface *container,
                                                               ContainerInterface::iterator it_placeholder,
                                                               list<shared_ptr<Terminus>> &candidate_terminii );
    
    void Validate() const;
    string GetTrace() const;

private:
    ContainerInterface *dest_container;
    ContainerInterface::iterator it_dest_placeholder;
    
    bool populated = false;
};    
    
}

#endif
