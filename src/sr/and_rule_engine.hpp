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
#include "db/x_tree_database.hpp"
#include "csp/solver.hpp"

#include <set>
#include <map>

class Graph;
    
namespace SR 
{
class Agent;
class EmbeddedSCRAgent;
class Conjecture;
class SpecialBase;
class CompareReplace;
class XTreeDatabase;

/// Solve an and-rule matching problem
class AndRuleEngine : public virtual GraphIdable,
                      public SerialNumber
{
public:
    // Any mismatch this class throws
    class Mismatch : public ::Mismatch
    {
    };
    
    // And-rule engine was unsucessful
    class AndRuleMismatch : public Mismatch
    {
    };
    
    // Evaluator returned false
    class EvaluatorFalse : public Mismatch
    {
    };
    
    // A solution was discovered to be a match
    class SuccessfulMatch : public Exception
    {
	public:
		SuccessfulMatch( SolutionMap solution_ );
		const SolutionMap &GetSolution() const;
	private:
		SolutionMap solution;
    };

    AndRuleEngine( PatternLink base_plink, 
                   const set<PatternLink> &surrounding_plinks,
                   const set<PatternLink> &surrounding_keyer_plinks,
                   const map<const Agent *, PatternLink> &surrounding_agents_to_keyers,
                   const map<const Agent *, set<PatternLink>> &surrounding_agents_to_residuals );
    
    ~AndRuleEngine();
    
    struct Plan : public virtual Traceable
    {
        Plan( AndRuleEngine *algo,  
              PatternLink base_plink, 
              const set<PatternLink> &surrounding_plinks,
              const set<PatternLink> &surrounding_keyer_plinks,
              const map<const Agent *, PatternLink> &surrounding_agents_to_keyers,
              const map<const Agent *, set<PatternLink>> &surrounding_agents_to_residuals );
        void PlanningStageFive( shared_ptr<const Lacing> lacing );

        void PopulateNormalAgents( set<Agent *> *normal_agents, 
                                   set<PatternLink> *my_normal_links,
                                   PatternLink link );
        void PopulateBoundaryAgents( PatternLink link,
                                    const set<Agent *> &surrounding_agents );

        void DetermineKeyersModuloDisjunction( PatternLink plink,
                                               set<Agent *> *senior_agents,
                                               set<Agent *> *disjunction_agents );                                               
        void DetermineKeyers( PatternLink plink,
                              set<Agent *> surrounding_agents );
        void DetermineResiduals( Agent *agent,
                                 set<Agent *> surrounding_agents );
                                 
        void ConfigureAgents();
        
        void DeduceCSPVariables();
        void CreateMyFullSymbolics();
        void CreateBoundarySymbolics();
        void SymbolicRewrites();

        void CreateSubordniateEngines( const set<Agent *> &normal_agents, 
                                       const set<PatternLink> &subordinate_surrounding_plinks, 
                                       const set<PatternLink> &subordinate_surrounding_keyer_plinks );

        // CSP solver stuff
        void CreateMyConstraints( list< shared_ptr<CSP::Constraint> > &constraints_list,
                                  shared_ptr<const Lacing> lacing );
        void Dump();

        string GetTrace() const; // used for debug
        
        AndRuleEngine * const algo;
        
        const TreePtr<Node> base_pattern;
        
        set< shared_ptr<SYM::BooleanExpression> > expressions_from_agents;
        set< shared_ptr<SYM::BooleanExpression> > expressions_split;
        set< shared_ptr<SYM::BooleanExpression> > expressions_for_current_solve;
        map< set<PatternLink>, set<shared_ptr<SYM::BooleanExpression>> > expressions_condensed;
        map< PatternLink, shared_ptr<AndRuleEngine> > my_free_abnormal_engines;
        map< PatternLink, shared_ptr<AndRuleEngine> > my_evaluator_abnormal_engines;
        map< PatternLink, shared_ptr<AndRuleEngine> > my_multiplicity_engines;
        shared_ptr<CSP::Solver> csp_solver;
        
        const PatternLink base_plink;
        Agent * const base_agent;
        const set<PatternLink> surrounding_plinks;
        const set<PatternLink> surrounding_keyer_plinks;
        set<Agent *> surrounding_agents;
        set<Agent *> my_normal_agents;   
        set<PatternLink> my_normal_links;
        set<PatternLink> my_normal_links_unique_by_agent;
        set< Agent *> my_evaluators;   
        set<Agent *> boundary_agents;
        set<PatternLink> coupling_residual_links;
        set<PatternLink> coupling_keyer_links_all; // All keyers
        set<PatternLink> my_boundary_links; // These are ALL residuals
        set<PatternLink> boundary_keyer_links; // Keyers linked from surroundings
        set<PatternLink> my_fixed_keyer_links; 
        map< Agent *, set<PatternLink> > parent_links_to_my_normal_agents;
        map< Agent *, set<PatternLink> > parent_residual_links_to_boundary_agents;
        map<const Agent *, PatternLink> agents_to_keyers;
        map<const Agent *, set<PatternLink>> agents_to_residuals;
        list<PatternLink> free_normal_links_ordered;
        set<PatternLink> current_solve_plinks;
        list<PatternLink> normal_and_boundary_links_preorder;
        map<const Agent *, shared_ptr<Conjecture>> agents_to_nlq_conjectures;

    private: // working variables in plan construction
        set<Agent *> reached_agents;
        set<PatternLink> reached_links; 
    } plan;
    
    void PlanningStageFive( shared_ptr<const Lacing> lacing );      
    
private:        
    void CompareLinks( Agent *agent,
                       shared_ptr<const DecidedQuery> query );
    void DecidedCompare( LocatedLink link );
    void CompareEvaluatorLinks( Agent *agent, 
                                const SolutionMap *solution_for_subordinates, 
                                const SolutionMap *solution_for_evaluators,
                                set<TreePtr<Node>> *keep_alive_nodes );
    void CompareMultiplicityNode( PatternLink plink, TreePtr<Node> node, 
                                  const SolutionMap *solution_for_subordinates,
                                  set<TreePtr<Node>> *keep_alive_nodes ); 
    void AgentRegeneration( Agent *agent,
                            SolutionMap &basic_solution,
                            const SolutionMap &solution_for_subordinates,
                            set<TreePtr<Node>> *keep_alive_nodes );
    void OnSolution(SolutionMap basic_solution, 
                    const SolutionMap &my_fixed_assignments, 
                    const SolutionMap *universal_assignments,
                    set<TreePtr<Node>> *keep_alive_nodes);
    
public:
    void SetXTreeDb( shared_ptr<const XTreeDatabase> x_tree_db );
    SolutionMap Compare( XLink base_xlink,
                         const SolutionMap *universal_assignments,
                         set<TreePtr<Node>> *keep_alive_nodes );

    const set<Agent *> &GetKeyedAgents() const;
    set<PatternLink> GetKeyerPatternLinks() const;
    
    map<const Agent *, PatternLink> GetAgentsToKeyersMap() const;
    map<const Agent *, set<PatternLink>> GetAgentsToResidualsMap() const;
    
    set< shared_ptr<SYM::BooleanExpression> > GetExpressions() const;
    list<const AndRuleEngine *> GetAndRuleEnginesInclThis() const;

    shared_ptr<Conjecture> GetNLQConjecture(const Agent *agent) const;

    string GetTrace() const; // used for debug
    virtual string GetGraphId() const;
    void GenerateGraphRegions( Graph &graph, string scr_engine_id ) const;
    void GenerateMyGraphRegion( Graph &graph, string scr_engine_id ) const;

private:
    // Information about the X tree
    shared_ptr<const XTreeDatabase> x_tree_db;
};

#endif
};
