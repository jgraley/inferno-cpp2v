#ifndef AGENT_COMMON_HPP
#define AGENT_COMMON_HPP

#include "agent.hpp"

#include "common/common.hpp"
#include "../conjecture.hpp"
#include "../query.hpp"
#include "../sym/lazy_eval.hpp"
#include "../up/patches.hpp"

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
                                  
    SYM::Lazy<SYM::BooleanExpression> SymbolicQuery( bool coupling_only ) const override;                                       
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQuery() const = 0;
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicCouplingQuery() const;       
    bool IsNonTrivialPreRestrictionNP(const TreePtrInterface *pptr) const override;
    bool IsNonTrivialPreRestriction() const;
    bool ShouldGenerateCategoryClause() const override;                                
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicPreRestriction() const;
    bool IsPreRestrictionMatch( TreePtr<Node> x ) const; // return true if matches
    bool IsPreRestrictionMatch( XLink x ) const; // return true if matches
    
    virtual void RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                           const SolutionMap *hypothesis_links,
                                           const XTreeDatabase *x_tree_db ) const;
    void RunRegenerationQuery( DecidedQueryAgentInterface &query,
                               const SolutionMap *hypothesis_links,
                               const XTreeDatabase *x_tree_db ) const;
    virtual QueryLambda StartRegenerationQuery( const SolutionMap *hypothesis_links,
                                                const XTreeDatabase *x_tree_db,
                                                bool use_DQ = false ) const;
    virtual QueryLambda TestStartRegenerationQuery( const SolutionMap *hypothesis_links,
                                                    const XTreeDatabase *x_tree_db ) const;
    
    virtual void ResetNLQConjecture();
     
    virtual const SCREngine *GetMasterSCREngine() const;      
    virtual PatternLink GetKeyerPatternLink() const override;                                  
    virtual set<PatternLink> GetResidualPatternLinks() const override;                                  
    virtual void Reset();    
    virtual void PlanOverlay( PatternLink me_plink, 
                              PatternLink under_plink );
    virtual void MaybeChildrenPlanOverlay( PatternLink me_plink, 
                                  PatternLink under_plink );
    virtual bool ReplaceKeyerQuery( PatternLink me_plink, 
                                    set<PatternLink> keyer_plinks );                                  
    TreePtr<Node> BuildForBuildersAnalysis( PatternLink me_plink ) override;
    ReplacePatchPtr GenReplaceLayout( const ReplaceKit &kit, 
                                PatternLink me_plink ) override;
    void SetAssign( TreePtr<Node> embedded_through_subtree ) const override;
    TreePtr<Node> GetEmbeddedSearchPattern() const override;
    TreePtr<Node> GetEmbeddedReplacePattern() const override;
    
    virtual ReplacePatchPtr GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                         PatternLink me_plink, 
                                         XLink key_xlink );
    TreePtr<Node> CloneNode() const;
    virtual string GetTrace() const;

protected:                                  
    const SCREngine *my_scr_engine = nullptr;    
    const Traceable *my_keyer_engine = nullptr;    
    shared_ptr<PatternQuery> pattern_query;
    PatternLink keyer_plink;
    set<PatternLink> residual_plinks;
    PatternLink overlay_under_plink;
    list<PatternLink> keyer_and_normal_plinks;
    Phase phase = UNDEFINED;
    
private:    
    shared_ptr<Conjecture> nlq_conjecture;

    virtual string GetGraphId() const;
    virtual string GetPlanAsString() const override;
};

};

#endif
