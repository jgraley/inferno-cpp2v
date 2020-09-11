#ifndef AND_RULE_ENGINE_HPP
#define AND_RULE_ENGINE_HPP

#include "query.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "common/mismatch.hpp"
#include "agent.hpp"
#include "placeholder_agent.hpp"
#include <set>

namespace CSP
{
class SolverHolder;
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
    
    void Configure( Agent *root_agent, const set<Agent *> &master_agents);
    void ConfigPopulateForSolver( list<Agent *> *normal_agents_ordered, 
                                  Agent *agent,
                                  const set<Agent *> &master_agents );
    void ConfigDetermineKeyersModuloMatchAny( set<PatternQuery::Link> *possible_keyer_links,
                                              Agent *agent,
                                              set<Agent *> *master_agents,
                                              set<Agent *> *match_any_agents ) const;
    void ConfigDeterminePossibleKeyers( set<PatternQuery::Link> *possible_keyer_links,
                                        Agent *agent,
                                        set<Agent *> master_agents ) const;
    void ConfigDetermineResiduals( set<PatternQuery::Link> *possible_keyer_links,
                                   Agent *agent,
                                   set<Agent *> master_agents );
    void ConfigFilterKeyers(set<PatternQuery::Link> *possible_keyer_links);
    void ConfigPopulateNormalAgents( set<Agent *> *normal_agents, 
                                     Agent *current_agent );
                                      
    void CompareCoupling( Agent *agent,
                          TreePtr<Node> x,
                          const CouplingMap *keys );
    void KeyCoupling( Agent *agent,
                      TreePtr<Node> x,
                      CouplingMap *keys );
    void CompareLinks( Agent *agent,
                       shared_ptr<const DecidedQuery> query );
    void CompareEvaluatorLinks( pair< shared_ptr<BooleanEvaluator>, DecidedQuery::Links > record,
							    const CouplingMap *hypothetical_solution_keys,
                                const CouplingMap *master_keys );
    void DecidedCompare( Agent *agent,
                         TreePtr<Node> x );
    void ExpandDomain( Agent *agent, set< TreePtr<Node> > &domain );
    void Compare( TreePtr<Node> start_x,
                  const CouplingMap *master_keys );
    void Compare( TreePtr<Node> start_x );
    void EnsureChoicesHaveIterators();

    const CouplingMap &GetCouplingKeys();

private:
    Agent *root_agent;
    set<Agent *> master_agents;
    set<Agent *> my_agents;   
    map< Agent *, shared_ptr<BooleanEvaluator> > my_evaluators;   
    map< Agent *, AndRuleEngine > my_evaluator_engines;
    map< Agent *, AndRuleEngine > my_abnormal_engines;
    map< Agent *, AndRuleEngine > my_multiplicity_engines;
    map< Agent *, shared_ptr<CSP::Constraint> > my_constraints;
    set<Agent *> master_boundary_agents;
    set< PatternQuery::Link > coupling_keyer_links;
    set< PatternQuery::Link > coupling_residual_links;
    map< PatternQuery::Link, PlaceholderAgent > diversion_agents; // link to real :-> diversion
    
    shared_ptr<Conjecture> conj;
    shared_ptr<CSP::SolverHolder> solver;
    CouplingMap my_keys; 
    CouplingMap solution_keys; 
    CouplingMap hypothetical_solution_keys; 
    CouplingMap master_coupling_candidates;
    const CouplingMap *master_keys;
    set<Agent *> reached; 
    set< pair< shared_ptr<BooleanEvaluator>, DecidedQuery::Links > > evaluator_records;   
    set< shared_ptr<const DecidedQuery::Link> > abnormal_links; 
    set< shared_ptr<const DecidedQuery::Link> > multiplicity_links;
};

#endif
};
