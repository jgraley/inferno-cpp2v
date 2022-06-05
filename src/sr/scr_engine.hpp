#ifndef SCR_ENGINE_HPP
#define SCR_ENGINE_HPP

#include "query.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "common/serial.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "common/mismatch.hpp"
#include "agents/agent.hpp"
#include "node/graphable.hpp"
#include "search_replace.hpp"

#include <set>

namespace SYM
{
    class BooleanExpression;
};

/// SR namespace contains the search and replace implementation
namespace SR 
{
class Agent;
class Conjecture;
class SpecialBase;
class AndRuleEngine;
class VNSequence;

class RequiresSubordinateSCREngine : public virtual Graphable
{
public:
	virtual TreePtr<Node> GetSearchPattern() const = 0;
	virtual TreePtr<Node> GetReplacePattern() const = 0;
};

class StartsOverlay : public virtual Graphable
{
public:
    virtual void StartPlanOverlay() = 0;
};

/// Common implementation for search+replace, compare+replace and slaves
class SCREngine : public virtual GraphIdable,
                  public SerialNumber
{      
public:
    SCREngine( VNSequence *vn_sequence,
               const CompareReplace *overall_master,
               CompareReplace::AgentPhases &in_progress_agent_phases,
               TreePtr<Node> cp,
               TreePtr<Node> rp = TreePtr<Node>(),
               const set<PatternLink> &master_plinks = set<PatternLink>(),                            
               const SCREngine *master = nullptr); /* if null, you are overall master */ 
    void PlanningStageTwo( const CompareReplace::AgentPhases &in_progress_agent_phases )
    {
        plan.PlanningStageTwo(in_progress_agent_phases); 
    }
    void PlanningStageThree(set<PatternLink> master_keyers)
    {
        plan.PlanningStageThree(master_keyers); 
    }
    void PlanningStageFive()
    {
        plan.PlanningStageFive(); 
    }
                    
private:
    struct Plan : public virtual Traceable
    {            
        Plan( SCREngine *algo,
              VNSequence *vn_sequence,
              const CompareReplace *overall_master,
              CompareReplace::AgentPhases &in_progress_agent_phases,
              TreePtr<Node> cp,
              TreePtr<Node> rp,
              const set<PatternLink> &master_plinks,                            
              const SCREngine *master ); /* if null, you are overall master */ 
        void CategoriseAgents( const set<PatternLink> &master_plinks, 
                               CompareReplace::AgentPhases &in_progress_agent_phases );
        void WalkVisible( set<PatternLink> &visible, 
                          list<PatternLink> *visible_postorder,
                          PatternLink base_plink, 
                          Agent::Path path ) const;
        void CreateMyEngines( CompareReplace::AgentPhases &in_progress_agent_phases );
        void PlanningStageTwo(const CompareReplace::AgentPhases &in_progress_agent_phases); // Stage one is the constructor
        void ConfigureAgents();
        void PlanningStageThree(set<PatternLink> master_keyers);
        void PlanningStageFive();
        void PlanCompare();
        void PlanReplace();
        void Dump();
        string GetTrace() const; // used for debug
        
        SCREngine * const algo;
        VNSequence *vn_sequence;
        const CompareReplace *overall_master_ptr;
        TreePtr<Node> root_pattern;
        PatternLink root_plink;
        Agent *root_agent;
        const SCREngine *master_ptr;
        const set<PatternLink> master_plinks;
        set<Agent *> master_agents;
        set<PatternLink> my_plinks;   
        set<Agent *> my_agents;   
        set<PatternLink> all_keyer_plinks;   
        set<StartsOverlay *> my_overlay_starter_engines;   
        map< RequiresSubordinateSCREngine *, shared_ptr<SCREngine> > my_engines;   
        shared_ptr<AndRuleEngine> and_rule_engine;
        CompareReplace::AgentPhases final_agent_phases;   
        list<PatternLink> my_replace_plinks_postorder;
        list<PatternLink> my_subordinate_plinks_postorder;
    } plan;

    void PostSlaveFixup( TreePtr<Node> through_subtree, TreePtr<Node> new_subtree ) const;
    void RunSlave( PatternLink plink_to_slave, TreePtr<Node> *p_root_x );
    TreePtr<Node> Replace( const SolutionMap *master_solution );

public: // For top level engine/VN trans
    void SingleCompareReplace( TreePtr<Node> *p_root_xnode,
                               const SolutionMap *master_solution );
    int RepeatingCompareReplace( TreePtr<Node> *p_root_xnode,
                                 const SolutionMap *master_solution );                                                                                               
    virtual void SetStopAfter( vector<int> ssa, int d=0 );
    static void SetMaxReps( int n, bool e );
    set< shared_ptr<SYM::BooleanExpression> > GetExpressions() const;
    list<const AndRuleEngine *> GetAndRuleEngines() const;
    list<const SCREngine *> GetSCREngines() const;
    void GenerateGraphRegions( Graph &graph ) const;
    
public: // For agents
    // Note: this is const but RepeatingCompareReplace() isn't. Why?
    // Because we're not calling OUR RepeatingCompareReplace but
    // the slave_engine's one - and that pointer is not const.
    void RecurseInto( RequiresSubordinateSCREngine *slave_agent, 
                      TreePtr<Node> *p_root_xnode ) const;    
    void SetReplaceKey( LocatedLink keyer_link ) const;
    TreePtr<Node> GetReplaceKey( PatternLink plink ) const;
    bool IsKeyedByAndRuleEngine( Agent *agent ) const; 
    bool IsKeyed( PatternLink plink ) const; 
    bool IsKeyed( Agent *agent ) const; 
    const CompareReplace * GetOverallMaster() const;
    XLink UniquifyDomainExtension( XLink xlink ) const;
    XLink FindDomainExtension( XLink xlink ) const;
    
    friend class Conjecture; 

public: // Trace stuff for anyone
    string GetTrace() const; // used for debug
    virtual string GetGraphId() const;    

private:    
    static int repetitions;
    static bool rep_error;
    
    vector<int> stop_after;
    int depth;    
    
    mutable SolutionMap replace_solution;
    bool keys_available = false;    
    mutable map< RequiresSubordinateSCREngine *, TreePtr<Node> > slave_though_subtrees;
};

};
#endif
