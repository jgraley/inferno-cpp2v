#ifndef MUTATOR_HPP
#define MUTATOR_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"

#include <unordered_set>

namespace SR 
{

// ------------------------- Mutator --------------------------    
    
class Mutator : public Traceable
{
public:
    Mutator( TreePtr<Node> parent_node_ );
    
    virtual TreePtr<Node> ExchangeChild( TreePtr<Node> new_child,                               
                                         list<shared_ptr<Mutator>> child_terminii = {} ) = 0;
    TreePtr<Node> GetParentNode() const;
    bool IsAtRoot() const;
    
    // After population, a mutator can now provide an XLink that's valid for the new boundary
    virtual const TreePtrInterface *GetTreePtrInterface() const = 0; // Only valid after populate
    XLink GetXLink() const; // Only valid after populate

private:
    TreePtr<Node> parent_node;
};    
    
// ------------------------- SingularMutator --------------------------    
    
class SingularMutator : public Mutator
{
public:
    explicit SingularMutator( TreePtr<Node> parent_node, TreePtrInterface *dest_tree_ptr_ );
    TreePtr<Node> ExchangeChild( TreePtr<Node> new_child,                               
                          list<shared_ptr<Mutator>> child_terminii = {} ) final;
    const TreePtrInterface *GetTreePtrInterface() const final;
    
    string GetTrace() const;

private:
    TreePtrInterface * const dest_tree_ptr;
};    
    
// ------------------------- ContainerMutator --------------------------    
    
class ContainerMutator : public Mutator
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
    explicit ContainerMutator( TreePtr<Node> parent_node, 
                                ContainerInterface *dest_container_,
                                ContainerInterface::iterator it_dest_ );             

    ContainerMutator &operator=( const ContainerMutator &other );

    TreePtr<Node> ExchangeChild( TreePtr<Node> new_child, 
                          list<shared_ptr<Mutator>> child_terminii = {} ) final;
    
    static TreePtr<Node> MakePlaceholder();
    
    static shared_ptr<ContainerMutator> FindMatchingTerminus( ContainerInterface *container,
                                                               ContainerInterface::iterator it_placeholder,
                                                               list<shared_ptr<Mutator>> &candidate_terminii );
    const TreePtrInterface *GetTreePtrInterface() const final;
    
    void Validate() const;
    string GetTrace() const;

private:
    ContainerInterface *dest_container;
    ContainerInterface::iterator it_dest;
    ContainerInterface::iterator it_dest_populated;
    
    bool populated = false;
};    
    
}

#endif
