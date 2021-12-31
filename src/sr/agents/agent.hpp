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
#include "../sym/lazy.hpp"

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
    class Mismatch : public ::Mismatch {};
    class NLQMismatch : public Mismatch {};
    class NLQConjOutAfterHitMismatch : public NLQMismatch {};
    class NLQConjOutAfterMissMismatch : public NLQMismatch {};
    class NLQFromDQLinkMismatch : public NLQMismatch {};
    class MMAXPropagationMismatch : public Mismatch {};
    class PreRestrictionMismatch : public Mismatch {};
    
    class CouplingMismatch : public Mismatch {};
        
    Agent& operator=(Agent& other);
	virtual void SCRConfigure( const SCREngine *e,
                               Phase phase ) = 0;
    virtual void ConfigureCoupling( const Traceable *e,
                                    PatternLink keyer_plink_, 
                                    set<PatternLink> residual_plinks_ ) = 0;
    virtual void AddResiduals( set<PatternLink> residual_plinks_ ) = 0;
                                   
    /// List the Agents reached via links during search
    virtual shared_ptr<PatternQuery> GetPatternQuery() const = 0;
    virtual shared_ptr<DecidedQuery> CreateDecidedQuery() const = 0;
    
    /// Produce info about an Agent given location (x) and a vector of choices (conj). 
    virtual void RunDecidedQuery( DecidedQueryAgentInterface &query,
                                  XLink keyer_xlink ) const = 0;     
    
    typedef function<shared_ptr<DecidedQuery>()> QueryLambda;

    /// Test an Agent given partial map of locations of base and normal links. 
    virtual bool ImplHasNLQ() const = 0;                                              

    /// Test an agent given given partial map of locations of keyer and residuals.  
    virtual void RunCouplingQuery( const SolutionMap *hypothesis_links ) const = 0;                                       

    /// Obtain a symbolic expression for this node's queries (coupling and maybe NLQ)
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicQuery( bool coupling_only ) const = 0; 

    /// Get abnormal/multiplicity info from an Agent given partial map of locations of base and normal links. 
    virtual QueryLambda StartRegenerationQuery( const SolutionMap *hypothesis_links,
                                                const TheKnowledge *knowledge,
                                                bool use_DQ = false ) const = 0;
    virtual QueryLambda TestStartRegenerationQuery( const SolutionMap *hypothesis_links,
                                                    const TheKnowledge *knowledge ) const = 0;
    virtual set<XLink> ExpandNormalDomain( const unordered_set<XLink> &keyer_xlinks ) = 0;
    virtual void ResetNLQConjecture() = 0;    

    virtual const SCREngine *GetMasterSCREngine() const = 0;      
    virtual PatternLink GetKeyerPatternLink() const = 0;
    virtual set<PatternLink> GetResidualPatternLinks() const = 0;                                  

    virtual void Reset() = 0;     
    virtual void PlanOverlay( PatternLink me_plink, 
                              PatternLink under_plink ) = 0;
    virtual bool ReplaceKeyerQuery( PatternLink me_plink, 
                                    set<PatternLink> keyer_plinks ) = 0;

    virtual TreePtr<Node> BuildReplace( PatternLink me_plink ) = 0;
    virtual TreePtr<Node> DuplicateSubtree( TreePtr<Node> source,
                                            TreePtr<Node> source_terminus = TreePtr<Node>(),
                                            TreePtr<Node> dest_terminus = TreePtr<Node>() ) const = 0;
    virtual list<PatternLink> GetChildren() const = 0;
    virtual list<PatternLink> GetVisibleChildren( Path v ) const = 0;
		
    virtual bool IsPreRestrictionMatch( XLink x ) const = 0; // return true if matches

	static Agent *AsAgent( shared_ptr<Node> node );
	static Agent *TryAsAgent( shared_ptr<Node> node );
	static const Agent *AsAgentConst( shared_ptr<const Node> node );
	static const Agent *TryAsAgentConst( shared_ptr<const Node> node );
    // Note the constness: thou shalt not try to mutate the agent via this function
    virtual shared_ptr<const Node> GetPatternPtr() const = 0;
    virtual string GetPlanAsString() const = 0;
};

};

#endif
