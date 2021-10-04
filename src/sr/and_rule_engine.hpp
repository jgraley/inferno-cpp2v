#ifndef AND_RULE_ENGINE_HPP
#define AND_RULE_ENGINE_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "common/mismatch.hpp"
#include "common/serial.hpp"
#include "agents/agent.hpp"
#include "agents/conjunction_agent.hpp"
#include "render/graph.hpp"

#include "query.hpp"
#include "link.hpp"
#include "coupling.hpp"
#include "the_knowledge.hpp"

#include <set>
#include <map>

class Graph;

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
class AndRuleEngine : public virtual GraphIdable,
                      public SerialNumber
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
    
    AndRuleEngine( PatternLink root_plink, 
                   const unordered_set<PatternLink> &master_plinks,
                   const SerialNumber *serial_to_use = nullptr );
    
    ~AndRuleEngine();
    
    const struct Plan : public virtual Traceable
    {
        Plan( AndRuleEngine *algo,  
              PatternLink root_plink, 
              const unordered_set<PatternLink> &master_plinks);
        void CreateMyConstraints( list< shared_ptr<CSP::Constraint> > &constraints_list );
        void CreateMasterCouplingConstraints( list< shared_ptr<CSP::Constraint> > &constraints_list );
        void CreateCSPSolver( const list< shared_ptr<CSP::Constraint> > &constraints_list );
        void PopulateSomeThings( PatternLink link,
                                 const unordered_set<Agent *> &master_agents );
        void DetermineKeyersModuloDisjunction( PatternLink plink,
                                               unordered_set<Agent *> *master_agents,
                                               unordered_set<Agent *> *match_any_agents );
        void DetermineKeyers( PatternLink plink,
                              unordered_set<Agent *> master_agents );
        void DetermineResiduals( Agent *agent,
                                 unordered_set<Agent *> master_agents );
        void DetermineNontrivialKeyers();
        void ConfigureAgents();
        void PopulateNormalAgents( unordered_set<Agent *> *normal_agents, 
                                   unordered_set<PatternLink> *my_normal_links,
                                   PatternLink link );
        void CreateSubordniateEngines( const unordered_set<Agent *> &normal_agents, 
                                       const unordered_set<PatternLink> &surrounding_plinks );
        string GetTrace() const; // used for debug
        
        AndRuleEngine * const algo;
        const PatternLink root_plink;
        const TreePtr<Node> root_pattern;
        Agent * const root_agent;
        const unordered_set<PatternLink> master_plinks;
        unordered_set<Agent *> master_agents;
        unordered_set<Agent *> my_normal_agents;   
        unordered_set<PatternLink> my_normal_links;
        unordered_set<PatternLink> my_normal_links_unique_by_agent;
        bool trivial_problem;
        unordered_set< Agent *> my_evaluators;   
        unordered_map< PatternLink, shared_ptr<AndRuleEngine> > my_free_abnormal_engines;
        unordered_map< PatternLink, shared_ptr<AndRuleEngine> > my_evaluator_abnormal_engines;
        unordered_map< PatternLink, shared_ptr<AndRuleEngine> > my_multiplicity_engines;
        unordered_map< PatternLink, shared_ptr<CSP::Constraint> > my_constraints;
        unordered_set<Agent *> master_boundary_agents;
        unordered_set<PatternLink> master_boundary_links;
        unordered_set<PatternLink> master_boundary_keyer_links;
        unordered_set<PatternLink> master_boundary_residual_links;
        unordered_set<PatternLink> coupling_residual_links;
        unordered_set<PatternLink> coupling_keyer_links; // only where child X has at least one residual link (i.e. non-trivial)
        unordered_map< Agent *, unordered_set<PatternLink> > parent_links_to_my_normal_agents;
        unordered_map< Agent *, unordered_set<PatternLink> > parent_links_to_master_boundary_agents;

        shared_ptr<Conjecture> conj;
        shared_ptr<CSP::SolverHolder> solver;
        list<PatternLink> normal_links_ordered;
        
    private: // working variables in plan construction
        unordered_set<Agent *> reached_agents;
        unordered_set<PatternLink> reached_links; 
    } plan;
    
private:        
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
                                const CouplingKeysMap &subordinate_keys );
    void RegenerationPass();
    
public:
    void Compare( XLink root_xlink,
                  const CouplingKeysMap *master_keys,
                  const TheKnowledge *knowledge );
    void CompareTrivialProblem( LocatedLink root_link );
    void Compare( TreePtr<Node> root_xnode );
    void EnsureChoicesHaveIterators();

    const CouplingKeysMap &GetCouplingKeys();
    const void ClearCouplingKeys();

private:    
    void RecordLink( LocatedLink link, KeyProducer place );
    void CompareCoupling( const CouplingKeysMap &keys, const LocatedLink &residual_link, KeyConsumer consumer );
    void KeyCoupling( CouplingKeysMap &keys, const LocatedLink &keyer_link, KeyProducer place );
    void AssertNewCoupling( const CouplingKeysMap &old, Agent *new_agent, XLink new_xlink, Agent *parent_agent );

public:
    string GetTrace() const; // used for debug
    list<const AndRuleEngine *> GetAndRuleEngines() const;
    virtual string GetGraphId() const;
    void GenerateGraphRegions( Graph &graph, string scr_engine_id ) const;
    void GenerateMyGraphRegion( Graph &graph, string scr_engine_id ) const;

private:
    // Information about the X tree
    const TheKnowledge *knowledge;

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
    bool used = false;  
};

#endif
};
