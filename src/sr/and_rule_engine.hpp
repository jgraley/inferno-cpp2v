#ifndef AND_RULE_ENGINE_HPP
#define AND_RULE_ENGINE_HPP

#include "query.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "common/mismatch.hpp"
#include "agents/agent.hpp"
#include "agents/placeholder_agent.hpp"
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
    
    AndRuleEngine( Agent *root_agent, const set<Agent *> &master_agents);
    
    const struct Plan : public virtual Traceable
    {
        Plan( AndRuleEngine *algo, Agent *root_agent, const set<Agent *> &master_agents);
        void PopulateForSolver( list<Agent *> *normal_agents_ordered, 
                                Agent *agent,
                                const set<Agent *> &master_agents );
        void DetermineKeyersModuloMatchAny( set<PatternLink> *possible_keyer_links,
                                            Agent *agent,
                                            set<Agent *> *master_agents,
                                            set<Agent *> *match_any_agents ) const;
        void DeterminePossibleKeyers( set<PatternLink> *possible_keyer_links,
                                      Agent *agent,
                                      set<Agent *> master_agents ) const;
        void DetermineResiduals( set<PatternLink> *possible_keyer_links,
                                 Agent *agent,
                                 set<Agent *> master_agents );
        void FilterKeyers( set<PatternLink> *possible_keyer_links );
        void PopulateNormalAgents( set<Agent *> *normal_agents, 
                                   Agent *agent );
        void CreateVariousThings( const set<Agent *> &normal_agents, 
                                  const set<Agent *> &surrounding_agents );
 
        AndRuleEngine * const algo;
        Agent *root_agent;
        set<Agent *> master_agents;
        set<Agent *> my_normal_agents;   
        set< Agent *> my_evaluators;   
        map< PatternLink, shared_ptr<AndRuleEngine> > my_evaluator_abnormal_engines;
        map< PatternLink, shared_ptr<AndRuleEngine> > my_free_abnormal_engines;
        map< PatternLink, shared_ptr<AndRuleEngine> > my_multiplicity_engines;
        map< PatternLink, shared_ptr<PlaceholderAgent> > diversion_agents; 
        map< Agent *, shared_ptr<CSP::Constraint> > my_constraints;
        set<Agent *> master_boundary_agents;
        set<PatternLink> master_boundary_links;
        set<PatternLink> coupling_keyer_links;
        set<PatternLink> coupling_residual_links;
        shared_ptr<Conjecture> conj;
        shared_ptr<CSP::SolverHolder> solver;
    private: // working varaibles in plan construction
        set<Agent *> reached; 
    } plan;
    
    void ExpandDomain( Agent *agent, set< TreePtr<Node> > &domain );
    void StartCSPSolver( TreePtr<Node> start_x,
                         const CouplingMap *master_keys );
    void GetNextCSPSolution();
    void CompareCoupling( Agent *agent,
                          TreePtr<Node> x,
                          const CouplingMap *keys );
    void KeyCoupling( Agent *agent,
                      TreePtr<Node> x,
                      CouplingMap *keys );
    void CompareLinks( Agent *agent,
                       shared_ptr<const DecidedQuery> query );
    void DecidedCompare( Agent *agent,
                         TreePtr<Node> x );
    void CompareEvaluatorLinks( Agent *agent,
							    const CouplingMap *after_pass_keys,
                                const CouplingMap *master_keys );
    void CompareAfterPass( const CouplingMap *master_keys );
    void Compare( TreePtr<Node> start_x,
                  const CouplingMap *master_keys );
    void CompareTrivialProblem( TreePtr<Node> start_x,
                                const CouplingMap *master_keys );
    void CompareMasterKeys( const CouplingMap *master_keys );
    void Compare( TreePtr<Node> start_x );
    void EnsureChoicesHaveIterators();

    const CouplingMap &GetCouplingKeys();

private:    
    void AssertNewCoupling( const CouplingMap &old, Agent *new_agent, TreePtr<Node> new_x, Agent *parent_agent );

    CouplingMap working_keys; 
    CouplingMap solution_keys; 
    CouplingMap after_pass_keys; 
    CouplingMap master_coupling_candidates;
    const CouplingMap *master_keys;
};

#endif
};
