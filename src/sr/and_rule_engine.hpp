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
public:
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
    
    void Configure( Agent *root_agent_, std::shared_ptr< Set<Agent *> > _my_agents);
    
    void CompareLinks( shared_ptr<const AgentQuery> query );
    void CompareEvaluatorLinks( shared_ptr<const AgentQuery> query,
							    const CouplingMap *coupling_keys );
    void DecidedCompare( Agent *agent,
                         const TreePtrInterface *px );
    void Compare( const TreePtrInterface *p_start_x,
                  const CouplingMap *master_keys );
    void Compare( const TreePtrInterface *p_start_x );

    const Conjecture &GetConjecture();
    const CouplingMap &GetCouplingKeys();

private:
    Agent *root_agent;
    std::shared_ptr< Set<Agent *> > my_agents;   

    Conjecture conj;
    CouplingMap slave_keys; 
    const CouplingMap *master_keys;
    Set<Agent *> reached; 
    Set< shared_ptr<const AgentQuery> > evaluator_queries;   
    Set< std::pair< shared_ptr<const AgentQuery>, const AgentQuery::Link * > > abnormal_links; 
    Set< std::pair< shared_ptr<const AgentQuery>, const AgentQuery::Link * > > multiplicity_links;
};

#endif
};
