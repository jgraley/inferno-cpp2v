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
        void PopulateForSolver( PatternLink link,
                                const set<Agent *> &master_agents );
        void DetermineKeyersModuloMatchAny( set<PatternLink> *coupling_keyer_links,
                                            PatternLink plink,
                                            set<Agent *> *master_agents,
                                            set<Agent *> *match_any_agents ) const;
        void DetermineKeyers( set<PatternLink> *coupling_keyer_links,
                              PatternLink plink,
                              set<Agent *> master_agents ) const;
        void DetermineResiduals( set<PatternLink> *coupling_keyer_links,
                                 Agent *agent,
                                 set<Agent *> master_agents );
        void DetermineNontrivialKeyers( set<PatternLink> *coupling_keyer_links );
        void PopulateNormalAgents( set<Agent *> *normal_agents, 
                                   set<PatternLink> *my_normal_links,
                                   PatternLink link );
        void CreateSubordniateEngines( const set<Agent *> &normal_agents, 
                                       const set<Agent *> &surrounding_agents );
 
        AndRuleEngine * const algo;
        TreePtr<Node> root_pattern;
        Agent *root_agent;
        PatternLink root_link;
        set<Agent *> master_agents;
        set<Agent *> my_normal_agents;   
        set<PatternLink> my_normal_links;
        set< Agent *> my_evaluators;   
        map< PatternLink, shared_ptr<AndRuleEngine> > my_evaluator_abnormal_engines;
        map< PatternLink, shared_ptr<AndRuleEngine> > my_free_abnormal_engines;
        map< PatternLink, shared_ptr<AndRuleEngine> > my_multiplicity_engines;
        map< PatternLink, shared_ptr<CSP::Constraint> > my_constraints;
        set<Agent *> master_boundary_agents;
        set<PatternLink> master_boundary_links;
        set<PatternLink> coupling_keyer_links;
        set<PatternLink> coupling_residual_links;
        set<PatternLink> coupling_nontrivial_keyer_links; // nontrivial means: child X has at least one residual link
        shared_ptr<Conjecture> conj;
        shared_ptr<CSP::SolverHolder> solver;
        set<PatternLink> by_equivalence_links;
        list<PatternLink> normal_links_ordered;
        
    private: // working variables in plan construction
        set<Agent *> reached_agents;
        set<PatternLink> reached_links; 
    } plan;
    
private:    
    typedef map< PatternLink, XLink > SolutionMap;

    void ExpandDomain( set< XLink > &domain );
    void StartCSPSolver( XLink root_xlink );
    void GetNextCSPSolution();
    void CompareLinks( Agent *agent,
                       shared_ptr<const DecidedQuery> query );
    void DecidedCompare( LocatedLink link );
    void CompareEvaluatorLinks( Agent *agent, 
                                const CouplingKeysMap *combined_keys, 
                                const SolutionMap *after_pass_keys );
    void CompareMultiplicityLinks( LocatedLink link, 
                                   const CouplingKeysMap *combined_keys ); 
    void RegenerationPassAgent( Agent *agent,
                                XLink xlink,
                                const CouplingKeysMap &subordinate_keys );
    void RegenerationPass();
    
public:
    void Compare( TreePtr<Node> root_xnode,
                  const CouplingKeysMap *master_keys );
    void CompareTrivialProblem( LocatedLink root_link );
    void Compare( TreePtr<Node> root_xnode );
    void EnsureChoicesHaveIterators();

    const CouplingKeysMap &GetCouplingKeys();

private:    
    void RecordLink( LocatedLink link );
    void CompareCoupling( const CouplingKeysMap &keys, const LocatedLink &residual_link );
    void KeyCoupling( CouplingKeysMap &keys, const LocatedLink &keyer_link );
    void AssertNewCoupling( const CouplingKeysMap &old, Agent *new_agent, TreePtr<Node> new_x, Agent *parent_agent );

    // Keys are mapped agaist agents, even though one of the links into
    // the agent is the keyer. This is well-defined and avoids merging
    // this instance's problem into master instance's problems. Note:
    // couplings are not allowed to specify the MMAX node.
    const CouplingKeysMap *master_keys;
    CouplingKeysMap my_coupling_keys; 
    CouplingKeysMap external_keys; 

    // These are partial solutions, and are mapped against the links
    // into the agents (half-link model). Note: solutions can specify
    // the MMAX node.
    SolutionMap basic_solution; 
};

#endif
};
