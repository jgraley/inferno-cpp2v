#ifndef TRANSFORM_OF_AGENT_HPP
#define TRANSFORM_OF_AGENT_HPP

#include "../search_replace.hpp"
#include "helpers/transformation.hpp"
#include "relocating_agent.hpp"
#include "delta_agent.hpp"
#include "embedded_scr_agent.hpp"
#include "special_agent.hpp"

namespace VN 
{
    
// ---------------------- TransformOfAgent ---------------------------
    
/// Matches the output of `transformation` when applied to the current tree node
/// against the sub-pattern at `pattern`. This permits an arbitrary `Transformation`
/// implementation to be "injected" into the search and replace operation.
/// The transformation should be invarient during any single search operation
/// but can change when replace acts to change the tree. Thus it can depend on
/// the tree.
class TransformOfAgent : public virtual RelocatingAgent
{
public:    
    class TransUtils;
    class AugBEMeandering;
    class ReachedNullChiled : public Mismatch {}; // Eg children of scaffold nodes

    // ---------------------- AugBECommon ---------------------------    
    class AugBECommon : public AugBEInterface, 
                        public Traceable
    {
    public:
        AugBECommon( const TransUtils *utils_ );
        AugBECommon( const AugBECommon &other ) = default;
        AugBECommon &operator=(const AugBECommon &other) = default;

        const TransUtils *GetUtils() const;
        Dependencies &GetDeps();
        shared_ptr<Dependencies> GetDepsPtr();
        const Dependencies &GetDeps() const;
        void OnDepLeak() override;

        string GetTrace() const;

    private:
        const TransUtils * const utils;    
        const shared_ptr<Dependencies> my_deps;            
    };
    
    // ---------------------- AugBERoaming ---------------------------    
    /**
     * Roaming back-end for AugTreePtr: this refers the an XLink inside
     * the X tree, db etc. Roaming refers to moving around within the X 
     * tree, possibly with the help of the database navigation features
     * like GetDeclarers().
     */ 
    class AugBERoaming : public AugBECommon
    {
    public:    
        AugBERoaming( XLink xlink_, const TransUtils *utils_ );
        AugBERoaming( const AugBECommon &other, XLink xlink_ );
        AugBERoaming( const AugBERoaming &other ) = default;    
        AugBERoaming &operator=(const AugBERoaming &other) = default;
        AugBERoaming *Clone() const final;

        XLink GetXLink() const;    

        AugBERoaming *OnGetChild( const TreePtrInterface *other_tree_ptr ) override;
        void OnSetChild( const TreePtrInterface *other_tree_ptr, AugBEInterface *new_val ) override;

        string GetTrace() const;

    private:
        const XLink xlink;
    };
    
    // ---------------------- AugBEMeandering ---------------------------    
    /**
     * Meandering back-end for AugTreePtr: this refers to a TreePtr not
     * inside the X tree. Meandering refers to the fact that we're moving
     * around in a space outside the X tree and could move into the X tree,
     * at which point we would begin roaming. Our implemntation keeps 
     * track of the current TreePtr, not XLink, because it's not in the 
     * x tree and hence db etc.
     */
    class AugBEMeandering : public AugBECommon
    {
    public:    
        AugBEMeandering( TreePtr<Node> generic_tree_ptr_, const TransUtils *utils_ );
        AugBEMeandering( const AugBECommon &other, TreePtr<Node> generic_tree_ptr_ );
        AugBEMeandering( const AugBEMeandering &other ) = default;    
        AugBEMeandering &operator=(const AugBEMeandering &other) = default;
        AugBEMeandering *Clone() const final;

        TreePtr<Node> GetGenericTreePtr() const;

        AugBEInterface *OnGetChild( const TreePtrInterface *other_tree_ptr ) override;
        void OnSetChild( const TreePtrInterface *other_tree_ptr, AugBEInterface *new_val ) override;

        string GetTrace() const;

    private:
        const TreePtr<Node> generic_tree_ptr;
    };

    // ---------------------- TransUtils ---------------------------
    class TransUtils : public TransUtilsInterface
    {
    public:    
        explicit TransUtils( const XTreeDatabase *db_, 
                             Dependencies *deps_ );

        // Create AugTreePtr from a link
        AugTreePtr<Node> CreateAugTreePtrRoaming(XLink xlink) const;

        ValuePtr<AugBEInterface> CreateBE( TreePtr<Node> tp ) const override;
            
        // Getters for AugTreePtr - back end only
        ValuePtr<TransformOfAgent::AugBECommon> GetBE( const AugTreePtrBase &atp ) const;

        
        set<AugTreePtr<Node>> GetDeclarers( AugTreePtr<Node> node ) const override;
        
        Dependencies *GetDeps() const;
            
    public: // TODO private:    
        const XTreeDatabase * const db;
        Dependencies * const deps;    
    };

    // ---------------------- TransformOfAgent ---------------------------

    TransformOfAgent( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) :
        pattern(p),
        transformation(t)
    {
    }

    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    RelocatingQueryResult RunRelocatingQuery( const XTreeDatabase *db, XLink stimulus_xlink ) const override;                
    bool IsExtenderChannelLess( const Extender &r ) const override;
    int GetExtenderChannelOrdinal() const override;

	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const final;
    string GetCouplingNameHint() const final;
    NodeBlock GetGraphBlockInfo() const final;
    string GetName() const override;
    string GetTrace() const override;
    
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
