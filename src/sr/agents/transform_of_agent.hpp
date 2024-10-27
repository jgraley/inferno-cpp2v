#ifndef TRANSFORM_OF_AGENT_HPP
#define TRANSFORM_OF_AGENT_HPP

#include "../search_replace.hpp"
#include "helpers/transformation.hpp"
#include "teleport_agent.hpp"
#include "delta_agent.hpp"
#include "embedded_scr_agent.hpp"
#include "special_agent.hpp"

namespace SR
{
	
// ---------------------- TransformOfAugBE ---------------------------

class TransformOfAugBE : public AugBEInterface
{
public:	
	explicit TransformOfAugBE();
	explicit TransformOfAugBE( TreePtr<Node> generic_tree_ptr_ );
    explicit TransformOfAugBE( const TreePtrInterface *p_tree_ptr_, TeleportAgent::DependencyReporter *dep_rep_ );
	TransformOfAugBE( const TransformOfAugBE &other ) = default;	
	TransformOfAugBE &operator=(const TransformOfAugBE &other) = default;
	TransformOfAugBE *Clone() const override;

	TreePtr<Node> GetGenericTreePtr() const;
	const TreePtrInterface *GetPTreePtr() const;	
    TransformOfAugBE *OnGetChild( const TreePtrInterface *other_tree_ptr ) override;
    void OnSetChild( const TreePtrInterface *other_tree_ptr, AugBEInterface *new_val ) override;
    void OnDepLeak() override;

private:
    TreePtr<Node> generic_tree_ptr;
    const TreePtrInterface *p_tree_ptr;
    TeleportAgent::DependencyReporter *dep_rep;	
};

// ---------------------- TransformOfUtils ---------------------------

class TransformOfUtils : public TransUtilsInterface
{
public:	
	explicit TransformOfUtils( const NavigationInterface *nav_, 
	                           TeleportAgent::DependencyReporter *dep_rep_ );

	// Create AugTreePtr from a link
    AugTreePtr<Node> CreateAugTreePtr(const TreePtrInterface *p_tree_ptr) const;

    ValuePtr<AugBEInterface> CreateBE( TreePtr<Node> tp ) const override;
		
	// Getters for AugTreePtr - back end only
    const TreePtrInterface *GetPTreePtr( const AugTreePtrBase &atp ) const;	
    TreePtr<Node> GetGenericTreePtr( const AugTreePtrBase &atp ) const;
	
	set<AugTreePtr<Node>> GetDeclarers( AugTreePtr<Node> node ) const override;
	
	TeleportAgent::DependencyReporter *GetDepRep() const;
		
private:	
	const NavigationInterface * const nav;
	TeleportAgent::DependencyReporter *dep_rep;	
};


// ---------------------- TransformOfAgent ---------------------------
	
/// Matches the output of `transformation` when applied to the current tree node
/// against the sub-pattern at `pattern`. This permits an arbitrary `Transformation`
/// implementation to be "injected" into the search and replace operation.
/// The transformation should be invarient during any single search operation
/// but can change when replace acts to change the tree. Thus it can depend on
/// the tree.
class TransformOfAgent : public virtual TeleportAgent
{
public:
    class TransformationFailedMismatch : public Mismatch {};

    TransformOfAgent( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) :
    	transformation(t),
    	pattern(p)
    {
    }

    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    QueryReturnType RunTeleportQuery( const XTreeDatabase *db, DependencyReporter *dep_rep, XLink stimulus_xlink ) const override;                

    virtual Block GetGraphBlockInfo() const;
    string GetName() const override;
    
 	bool IsExtenderChannelLess( const Extender &r ) const override;
	int GetExtenderChannelOrdinal() const override;

    TreePtr<Node> pattern; 
    Transformation *transformation;

protected: 
    TransformOfAgent() {}    
};

// ---------------------- TransformOf<> ---------------------------

/// Match the output of some transformation against the child pattern 
template<class PRE_RESTRICTION>
class TransformOf : public TransformOfAgent,
                    public Special<PRE_RESTRICTION>
{
public:
    SPECIAL_NODE_FUNCTIONS	

    shared_ptr<const Node> GetPatternPtr() const
    {
        return shared_from_this();
    }
    
    TransformOf() {}    
    TransformOf( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) : 
        TransformOfAgent(t, p) 
    {
    }
};

};
#endif
