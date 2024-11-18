#ifndef FREE_TERMINUS_HPP
#define FREE_TERMINUS_HPP

#include "../link.hpp"
#include "common/standard.hpp"
#include "node/specialise_oostd.hpp"

#include <unordered_set>

namespace SR 
{

// ------------------------- FreeTerminus --------------------------    
    
class FreeTerminus : public Traceable
{
public:
	FreeTerminus( TreePtr<Node> parent_node_ );
	
    virtual void Populate( TreePtr<Node> child_base,                               
                           list<shared_ptr<FreeTerminus>> child_terminii = {} ) = 0;
	TreePtr<Node> GetParentNode() const;
	
	// Only valid after populate
	virtual const TreePtrInterface *GetTreePtrInterface() const = 0;

private:
	TreePtr<Node> parent_node;
};    
    
// ------------------------- FreeSingularTerminus --------------------------    
    
class FreeSingularTerminus : public FreeTerminus
{
public:
    explicit FreeSingularTerminus( TreePtr<Node> parent_node, TreePtrInterface *dest_tree_ptr_ );
    void Populate( TreePtr<Node> child_base,                               
                   list<shared_ptr<FreeTerminus>> child_terminii = {} ) final;
	const TreePtrInterface *GetTreePtrInterface() const final;
    
    string GetTrace() const;

private:
    TreePtrInterface * const dest_tree_ptr;
};    
    
// ------------------------- FreeContainerTerminus --------------------------    
    
class FreeContainerTerminus : public FreeTerminus
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
    explicit FreeContainerTerminus( TreePtr<Node> parent_node, 
								ContainerInterface *dest_container_,
                                ContainerInterface::iterator it_dest_placeholder_ );             

	FreeContainerTerminus &operator=( const FreeContainerTerminus &other );

    void Populate( TreePtr<Node> child_base, 
                   list<shared_ptr<FreeTerminus>> child_terminii = {} ) final;
    
    static TreePtr<Node> MakePlaceholder();
    
    static shared_ptr<FreeContainerTerminus> FindMatchingTerminus( ContainerInterface *container,
                                                               ContainerInterface::iterator it_placeholder,
                                                               list<shared_ptr<FreeTerminus>> &candidate_terminii );
	const TreePtrInterface *GetTreePtrInterface() const final;
    
    void Validate() const;
    string GetTrace() const;

private:
    ContainerInterface *dest_container;
    ContainerInterface::iterator it_dest_placeholder;
    ContainerInterface::iterator it_dest_populated;
    
    bool populated = false;
};    
    
}

#endif
