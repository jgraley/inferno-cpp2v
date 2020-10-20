#ifndef AND_RULE_ENGINE_HPP
#define AND_RULE_ENGINE_HPP

#include "query.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "common/mismatch.hpp"
#include "agents/agent.hpp"
#include "agents/placeholder_agent.hpp"
#include "agents/match_all_agent.hpp"
#include "link.hpp"
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
    
    AndRuleEngine( TreePtr<Node> root_pattern_, const set<Agent *> &master_agents);
    
    const struct Plan : public virtual Traceable
    {
        Plan( AndRuleEngine *algo, TreePtr<Node> root_pattern_, const set<Agent *> &master_agents);
        void PopulateForSolver( Agent *agent,
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
        TreePtr<Node> root_pattern;
        Agent *root_agent;
        PatternLink root_pattern_link;
        TreePtr< MatchAll<Node> > closure_pattern;
        set<Agent *> master_agents;
        set<Agent *> my_normal_agents;   
        set< Agent *> my_evaluators;   
        map< PatternLink, shared_ptr<AndRuleEngine> > my_evaluator_abnormal_engines;
        map< PatternLink, shared_ptr<AndRuleEngine> > my_free_abnormal_engines;
        map< PatternLink, shared_ptr<AndRuleEngine> > my_multiplicity_engines;
        map< Agent *, shared_ptr<CSP::Constraint> > my_constraints;
        set<Agent *> master_boundary_agents;
        set<PatternLink> master_boundary_links;
        set<PatternLink> coupling_keyer_links;
        set<PatternLink> coupling_residual_links;
        shared_ptr<Conjecture> conj;
        shared_ptr<CSP::SolverHolder> solver;
        set<PatternLink> by_equivalence_links;
        list<Agent *> normal_agents_ordered;
    private: // working varaibles in plan construction
        set<Agent *> reached; 
    } plan;
    
private:    
    typedef map< PatternLink, TreePtr<Node> > SolutionMap;

    void ExpandDomain( set< TreePtr<Node> > &domain );
    void StartCSPSolver( TreePtr<Node> start_x );
    void GetNextCSPSolution( TreePtr<Node> start_x );
    void CompareLinks( Agent *agent,
                       shared_ptr<const DecidedQuery> query );
    void DecidedCompare( LocatedLink link );
    void CompareEvaluatorLinks( Agent *agent, 
                                const CouplingKeys *combined_keys, 
                                const SolutionMap *after_pass_keys );
    void CompareMultiplicityLinks( LocatedLink link, 
                                   const CouplingKeys *combined_keys ); 
    void CompareAfterPassAgent( Agent *agent, 
                                TreePtr<Node> x,
                                const CouplingKeys &external_combined_keys,
                                const SolutionMap &internal_combined_solution );
    void CompareAfterPass();
    
public:
    void Compare( TreePtr<Node> start_x,
                  const CouplingKeys *master_keys );
    void CompareTrivialProblem( LocatedLink root_link );
    void CompareMasterKeys();
    void Compare( TreePtr<Node> start_x );
    void EnsureChoicesHaveIterators();

    const CouplingKeys &GetCouplingKeys();

private:    
    void CompareCoupling( const CouplingKeys &keys, const LocatedLink &residual_link );
    void KeyCoupling( CouplingKeys &keys, const LocatedLink &keyer_link );
    void AssertNewCoupling( const CouplingKeys &old, Agent *new_agent, TreePtr<Node> new_x, Agent *parent_agent );

    // Keys are mapped agaist agents, even though one of the links into
    // the agent is the keyer. This is well-defined and avoids merging
    // this instance's problem into master instance's problems.
    const CouplingKeys *master_keys;
    CouplingKeys internal_coupling_keys; 
    CouplingKeys external_keys; 

    // These are partial solutions, and are mapped against the links
    // into the agents (half-link model).
    SolutionMap master_coupling_candidates;
    SolutionMap internal_solution; 
};

#endif
};
