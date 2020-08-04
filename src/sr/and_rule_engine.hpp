#ifndef AND_RULE_ENGINE_HPP
#define AND_RULE_ENGINE_HPP

#include "query.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "common/mismatch.hpp"
#include "agent.hpp"
#include <set>

/// SR namespace contains the search and replace implementation
namespace SR 
{
class Agent;
class SlaveAgent;
class Conjecture;
class SpecialBase;
class CompareReplace;

/// Solve an and-rule matching problem
class AndRuleEngine : public virtual Traceable
{
protected:
    struct CompareState
    {
        Conjecture *conj;
        CouplingMap *slave_keys; // applies ACROSS PASSES
        const CouplingMap *master_keys;
        Set<Agent *> reached; // applies to CURRENT PASS only
        Set< shared_ptr<const AgentQuery> > evaluator_queries;    // applies ACROSS PASSES
        Set< std::pair< shared_ptr<const AgentQuery>, const AgentQuery::Link * > > abnormal_links;    // applies ACROSS PASSES
    };
    
    // Any mismatch this class throws
    class Mismatch : public ::Mismatch
    {
    };
    
    // Conjecture ran out of choices
    class NoSolution : public Mismatch
    {
    };
    
    // Evaluator returned false
    class EvaluatorFalse : public Mismatch
    {
    };
public:
    void CompareLinks( shared_ptr<const AgentQuery> query,
                       CompareState &state ) const;
    void CompareEvaluatorLinks( shared_ptr<const AgentQuery> query,
							    const CouplingMap *slave_keys ) const;
    void DecidedCompare( Agent *agent,
                         const TreePtrInterface *px,
                         CompareState &state ) const;
    void Compare( Agent *start_agent,
                  const TreePtrInterface *p_start_x,
                  Conjecture *conj,
                  CouplingMap *slave_keys,
                  const CouplingMap *master_keys ) const;
    void Compare( Agent *start_agent,
                  const TreePtrInterface *p_start_x,
                  const CouplingMap *master_keys ) const;

protected:
    std::shared_ptr< Set<Agent *> > my_agents;   
};

#endif
};
