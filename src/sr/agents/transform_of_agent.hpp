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
	class TransUtils;

	// ---------------------- AugBE ---------------------------	
	class AugBE : public AugBEInterface, 
	              public Traceable
	{
	public:	
		AugBE( TreePtr<Node> generic_tree_ptr_, const TransUtils *utils_ );
		AugBE( XLink xlink_, const TransUtils *utils_ );
		AugBE( const AugBE &other, TreePtr<Node> generic_tree_ptr_ );
		AugBE( const AugBE &other, XLink xlink_ );
		AugBE( const AugBE &other ) = default;	
		AugBE &operator=(const AugBE &other) = default;
		AugBE *Clone() const override;

		TreePtr<Node> GetGenericTreePtr() const;
		XLink GetXLink() const;	
		const Dependencies &GetDeps() const;

		AugBE *OnGetChild( const TreePtrInterface *other_tree_ptr ) override;
		void OnSetChild( const TreePtrInterface *other_tree_ptr, AugBEInterface *new_val ) override;
		void OnDepLeak() override;

		string GetTrace() const;

	private:
		const TransUtils * const utils;	
		XLink xlink;
		TreePtr<Node> generic_tree_ptr;
		const shared_ptr<Dependencies> my_deps;			
	};

	// ---------------------- TransUtils ---------------------------
	class TransUtils : public TransUtilsInterface
	{
	public:	
		explicit TransUtils( const XTreeDatabase *db_, 
							 Dependencies *deps_ );

		// Create AugTreePtr from a link
		AugTreePtr<Node> CreateAugTreePtr(XLink xlink) const;

		ValuePtr<AugBEInterface> CreateBE( TreePtr<Node> tp ) const override;
			
		// Getters for AugTreePtr - back end only
		ValuePtr<TransformOfAgent::AugBE> GetBE( const AugTreePtrBase &atp ) const;
		
		set<AugTreePtr<Node>> GetDeclarers( AugTreePtr<Node> node ) const override;
		
		Dependencies *GetDeps() const;
			
	public: // TODO private:	
		const XTreeDatabase * const db;
		Dependencies * const deps;	
	};

	// ---------------------- TransformOfAgent ---------------------------

    TransformOfAgent( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) :
    	transformation(t),
    	pattern(p)
    {
    }

    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    QueryReturnType RunTeleportQuery( const XTreeDatabase *db, Dependencies *deps, XLink stimulus_xlink ) const override;                

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
