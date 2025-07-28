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
#include "db/duplicate.hpp"
#include "up/up_common.hpp"

#include <set>

namespace SYM
{
    class BooleanExpression;
};


namespace SR 
{
class Agent;
class Conjecture;
class SpecialBase;
class AndRuleEngine;
class VNSequence;

class StartsOverlay : public virtual Graphable
{
public:
    virtual void StartPlanOverlay() = 0;
};

/// Common implementation for search+replace, compare+replace and embeddeds
class SCREngine : public virtual GraphIdable,
                  public SerialNumber
{      
public:
    SCREngine( VNSequence *vn_sequence,
               const CompareReplace *root_engine,
               CompareReplace::AgentPhases &in_progress_agent_phases,
               TreePtr<Node> cp,
               TreePtr<Node> rp = TreePtr<Node>(),
               const set<PatternLink> &enclosing_plinks = set<PatternLink>(),                            
               const SCREngine *enclosing_engine = nullptr); /* if null, you are root SCR engine */ 
    void PlanningStageTwo( const CompareReplace::AgentPhases &in_progress_agent_phases )
    {
        plan.PlanningStageTwo(in_progress_agent_phases); 
    }
    void PlanningStageThree(set<PatternLink> enclosing_keyers)
    {
        plan.PlanningStageThree(enclosing_keyers); 
    }
    void PlanningStageFive( shared_ptr<const Lacing> lacing )
    {
        plan.PlanningStageFive(lacing); 
    }
                    
private:
    struct Plan : public virtual Traceable
    {            
        Plan( SCREngine *algo,
              VNSequence *vn_sequence,
              const CompareReplace *root_engine,
              CompareReplace::AgentPhases &in_progress_agent_phases,
              TreePtr<Node> cp,
              TreePtr<Node> rp,
              const set<PatternLink> &enclosing_plinks,                            
              const SCREngine *enclosing_engine ); /* if null, you are root SCR engine */ 
        void CategoriseAgents( const set<PatternLink> &enclosing_plinks, 
                               CompareReplace::AgentPhases &in_progress_agent_phases );
        void WalkVisible( set<PatternLink> &visible, 
                          list<PatternLink> *visible_postorder,
                          PatternLink plink, 
                          Agent::Path path ) const;
        void CreateMyEngines( CompareReplace::AgentPhases &in_progress_agent_phases );
        void PlanningStageTwo(const CompareReplace::AgentPhases &in_progress_agent_phases); // Stage one is the constructor
        void ConfigureAgents();
        void PlanningStageThree(set<PatternLink> enclosing_keyers);
        void PlanCompare();
        void PlanReplace();
        bool IsAgentKeyer( Agent *agent ) const;   
        void PlanningStageFive( shared_ptr<const Lacing> lacing );
        void Dump();
        string GetTrace() const; // used for debug
        
        SCREngine * const algo;
        VNSequence *vn_sequence;
        const CompareReplace *root_engine;
        TreePtr<Node> pattern_origin;
        PatternLink origin_plink;
        Agent *origin_agent;
        const SCREngine *enclosing_engine;
        const set<PatternLink> enclosing_plinks;
        set<Agent *> enclosing_agents;
        set<PatternLink> my_plinks;   
        set<Agent *> my_agents;   
        set<PatternLink> and_rule_engine_keyer_plinks;   
        set<PatternLink> all_keyer_plinks;   
        set<PatternLink> keyed_before_replace_plinks;
        set<StartsOverlay *> my_overlay_starter_engines;   
        map< Agent *, shared_ptr<SCREngine> > my_engines;   
        shared_ptr<AndRuleEngine> and_rule_engine;
        CompareReplace::AgentPhases final_agent_phases;   
        list<PatternLink> my_replace_only_plinks_postorder;
        list<PatternLink> my_embedded_plinks_postorder;
    } plan;

    void RunEmbedded( PatternLink plink_to_embedded );
    ReplaceAssignments Replace( XLink origin_xlink );
    void SingleCompareReplace( XLink origin_xlink );                                                                                              

public: // For top level engine/VN trans
    int RepeatingCompareReplace( XLink origin_xlink,
                                 SolutionMap *universal_assignments_ );                                                                                               
    virtual void SetStopAfter( vector<int> ssa, int d=0 );
    static void SetMaxReps( int n, bool e );
    void SetXTreeDb( shared_ptr<XTreeDatabase> x_tree_db );
    set< shared_ptr<SYM::BooleanExpression> > GetExpressions() const;
    list<const AndRuleEngine *> GetAndRuleEngines() const;
    list<const SCREngine *> GetSCREngines() const;
    void GenerateGraphRegions( Graph &graph ) const;
    
public: // For agents
    // Note: this is const but RepeatingCompareReplace() isn't. Why?
    // Because we're not calling OUR RepeatingCompareReplace but
    // the embedded_engine's one - and that pointer is not const. 
    bool IsKeyedBeforeReplace( PatternLink plink ) const;
    XLink GetKey( PatternLink plink ) const;
    bool IsKeyedByAndRuleEngine( Agent *agent ) const; 
    const CompareReplace * GetOverallMaster() const;
    
    friend class Conjecture; 

public: // Trace stuff for anyone
    string GetTrace() const; // used for debug
    virtual string GetGraphId() const;    

private:    
    static int repetitions;
    static bool rep_error;
    
    vector<int> stop_after;
    vector<int>::size_type depth;    
    
    shared_ptr<XTreeDatabase> x_tree_db;   
    SolutionMap *universal_assignments = nullptr;    
};

};
#endif
