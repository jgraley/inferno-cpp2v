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

namespace CSP
{
class SimpleSolver;
class Constraint;
};
    
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
    
    void Configure( Agent *root_agent, const Set<Agent *> &master_agents);
    void ConfigPopulateNormalAgents( Set<Agent *> *normal_agents, Agent *current_agent );

    void CompareLinks( shared_ptr<const DecidedQuery> query );
    void CompareEvaluatorLinks( shared_ptr<const DecidedQuery> query,
							    const CouplingMap *coupling_keys );
    void DecidedCompare( Agent *agent,
                         TreePtr<Node> x );
    void Compare( TreePtr<Node> start_x,
                  const CouplingMap *master_keys );
    void Compare( TreePtr<Node> start_x );
    void EnsureChoicesHaveIterators();

    const Conjecture &GetConjecture();
    const CouplingMap &GetCouplingKeys();

private:
    Agent *root_agent;
    Set<Agent *> my_agents;   
    map< Agent *, AndRuleEngine > my_abnormal_engines;
    map< Agent *, AndRuleEngine > my_multiplicity_engines;
    map< Agent *, shared_ptr<CSP::Constraint> > my_constraints;
    
    Conjecture conj;
    shared_ptr<CSP::SimpleSolver> solver;
    CouplingMap my_keys; 
    const CouplingMap *master_keys;
    Set<Agent *> reached; 
    Set< shared_ptr<const DecidedQuery> > evaluator_queries;   
    Set< std::pair< shared_ptr<const DecidedQuery>, const DecidedQuery::Link * > > abnormal_links; 
    Set< std::pair< shared_ptr<const DecidedQuery>, const DecidedQuery::Link * > > multiplicity_links;
};

#endif
};
