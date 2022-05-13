#ifndef AGENT_COMMON_HPP
#define AGENT_COMMON_HPP

#include "agent.hpp"

#include "common/common.hpp"
#include "../conjecture.hpp"
#include "../query.hpp"
#include "../equivalence.hpp"
#include "../sym/overloads.hpp"

namespace SR
{ 
// - Configure
// - Pre-restriction
// - Keying (default case)
// - Compare and MatchingDecidedCompare rings
/// Implments misc functionality common to all or most agents
class AgentCommon : public Agent
{
public:
    AgentCommon();
    virtual void SCRConfigure( const SCREngine *e,
                               Phase phase );
    virtual void ConfigureCoupling( const Traceable *e,
                                    PatternLink keyer_plink_, 
                                    set<PatternLink> residual_plinks_ );
    virtual void AddResiduals( set<PatternLink> residual_plinks_ );
    virtual list<PatternLink> GetChildren() const override;
    virtual list<PatternLink> GetVisibleChildren( Path v ) const override;
    virtual shared_ptr<DecidedQuery> CreateDecidedQuery() const;                                    
                                  
    virtual SYM::Over<SYM::BooleanExpression> SymbolicQuery( bool coupling_only ) const override;                                       
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQueryImpl() const;                                       
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQuery() const;                                       
    virtual SYM::Over<SYM::BooleanExpression> SymbolicCouplingQuery() const;                                       
	virtual SYM::Over<SYM::BooleanExpression> SymbolicPreRestriction() const;
	
    virtual void RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                           const SolutionMap *hypothesis_links,
                                           const TheKnowledge *knowledge ) const;
    void RunRegenerationQuery( DecidedQueryAgentInterface &query,
                               const SolutionMap *hypothesis_links,
                               const TheKnowledge *knowledge ) const;
    virtual QueryLambda StartRegenerationQuery( const SolutionMap *hypothesis_links,
                                                const TheKnowledge *knowledge,
                                                bool use_DQ = false ) const;
    virtual QueryLambda TestStartRegenerationQuery( const SolutionMap *hypothesis_links,
                                                    const TheKnowledge *knowledge ) const;
    virtual set<XLink> ExpandNormalDomain( const unordered_set<XLink> &keyer_xlinks ) { return {}; }
    virtual void ResetNLQConjecture();
     
public:
    virtual const SCREngine *GetMasterSCREngine() const;      
    virtual PatternLink GetKeyerPatternLink() const override;                                  
    virtual set<PatternLink> GetResidualPatternLinks() const override;                                  
    virtual void Reset();    
    virtual void PlanOverlay( PatternLink me_plink, 
                              PatternLink under_plink );
    virtual void PlanOverlayImpl( PatternLink me_plink, 
                                  PatternLink under_plink );
    virtual bool ReplaceKeyerQuery( PatternLink me_plink, 
                                    set<PatternLink> keyer_plinks );                                  
    virtual TreePtr<Node> BuildReplace( PatternLink me_plink );
    virtual TreePtr<Node> BuildReplaceImpl( PatternLink me_plink, 
                                            TreePtr<Node> key_node );
    TreePtr<Node> DuplicateNode( TreePtr<Node> pattern,
                                 bool force_dirty ) const;
    TreePtr<Node> DuplicateSubtree( TreePtr<Node> source,
                                    TreePtr<Node> source_terminus = TreePtr<Node>(),
                                    TreePtr<Node> dest_terminus = TreePtr<Node>() ) const;
    virtual bool IsPreRestrictionMatch( XLink x ) const override; // return true if matches
    virtual string GetTrace() const;

protected:                                  
    const SCREngine *master_scr_engine = nullptr;    
    const Traceable *coupling_master_engine = nullptr;    
    shared_ptr<PatternQuery> pattern_query;
    PatternLink keyer_plink;
    set<PatternLink> residual_plinks;
    PatternLink overlay_under_plink;
    list<PatternLink> keyer_and_normal_plinks;
    Phase phase = UNDEFINED;
    
private:    
    int num_decisions;
    shared_ptr<Conjecture> nlq_conjecture;

    virtual string GetGraphId() const;
    virtual string GetPlanAsString() const override;
};


// --- General note on SPECIAL_NODE_FUNCTIONS and PRE_RESTRICTION ---
// Special nodes (that is nodes defined here with special S&R behaviour)
// derive from a standard tree node via templating. This base class is
// the PRE_RESTRICTION node, and we want it for 2 reasons:
// 1. To allow standard nodes to point to special nodes, they must
//    expose a standard interface, which can vary depending on usage
//    so must be templated.
// 2. We are able to provide a "free" and-rule restriction on all
//    special nodes by restricting to non-strict subclasses of the
//    pre-restrictor.
// In order to make 2. work, we need to *avoid* overriding IsLocalMatch()
// or IsSubclass() on special nodes, so that the behaviour of the 
// PRE_RESTRICTION is preserved wrt comparisons. So all special nodes
// including speicialisations of TransformTo etc should use 
// SPECIAL_NODE_FUNCTIONS instead of NODE_FUNCTIONS.
// Itemise is known required (for eg graph plotting), other bounces
// are TBD.

#define ARCHETYPE_FUNCTION \
    virtual TreePtr<Node> GetArchetypeNode() const override \
    { \
        return this->SpecialGetArchetypeNode(); \
    }


#define SPECIAL_NODE_FUNCTIONS ITEMISE_FUNCTION ARCHETYPE_FUNCTION
/// Common stuff for pattern nodes other than standard nodes
class SpecialBase
{
public:    
    virtual shared_ptr< TreePtrInterface > GetArchetypeTreePtr() const = 0;
    virtual TreePtr<Node> SpecialGetArchetypeNode() const = 0;
	static bool IsNonTrivialPreRestriction(const TreePtrInterface *ptr);
};


/// Common stuff for pattern nodes other than standard nodes
template<class PRE_RESTRICTION>
class Special : public SpecialBase, 
                public virtual PRE_RESTRICTION
{
public:
    virtual shared_ptr< TreePtrInterface > GetArchetypeTreePtr() const override
    {
        return make_shared<TreePtr<PRE_RESTRICTION>>();  
    }
    virtual TreePtr<Node> SpecialGetArchetypeNode() const override
    {
        return TreePtr<Node>( new PRE_RESTRICTION );  
    }
};

};

#endif
