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
                   const set<PatternLink> &master_plinks,
                   const set<PatternLink> &master_keyer_plinks );
    
    ~AndRuleEngine();
    
    const struct Plan : public virtual Traceable
    {
        Plan( AndRuleEngine *algo,  
              PatternLink root_plink, 
              const set<PatternLink> &master_plinks,
              const set<PatternLink> &master_keyer_plinks );
        void PopulateMasterBoundaryStuff( PatternLink link,
                                          const set<Agent *> &master_agents );
        void DetermineKeyersModuloDisjunction( PatternLink plink,
                                               set<Agent *> *senior_agents,
                                               set<Agent *> *disjunction_agents );
        void DetermineKeyers( PatternLink plink,
                              set<Agent *> master_agents );
        void DetermineResiduals( Agent *agent,
                                 set<Agent *> master_agents );
        void ConfigureAgents();
        void PopulateNormalAgents( set<Agent *> *normal_agents, 
                                   set<PatternLink> *my_normal_links,
                                   PatternLink link );
        void CreateSubordniateEngines( const set<Agent *> &normal_agents, 
                                       const set<PatternLink> &surrounding_plinks, 
                                       const set<PatternLink> &surrounding_keyer_plinks );
        void CreateMyFullSymbolics();
        void CreateMasterCouplingSymbolics();

        void SymbolicRewrites();

        // CSP solver stuff
        void DeduceCSPVariables();
        void CreateMyConstraints( list< shared_ptr<CSP::Constraint> > &constraints_list );
        void Dump();

        string GetTrace() const; // used for debug
        
        AndRuleEngine * const algo;
        const PatternLink root_plink;
        const TreePtr<Node> root_pattern;
        Agent * const root_agent;
        const set<PatternLink> master_plinks;
        const set<PatternLink> master_keyer_plinks;
        set<Agent *> master_agents;
        set<Agent *> my_normal_agents;   
        set<PatternLink> my_normal_links;
        set<PatternLink> my_normal_links_unique_by_agent;
        set< Agent *> my_evaluators;   
        map< PatternLink, shared_ptr<AndRuleEngine> > my_free_abnormal_engines;
        map< PatternLink, shared_ptr<AndRuleEngine> > my_evaluator_abnormal_engines;
        map< PatternLink, shared_ptr<AndRuleEngine> > my_multiplicity_engines;
        set<Agent *> master_boundary_agents;
        set<PatternLink> coupling_residual_links;
        set<PatternLink> coupling_keyer_links_all; // All keyers
        set<PatternLink> my_master_boundary_links; // These are ALL residuals
        set<PatternLink> master_boundary_keyer_links; // Keyers linked from master
        map< Agent *, set<PatternLink> > parent_links_to_my_normal_agents;
        map< Agent *, set<PatternLink> > parent_residual_links_to_master_boundary_agents;
        list<PatternLink> free_normal_links_ordered;
        set<PatternLink> current_solve_plinks;
        set< shared_ptr<SYM::BooleanExpression> > expressions_from_agents;
        set< shared_ptr<SYM::BooleanExpression> > expressions_split;
        set< shared_ptr<SYM::BooleanExpression> > expressions_for_current_solve;
        map< set<PatternLink>, set<shared_ptr<SYM::BooleanExpression>> > expressions_condensed;

        shared_ptr<CSP::SolverHolder> solver_holder;
        list<PatternLink> normal_and_boundary_links_preorder;
        
    private: // working variables in plan construction
        set<Agent *> reached_agents;
        set<PatternLink> reached_links; 
    } plan;
    
private:        
    void StartCSPSolver( XLink root_xlink );
    void GetNextCSPSolution( LocatedLink root_link );
    void CompareLinks( Agent *agent,
                       shared_ptr<const DecidedQuery> query );
    void DecidedCompare( LocatedLink link );
    void CompareEvaluatorLinks( Agent *agent, 
                                const SolutionMap *solution_for_subordinates, 
                                const SolutionMap *solution_for_evaluators );
    void CompareMultiplicityLinks( LocatedLink link, 
                                   const SolutionMap *solution_for_subordinates ); 
    void RegenerationPassAgent( Agent *agent,
                                const SolutionMap &solution_for_subordinates );
    void RegenerationPass();
    
public:
    void Compare( XLink root_xlink,
                  const SolutionMap *master_solution,
                  const TheKnowledge *knowledge );
    void Compare( TreePtr<Node> root_xnode );

    const SolutionMap &GetSolution();
    void ClearSolution();
    const set<Agent *> &GetKeyedAgents() const;
    const set<PatternLink> GetKeyerPatternLinks() const;
    set< shared_ptr<SYM::BooleanExpression> > GetExpressions() const;
    list<const AndRuleEngine *> GetAndRuleEngines() const;

    string GetTrace() const; // used for debug
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
    const SolutionMap *master_solution;

    // These are partial solutions, and are mapped against the links
    // into the agents (half-link model). Note: solutions can specify
    // the MMAX node.
    SolutionMap basic_solution; 
    bool used = false;  
};

#endif
};
