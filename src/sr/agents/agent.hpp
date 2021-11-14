#ifndef AGENT_HPP
#define AGENT_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "../conjecture.hpp"
#include "../boolean_evaluator.hpp"
#include "../query.hpp"
#include "common/mismatch.hpp"
#include "../equivalence.hpp"
#include "../cache.hpp"

#include <vector>
#include <boost/type_traits.hpp>
#include <functional>
#include "node/graphable.hpp"

#define HINTS_IN_EXCEPTIONS

namespace SR
{ 
class SpecialBase;
class SCREngine;
class AndRuleEngine;
class Agent;
class TheKnowledge;

/// Interface for Agents, which co-exist with pattern nodes and implement the search and replace funcitonality for each pattern node.
class Agent : public virtual Graphable,
              public virtual Node
{
public:  
    typedef Graphable::Phase Phase;
    
    enum Path
    {
        COMPARE_PATH,
        REPLACE_PATH
    };
    
    // Any mismatch this class throws
    class Mismatch : public ::Mismatch 
    {
#ifdef HINTS_IN_EXCEPTIONS        
    public:
        LocatedLink hint;
        virtual string What() const noexcept { return ::Mismatch::What() + (hint ? string(",hint:") + Trace(hint) : string(",no-hint")); }
#endif        
    };
    
    class NLQMismatch : public Mismatch {};
    class NLQConjOutAfterHitMismatch : public NLQMismatch {};
    class NLQConjOutAfterMissMismatch : public NLQMismatch {};
    class NLQFromDQLinkMismatch : public NLQMismatch {};
    
    class CouplingMismatch : public Mismatch {};
        
    Agent& operator=(Agent& other);
	virtual void SCRConfigure( const SCREngine *e,
                               Phase phase ) = 0;
    virtual void ConfigureCoupling( const Traceable *e,
                                    PatternLink base_plink_, 
                                    set<PatternLink> coupled_plinks_ ) = 0;
    virtual void AddResiduals( set<PatternLink> coupled_plinks_ ) = 0;
                                   
    /// List the Agents reached via links during search
    virtual shared_ptr<PatternQuery> GetPatternQuery() const = 0;
    virtual shared_ptr<DecidedQuery> CreateDecidedQuery() const = 0;
    
    /// Produce info about an Agent given location (x) and a vector of choices (conj). 
    virtual void RunDecidedQuery( DecidedQueryAgentInterface &query,
                                  XLink base_xlink ) const = 0;     
    
    typedef function<shared_ptr<DecidedQuery>()> QueryLambda;

    /// Test an Agent given partial map of locations of base and normal links. 
    virtual bool ImplHasNLQ() const = 0;                                              
    virtual bool NLQRequiresBase() const = 0;                                              
    virtual void RunNormalLinkedQuery( const SolutionMap *required_links,
                                       const TheKnowledge *knowledge ) const = 0;

    /// Test an agent given given partial map of locations of keyer and residuals.  
    virtual void RunCouplingQuery( const SolutionMap *required_links ) = 0;                                       

    /// Get abnormal/multiplicity info from an Agent given partial map of locations of base and normal links. 
    virtual QueryLambda StartRegenerationQuery( const SolutionMap *required_links,
                                                const TheKnowledge *knowledge,
                                                bool use_DQ = false ) const = 0;
    virtual QueryLambda TestStartRegenerationQuery( const SolutionMap *required_links,
                                                    const TheKnowledge *knowledge ) const = 0;
    virtual set<XLink> ExpandNormalDomain( const unordered_set<XLink> &base_xlinks ) = 0;
    virtual void ResetNLQConjecture() = 0;    

    virtual const SCREngine *GetMasterSCREngine() const = 0;      
    virtual PatternLink GetKeyerPatternLink() const = 0;

    virtual void Reset() = 0;     
    virtual void PlanOverlay( PatternLink me_plink, 
                              PatternLink under_plink ) = 0;
    virtual bool ReplaceKeyerQuery( PatternLink me_plink, 
                                    unordered_set<PatternLink> keyer_plinks ) = 0;

    virtual TreePtr<Node> BuildReplace( PatternLink me_plink ) = 0;
    virtual TreePtr<Node> DuplicateSubtree( TreePtr<Node> source,
                                            TreePtr<Node> source_terminus = TreePtr<Node>(),
                                            TreePtr<Node> dest_terminus = TreePtr<Node>() ) const = 0;
    virtual list<PatternLink> GetChildren() const = 0;
    virtual list<PatternLink> GetVisibleChildren( Path v ) const = 0;
		
	static Agent *AsAgent( shared_ptr<Node> node );
	static Agent *TryAsAgent( shared_ptr<Node> node );
	static const Agent *AsAgentConst( shared_ptr<const Node> node );
	static const Agent *TryAsAgentConst( shared_ptr<const Node> node );
    // Note the constness: thou shalt not try to mutate the agent via this function
    virtual shared_ptr<const Node> GetPatternPtr() const = 0;
};


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
                                    PatternLink base_plink_, 
                                    set<PatternLink> coupled_plinks_ );
    virtual void AddResiduals( set<PatternLink> coupled_plinks_ );
    virtual list<PatternLink> GetChildren() const override;
    virtual list<PatternLink> GetVisibleChildren( Path v ) const override;
    virtual shared_ptr<DecidedQuery> CreateDecidedQuery() const;                                    
    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                      XLink base_xlink ) const;                                                
    virtual void RunDecidedQuery( DecidedQueryAgentInterface &query,
                                  XLink base_xlink ) const;       
                                                                           
    virtual bool ImplHasNLQ() const;
    virtual bool NLQRequiresBase() const;
    void NLQFromDQ( const SolutionMap *required_links,
                    const TheKnowledge *knowledge ) const;                                              
    virtual void RunNormalLinkedQueryImpl( const SolutionMap *required_links,
                                           const TheKnowledge *knowledge ) const;
    virtual void RunNormalLinkedQuery( const SolutionMap *required_links,
                                       const TheKnowledge *knowledge ) const;
                                       
    virtual void RunCouplingQuery( const SolutionMap *required_links );                                       
    
    virtual void RunRegenerationQueryImpl( DecidedQueryAgentInterface &query,
                                           const SolutionMap *required_links,
                                           const TheKnowledge *knowledge ) const;
    void RunRegenerationQuery( DecidedQueryAgentInterface &query,
                               const SolutionMap *required_links,
                               const TheKnowledge *knowledge ) const;
    virtual QueryLambda StartRegenerationQuery( const SolutionMap *required_links,
                                                const TheKnowledge *knowledge,
                                                bool use_DQ = false ) const;
    virtual QueryLambda TestStartRegenerationQuery( const SolutionMap *required_links,
                                                    const TheKnowledge *knowledge ) const;
    virtual set<XLink> ExpandNormalDomain( const unordered_set<XLink> &base_xlinks ) { return {}; }
    virtual void ResetNLQConjecture();
     
