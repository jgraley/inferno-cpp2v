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
    void Configure( std::shared_ptr< Set<Agent *> > _my_agents);
    
    void CompareLinks( shared_ptr<const AgentQuery> query ) const;
    void CompareEvaluatorLinks( shared_ptr<const AgentQuery> query,
							    const CouplingMap *coupling_keys ) const;
    void DecidedCompare( Agent *agent,
                         const TreePtrInterface *px ) const;
    void Compare( Agent *start_agent,
                  const TreePtrInterface *p_start_x,
                  const CouplingMap *master_keys ) const;
    
    const Conjecture &GetConjecture();
    const CouplingMap &GetCouplingKeys();

protected:
    std::shared_ptr< Set<Agent *> > my_agents;   

    // See #66 for getting rid of mutable
    mutable Conjecture conj;
    mutable CouplingMap slave_keys; 
    mutable const CouplingMap *master_keys;
    mutable Set<Agent *> reached; 
    mutable Set< shared_ptr<const AgentQuery> > evaluator_queries;   
    mutable Set< std::pair< shared_ptr<const AgentQuery>, const AgentQuery::Link * > > abnormal_links; 
};

#endif
};
