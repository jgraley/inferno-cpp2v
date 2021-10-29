#ifndef SCR_ENGINE_HPP
#define SCR_ENGINE_HPP

#include "query.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "common/serial.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "common/mismatch.hpp"
#include "the_knowledge.hpp"
#include "agents/agent.hpp"
#include "node/graphable.hpp"
#include "search_replace.hpp"

#include <set>


/// SR namespace contains the search and replace implementation
namespace SR 
{
class Agent;
class Conjecture;
class SpecialBase;
class AndRuleEngine;

class RequiresSubordinateSCREngine
{
public:
	virtual TreePtr<Node> GetSearchPattern() const = 0;
	virtual TreePtr<Node> GetReplacePattern() const = 0;
    virtual void ConfigureMyEngine( SCREngine *my_engine ) = 0;
};

class StartsOverlay
{
public:
    virtual void StartPlanOverlay(map<PatternLink, PatternLink> &overlay_plinks) = 0;
};

/// Common implementation for search+replace, compare+replace and slaves
class SCREngine : public virtual GraphIdable,
                  public SerialNumber
{      
public:
    SCREngine( const CompareReplace *overall_master,
               CompareReplace::AgentPhases &in_progress_agent_phases,
               TreePtr<Node> cp,
               TreePtr<Node> rp = TreePtr<Node>(),
               const unordered_set<PatternLink> &master_plinks = unordered_set<PatternLink>(),                            
               const SCREngine *master = nullptr); /* if null, you are overall master */ 
    void PlanningStageTwo( const CompareReplace::AgentPhases &in_progress_agent_phases )
    {
        plan.PlanningStageTwo(in_progress_agent_phases); 
    }
    void PlanningStageThree()
    {
        plan.PlanningStageThree(); 
    }
                    
private:
    struct Plan : public virtual Traceable
    {            
        Plan( SCREngine *algo,
              const CompareReplace *overall_master,
              CompareReplace::AgentPhases &in_progress_agent_phases,
              TreePtr<Node> cp,
              TreePtr<Node> rp,
              const unordered_set<PatternLink> &master_plinks,                            
              const SCREngine *master ); /* if null, you are overall master */ 
        void CategoriseSubs( const unordered_set<PatternLink> &master_plinks, 
                             CompareReplace::AgentPhases &in_progress_agent_phases );
        void WalkVisible( unordered_set<PatternLink> &visible, 
                          PatternLink base_plink, 
                          Agent::Path path ) const;
        void CreateMyEngines( CompareReplace::AgentPhases &in_progress_agent_phases );
        void PlanningStageTwo(const CompareReplace::AgentPhases &in_progress_agent_phases); // Stage one is the constructor
        void ConfigureAgents();
        void PlanningStageThree();
        string GetTrace() const; // used for debug
        
        SCREngine * const algo;
        const CompareReplace *overall_master_ptr;
        TreePtr<Node> root_pattern;
        PatternLink root_plink;
        Agent *root_agent;
        const SCREngine *master_ptr;
        const unordered_set<PatternLink> master_plinks;
        unordered_set<Agent *> master_agents;
        unordered_set<PatternLink> my_plinks;   
        unordered_set<Agent *> my_agents;   
        set<RequiresSubordinateSCREngine *> my_agents_needing_engines;   
        set<StartsOverlay *> my_overlay_starter_engines;   
        map< RequiresSubordinateSCREngine *, shared_ptr<SCREngine> > my_engines;   
        shared_ptr<AndRuleEngine> and_rule_engine;
        CompareReplace::AgentPhases final_agent_phases;   
        map<PatternLink, PatternLink> overlay_plinks; 
    } plan;

    TreePtr<Node> Replace( const CouplingKeysMap *master_keys );

public: // For top level engine/VN trans
    void SingleCompareReplace( TreePtr<Node> *p_root_xnode,
                               const CouplingKeysMap *master_keys );
    int RepeatingCompareReplace( TreePtr<Node> *p_root_xnode,
                                 const CouplingKeysMap *master_keys );                                                                                               
    virtual void SetStopAfter( vector<int> ssa, int d=0 );
    static void SetMaxReps( int n, bool e );
    list<const AndRuleEngine *> GetAndRuleEngines() const;
    list<const SCREngine *> GetSCREngines() const;
    void GenerateGraphRegions( Graph &graph ) const;
    
public: // For agents
    // Note: this is const but RepeatingCompareReplace() isn't. Why?
    // Because we're not calling OUR RepeatingCompareReplace but
    // the slave_engine's one - and that pointer is not const.
    void RecurseInto( SCREngine *slave_engine, 
                      TreePtr<Node> *p_root_xnode ) const;    
    void SetReplaceKey( LocatedLink keyer_link, KeyProducer place ) const;
    CouplingKey GetReplaceKey( const Agent *agent ) const;
    void CopyReplaceKey( PatternLink keyer_plink, PatternLink src_plink, KeyProducer place ) const;
    bool IsKeyedByAndRuleEngine( Agent *agent ) const; 
    const CompareReplace * GetOverallMaster() const;
    XLink UniquifyDomainExtension( XLink xlink ) const;
    
    friend class Conjecture; 

public: // Trace stuff for anyone
    string GetTrace() const; // used for debug
    virtual string GetGraphId() const;    

private:    
    static int repetitions;
    static bool rep_error;
    
    vector<int> stop_after;
    int depth;    
    TheKnowledge knowledge;    
    
    mutable CouplingKeysMap replace_keys;
    mutable unordered_set<PatternLink> my_keyer_plinks_measured;   
    bool keys_available = false;    
};

};
#endif
