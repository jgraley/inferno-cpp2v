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
	virtual bool IsSearch() const = 0;
	virtual TreePtr<Node> GetSearchPattern() const = 0;
	virtual TreePtr<Node> GetReplacePattern() const = 0;
    virtual void ConfigureMyEngine( SCREngine *my_engine ) = 0;
};

class StartsOverlay
{
public:
    virtual void StartKeyForOverlay() = 0;
};

/// Common implementation for search+replace, compare+replace and slaves
class SCREngine : public virtual Graphable,
                  public SerialNumber
{      
public:
    SCREngine( bool is_search,
               const CompareReplace *overall_master,
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
              bool is_search,
              const CompareReplace *overall_master,
              CompareReplace::AgentPhases &in_progress_agent_phases,
              TreePtr<Node> cp,
              TreePtr<Node> rp,
              const unordered_set<PatternLink> &master_plinks,                            
              const SCREngine *master ); /* if null, you are overall master */ 
        void PlanningStageTwo(const CompareReplace::AgentPhases &in_progress_agent_phases); // Stage one is the constructor
        void PlanningStageThree();
        void InstallRootAgents( TreePtr<Node> cp,
                                TreePtr<Node> rp );
        void CategoriseSubs( const unordered_set<PatternLink> &master_plinks, 
                             CompareReplace::AgentPhases &in_progress_agent_phases );
        void WalkVisible( unordered_set<PatternLink> &visible, 
                          PatternLink base_plink, 
                          Agent::Path path ) const;
        void CreateMyEngines( CompareReplace::AgentPhases &in_progress_agent_phases );
        void ConfigureAgents();
        string GetTrace() const; // used for debug
        
        SCREngine * const algo;
        const bool is_search;    
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
    } plan;
public:
    // Note: this is const but RepeatingCompareReplace(). Why?
    // Because we're not calling OUR RepeatingCompareReplace but
    // the slave_engine's one - and that pointer is not const.
    void RecurseInto( SCREngine *slave_engine, 
                      TreePtr<Node> *p_root_xnode ) const;    
    void SingleCompareReplace( TreePtr<Node> *p_root_xnode,
                               const CouplingKeysMap *master_keys );
    int RepeatingCompareReplace( TreePtr<Node> *p_root_xnode,
                                 const CouplingKeysMap *master_keys );                   
                                            
private:
    TreePtr<Node> Replace( const CouplingKeysMap *master_keys );

public:
    static void SetMaxReps( int n, bool e ) { repetitions=n; rep_error=e; }
    const CompareReplace * GetOverallMaster() const;

    //friend class StandardAgent;
    friend class Conjecture;

    virtual void SetStopAfter( vector<int> ssa, int d=0 );
    XLink UniquifyDomainExtension( XLink xlink ) const;
    string GetTrace() const; // used for debug
    
    virtual Block GetGraphBlockInfo( const LinkNamingFunction &lnf,
                                     const NonTrivialPreRestrictionFunction &ntprf ) const;
    virtual string GetGraphId() const;
    
    list<const AndRuleEngine *> GetAndRuleEngines() const;
    list<const SCREngine *> GetSCREngines() const;
    void GenerateGraphRegions( Graph &graph ) const;

    void SetReplaceKey( const Agent *agent, CouplingKey key ) const;
    CouplingKey GetReplaceKey( const Agent *agent ) const;
    void CopyReplaceKey( const Agent *dest_agent, const Agent *src_agent ) const;
    
private:    
    static int repetitions;
    static bool rep_error;
    
    vector<int> stop_after;
    int depth;    
    TheKnowledge knowledge;    
    
    mutable CouplingKeysMap replace_keys;
    bool keys_available = false;
};

};
#endif
