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
protected:
	// It is a "mode" because it can change
	enum class Mode
	{
		Root,
		Singular,
		Container		
	};
	
public:	
	static shared_ptr<Mutator> MakeRootMutator( TreePtrInterface *parent_singular );
	static shared_ptr<Mutator> MakeSingularMutator( TreePtr<Node> parent_node, 
	                                                TreePtrInterface *parent_singular );
	static shared_ptr<Mutator> MakeContainerMutator( TreePtr<Node> parent_node, 
                                                     ContainerInterface *parent_container,
                                                     ContainerInterface::iterator container_iterator );
	
private:           
    explicit Mutator( Mode mode_,
                      TreePtr<Node> parent_node, 
					  TreePtrInterface *parent_singular_,
                      ContainerInterface *parent_container_,
                      ContainerInterface::iterator container_iterator_ );             

public:    
    TreePtr<Node> ExchangeChild( TreePtr<Node> new_child,                               
                                 list<shared_ptr<Mutator>> child_terminii = {} );
    void ExchangeParent( Mutator& other_mut );
    TreePtr<Node> GetParentNode() const;

    bool IsAtRoot() const;
    
    // After population, a mutator can now provide an XLink that's valid for the new boundary
    XLink GetXLink() const; // Only valid after populate
    const TreePtrInterface *GetTreePtrInterface() const; // Only valid after populate
    
    static TreePtr<Node> MakePlaceholder();    

    static shared_ptr<Mutator> FindMatchingTerminus( ContainerInterface *container,
                                                     ContainerInterface::iterator it_placeholder,
                                                     list<shared_ptr<Mutator>> &candidate_terminii );
    
    void Validate() const;

    string GetTrace() const;

private:
	Mode mode;
    TreePtr<Node> parent_node;
    TreePtrInterface *parent_singular;    // TODO could combine into an Itemiser::Element *
    ContainerInterface *parent_container;
    ContainerInterface::iterator container_iterator;
};        
    

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
	  
    
}

#endif
