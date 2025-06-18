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
		Null,
		Root,
		Singular,
		Container		
	};
	
public:	
	static Mutator CreateFreeSingular( TreePtr<Node> parent_node, 
	                                   TreePtrInterface *parent_singular );
	static Mutator CreateFreeContainer( TreePtr<Node> parent_node, 
                                        ContainerInterface *parent_container,
                                        ContainerInterface::iterator container_iterator );
                                             
private: friend class XTreeDatabase;
	static Mutator CreateTreeRoot( shared_ptr<TreePtr<Node>> sp_tp_root_node );
	static Mutator CreateTreeSingular( TreePtr<Node> parent_node, 
	                                   TreePtrInterface *parent_singular );
	static Mutator CreateTreeContainer( TreePtr<Node> parent_node, 
                                        ContainerInterface *parent_container,
                                        ContainerInterface::iterator container_iterator );
                                             
private:
    explicit Mutator( Mode mode_,
                      TreePtr<Node> parent_node, 
					  TreePtrInterface *parent_singular_,
                      ContainerInterface *parent_container_,
                      ContainerInterface::iterator container_iterator_,
                      shared_ptr<TreePtr<Node>> sp_tp_root_node_ );             

public:  
    explicit Mutator();
    	
    bool operator<(const Mutator &right) const;
	bool operator==( const Mutator &right ) const;
	bool operator!=( const Mutator &right ) const;      
	operator bool() const;      
  
    TreePtr<Node> ExchangeChild( TreePtr<Node> free_child ) const;
    TreePtr<Node> ExchangeContainer( ContainerInterface *child_container,                               
                                     list<Mutator> &child_terminii );
    static void SwapParents( Mutator& mut_l, Mutator& mut_r );
    void SetParent( const Mutator &free_mut );

    TreePtr<Node> GetParentNode() const;
    bool IsAtRoot() const;        
    XLink GetXLink() const; 
    const TreePtrInterface *GetTreePtrInterface() const;
    TreePtr<Node> GetChildTreePtr() const;
    static TreePtr<Node> MakePlaceholder();    

    static Mutator *FindMatchingTerminus( ContainerInterface *container,
                                          ContainerInterface::iterator it_placeholder,
                                          list<Mutator> &candidate_terminii );
        
    void Validate() const;
    string GetTrace() const;

private:
	Mode mode;
    TreePtr<Node> parent_node;
    TreePtrInterface *parent_singular;    // TODO could combine into an Itemiser::Element *
    ContainerInterface *parent_container;
    ContainerInterface::iterator container_iterator;
    shared_ptr<TreePtr<Node>> sp_tp_root_node;
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
