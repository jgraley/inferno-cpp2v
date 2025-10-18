#ifndef AGENT_COMMON_HPP
#define AGENT_COMMON_HPP

#include "agent.hpp"

#include "common/common.hpp"
#include "../conjecture.hpp"
#include "../query.hpp"
#include "../sym/lazy_eval.hpp"
#include "../up/patches.hpp"

namespace VN
{ 
/// Implments misc functionality common to all or most agents
class AgentCommon : public Agent
{
public:
    AgentCommon();
    virtual void SCRConfigure( Phase phase );
    virtual list<PatternLink> GetChildren() const override;
    virtual list<PatternLink> GetVisibleChildren( Path v ) const override;
    virtual shared_ptr<DecidedQuery> CreateDecidedQuery() const;                                    
                                  
    SYM::Lazy<SYM::BooleanExpression> SymbolicQuery( PatternLink keyer, const set<PatternLink> &residuals, bool coupling_only ) const override;                                       
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicNormalLinkedQuery(PatternLink keyer_plink) const = 0;
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicCouplingQuery(PatternLink keyer, const set<PatternLink> &residuals) const;       
    bool IsNonTrivialPreRestriction(const TreePtrInterface *pptr) const override;
    bool ShouldGenerateCategoryClause() const override;                                
	Syntax::Production GetAgentProduction() const override;
	string GetRender( const RenderKit &kit, Syntax::Production surround_prod ) const override;
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicPreRestriction(PatternLink keyer_plink) const;
    bool IsPreRestrictionMatch( TreePtr<Node> x ) const; // return true if matches
    bool IsPreRestrictionMatch( XLink x ) const; // return true if matches
    
    virtual void RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                           const SolutionMap *hypothesis_links,
                                           PatternLink keyer_plink,
                                           const XTreeDatabase *x_tree_db ) const;
    void RunRegenerationQuery( DecidedQueryAgentInterface &query,
                               const SolutionMap *hypothesis_links,
                               PatternLink keyer_plink,
                               const XTreeDatabase *x_tree_db ) const;
    QueryLambda StartRegenerationQuery( const AndRuleEngine *acting_engine,
                                        const SolutionMap *hypothesis_links,
                                        PatternLink keyer_plink,
                                        const XTreeDatabase *x_tree_db ) const final;
    QueryLambda TestStartRegenerationQuery( const AndRuleEngine *acting_engine,
                                            const SolutionMap *hypothesis_links,
										    PatternLink keyer_plink,
                                            const XTreeDatabase *x_tree_db ) const final;
         
    virtual void Reset();    
    virtual void PlanOverlay( SCREngine *acting_engine,
                              PatternLink me_plink, 
                              PatternLink bottom_layer_plink );
    virtual void MaybeChildrenPlanOverlay( SCREngine *acting_engine,
                                           PatternLink me_plink, 
                                           PatternLink bottom_layer_plink );                                 
    TreePtr<Node> BuildForBuildersAnalysis( PatternLink me_plink,
                                            const SCREngine *acting_engine ) override;
    ReplacePatchPtr GenReplaceLayout( const ReplaceKit &kit, 
                                PatternLink me_plink,
                                const SCREngine *acting_engine ) override;
    TreePtr<Node> GetEmbeddedSearchPattern() const override;
    TreePtr<Node> GetEmbeddedReplacePattern() const override;
    
    virtual ReplacePatchPtr GenReplaceLayoutImpl( const ReplaceKit &kit, 
                                                  PatternLink me_plink, 
                                                  XLink key_xlink,
                                                  const SCREngine *acting_engine );
    TreePtr<Node> CloneNode() const;
    virtual string GetTrace() const;

    virtual string GetGraphId() const;
    virtual string GetPlanAsString() const override;
    
protected:                                  
    Phase phase = UNDEFINED;    
};

};

#endif
