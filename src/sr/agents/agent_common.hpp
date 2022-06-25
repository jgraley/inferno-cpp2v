#ifndef AGENT_COMMON_HPP
#define AGENT_COMMON_HPP

#include "agent.hpp"

#include "common/common.hpp"
#include "../conjecture.hpp"
#include "../query.hpp"
#include "../sc_relation.hpp"
#include "../sym/overloads.hpp"

namespace SR
{ 
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
                                  
    SYM::Over<SYM::BooleanExpression> SymbolicQuery( bool coupling_only ) const override;                                       
    virtual SYM::Over<SYM::BooleanExpression> SymbolicNormalLinkedQuery() const = 0;
    virtual SYM::Over<SYM::BooleanExpression> SymbolicCouplingQuery() const;       
    bool IsNonTrivialPreRestrictionNP(const TreePtrInterface *pptr) const override;
    bool IsNonTrivialPreRestriction() const;
    bool ShouldGenerateCategoryClause() const override;                                
	virtual SYM::Over<SYM::BooleanExpression> SymbolicPreRestriction() const;
    bool IsPreRestrictionMatch( TreePtr<Node> x ) const; // return true if matches
    bool IsPreRestrictionMatch( XLink x ) const; // return true if matches
	
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

};

#endif