public:
    virtual const SCREngine *GetMasterSCREngine() const;      
    virtual PatternLink GetKeyerPatternLink() const;                                  
    virtual void Reset();    
    virtual void PlanOverlay( PatternLink me_plink, 
                              PatternLink under_plink );
    virtual void PlanOverlayImpl( PatternLink me_plink, 
                                  PatternLink under_plink );
    virtual bool ReplaceKeyerQuery( PatternLink me_plink, 
                                    unordered_set<PatternLink> keyer_plinks );                                  
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
    PatternLink base_plink;
    PatternLink overlay_under_plink;
    set<PatternLink> coupled_plinks;
    list<PatternLink> base_and_normal_plinks;
    Phase phase = UNDEFINED;
    
private:    
    int num_decisions;
    EquivalenceRelation equivalence_relation;
    shared_ptr<Conjecture> nlq_conjecture;

    virtual string GetGraphId() const;
};


class DefaultMMAXAgent : public AgentCommon
{
public:    
    class MMAXPropagationMismatch : public Mismatch {};

    virtual void RunDecidedQueryImpl( DecidedQueryAgentInterface &query,
                                      XLink base_xlink ) const;                                      
    virtual void RunDecidedQueryMMed( DecidedQueryAgentInterface &query,
                                      XLink base_xlink ) const = 0;
                                                                                      
    virtual void RunNormalLinkedQueryImpl( const SolutionMap *required_links,
                                           const TheKnowledge *knowledge ) const;                                             
    virtual void RunNormalLinkedQueryMMed( const SolutionMap *required_links,
                                           const TheKnowledge *knowledge ) const;
};


class PreRestrictedAgent : public DefaultMMAXAgent
{
public:    
    class PreRestrictionMismatch : public Mismatch {};

    virtual void RunDecidedQueryMMed( DecidedQueryAgentInterface &query,
                                      XLink base_xlink ) const;                                      
    virtual void RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                      XLink base_xlink ) const = 0;                                          
                                      
    virtual void RunNormalLinkedQueryMMed( const SolutionMap *required_links,
                                           const TheKnowledge *knowledge ) const;
    virtual void RunNormalLinkedQueryPRed( const SolutionMap *required_links,
                                           const TheKnowledge *knowledge ) const;                                      
};


class TeleportAgent : public PreRestrictedAgent
{
public:    
    virtual void RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                      XLink base_xlink ) const;                  
    virtual map<PatternLink, XLink> RunTeleportQuery( XLink base_xlink ) const { ASSERTFAIL(); }
    
    virtual set<XLink> ExpandNormalDomain( const unordered_set<XLink> &base_xlinks );

    virtual void Reset();    

private:
    mutable CacheByLocation cache;    
};


class SearchLeafAgent : public PreRestrictedAgent
{
    virtual shared_ptr<PatternQuery> GetPatternQuery() const;
    virtual void RunDecidedQueryPRed( DecidedQueryAgentInterface &query,
                                      XLink base_xlink ) const;                  
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
#define SPECIAL_NODE_FUNCTIONS ITEMISE_FUNCTION  
/// Common stuff for pattern nodes other than standard nodes
class SpecialBase
{
public:    
    virtual shared_ptr< TreePtrInterface > GetPreRestrictionArchitype() const = 0;
	static bool IsNonTrivialPreRestriction(const TreePtrInterface *ptr);

};


/// Common stuff for pattern nodes other than standard nodes
template<class PRE_RESTRICTION>
class Special : public SpecialBase, 
                public virtual PRE_RESTRICTION
{
public:
    virtual shared_ptr< TreePtrInterface > GetPreRestrictionArchitype() const
    {
        // Esta muchos indirection
        return shared_ptr<TreePtrInterface>( new TreePtr<PRE_RESTRICTION>( new PRE_RESTRICTION ));  
    }
};

};
#endif
