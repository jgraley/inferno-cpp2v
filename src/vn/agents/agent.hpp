#ifndef AGENT_HPP
#define AGENT_HPP

#include "common/common.hpp"
#include "../query.hpp"
#include "node/graphable.hpp"
#include "../sym/lazy_eval.hpp"


namespace VN
{ 
class SpecialBase;
class SCREngine;
class AndRuleEngine;
class Agent;
class XTreeDatabase;
class Patch;

/// Interface for Agents, which co-exist with pattern nodes and implement the search and replace funcitonality for each pattern node.
class Agent : public virtual Graphable,
              public virtual Node
{
public:  
    struct ReplaceKit
    {
        XTreeDatabase *x_tree_db;
        const map<const Agent *, PatternLink> *agents_to_keyers;
    };

    // really just to reduce the amount of typing if I change it
    typedef shared_ptr<Patch> ReplacePatchPtr;

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
        
	Agent() = default;
	Agent(const VN::Agent&) = default;
    Agent& operator=(Agent& other);
    virtual void SCRConfigure( Phase phase ) = 0;
                                   
    /// List the Agents reached via links during search
    virtual shared_ptr<PatternQuery> GetPatternQuery() const = 0;
    virtual shared_ptr<DecidedQuery> CreateDecidedQuery() const = 0;
    
    typedef function<shared_ptr<DecidedQuery>()> QueryLambda;

    /// Obtain a symbolic expression for this node's queries (coupling and maybe NLQ)
    virtual SYM::Lazy<SYM::BooleanExpression> SymbolicQuery( PatternLink keyer, const set<PatternLink> &residuals, bool coupling_only ) const = 0; 

    /// Get abnormal/multiplicity info from an Agent given partial map of locations of base and normal links. 
    virtual QueryLambda StartRegenerationQuery( const AndRuleEngine *acting_engine,
												const SolutionMap *hypothesis_links,
                                                PatternLink keyer_plink, 
                                                const XTreeDatabase *x_tree_db ) const = 0;
    virtual QueryLambda TestStartRegenerationQuery( const AndRuleEngine *acting_engine,
                                                    const SolutionMap *hypothesis_links,
													PatternLink keyer_plink,
                                                    const XTreeDatabase *x_tree_db ) const = 0;

    virtual void Reset() = 0;     
    virtual void PlanOverlay( SCREngine *acting_engine,
                              PatternLink me_plink, 
                              PatternLink bottom_layer_plink ) = 0;

    virtual TreePtr<Node> BuildForBuildersAnalysis( PatternLink me_plink,
                                                    const SCREngine *acting_engine ) = 0;
    virtual ReplacePatchPtr GenReplaceLayout( const ReplaceKit &kit, 
                                              PatternLink me_plink,
                                              const SCREngine *acting_engine ) = 0;
    virtual TreePtr<Node> GetEmbeddedSearchPattern() const = 0;
    virtual TreePtr<Node> GetEmbeddedReplacePattern() const = 0;
    
    virtual list<PatternLink> GetChildren() const = 0;
    virtual list<PatternLink> GetVisibleChildren( Path v ) const = 0;                        
    virtual bool ShouldGenerateCategoryClause() const = 0;                                

    static Agent *AsAgent( shared_ptr<Node> node );
    static Agent *TryAsAgent( shared_ptr<Node> node );
    static const Agent *AsAgentConst( shared_ptr<const Node> node );
    static const Agent *TryAsAgentConst( shared_ptr<const Node> node );
    // Note the constness: do not try to mutate the agent via this function
    virtual shared_ptr<const Node> GetPatternPtr() const = 0;
    virtual TreePtr<Node> GetArchetypeNode() const = 0;
    virtual shared_ptr< TreePtrInterface > GetArchetypeTreePtr() const = 0;
    virtual string GetPlanAsString() const = 0;
};

};

#endif
