#ifndef SCR_ENGINE_HPP
#define SCR_ENGINE_HPP

#include "query.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
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
class AgentCommonNeedSCREngine;
class Conjecture;
class SpecialBase;
class AndRuleEngine;

/// Common implementation for search+replace, compare+replace and slaves
class SCREngine : public virtual Graphable
{      
public:
    SCREngine( bool is_search,
               const CompareReplace *overall_master,
               CompareReplace::AgentPhases &agent_phases,
               TreePtr<Node> cp,
               TreePtr<Node> rp = TreePtr<Node>(),
               const unordered_set<Agent *> &master_agents = unordered_set<Agent *>(),                            
               const SCREngine *master = nullptr); /* if null, you are overall master */ 
    void InitPartTwo( const CompareReplace::AgentPhases &agent_phases )
    {
        plan.InitPartTwo(agent_phases); 
    }
                    
private:
    struct Plan : public virtual Traceable
    {            
        Plan( SCREngine *algo,
              bool is_search,
              const CompareReplace *overall_master,
              CompareReplace::AgentPhases &agent_phases,
              TreePtr<Node> cp,
              TreePtr<Node> rp,
              const unordered_set<Agent *> &master_agents,                            
              const SCREngine *master ); /* if null, you are overall master */ 
        void InitPartTwo(const CompareReplace::AgentPhases &agent_phases); // Part one is the constructor
        void InstallRootAgents( TreePtr<Node> cp,
                                TreePtr<Node> rp );
        void CategoriseSubs( const unordered_set<Agent *> &master_agents, 
                             set<AgentCommonNeedSCREngine *> &my_agents_needing_engines,
                             CompareReplace::AgentPhases &agent_phases );
        void CreateMyEngines( const unordered_set<Agent *> &master_agents,                       
                              const set<AgentCommonNeedSCREngine *> &my_agents_needing_engines,
                              CompareReplace::AgentPhases &agent_phases );
        void ConfigureAgents(const CompareReplace::AgentPhases &agent_phases);

        SCREngine * const algo;
        const bool is_search;    
        const CompareReplace *overall_master_ptr;
        TreePtr<Node> root_pattern;
        PatternLink root_plink;
        Agent *root_agent;
        const SCREngine *master_ptr;
        const unordered_set<Agent *> master_agents;
        shared_ptr< unordered_set<Agent *> > my_agents;   
        shared_ptr< unordered_set<PatternLink> > my_agent_links;   
        map< AgentCommonNeedSCREngine *, shared_ptr<SCREngine> > my_engines;   
        shared_ptr<AndRuleEngine> and_rule_engine;
    } plan;
public:
    void GatherCouplings( CouplingKeysMap *coupling_keys ) const;    
    void SingleCompareReplace( TreePtr<Node> *p_root_xnode,
                               const CouplingKeysMap *master_keys );
    int RepeatingCompareReplace( TreePtr<Node> *p_root_xnode,
                                 const CouplingKeysMap *master_keys );                              
private:
    void KeyReplaceNodes( const CouplingKeysMap *coupling_keys) const;
    TreePtr<Node> Replace() const;

public:
    virtual Block GetGraphBlockInfo() const;

    static void SetMaxReps( int n, bool e ) { repetitions=n; rep_error=e; }
    const CompareReplace * GetOverallMaster() const;

    //friend class StandardAgent;
    friend class Conjecture;

    virtual void SetStopAfter( vector<int> ssa, int d=0 );
    XLink UniquifyDomainExtension( XLink xlink ) const;
        
private:    
    static int repetitions;
    static bool rep_error;
    
    vector<int> stop_after;
    int depth;    
    TheKnowledge knowledge;
    
	/** Walks the tree, avoiding the "search"/"compare" and "replace" members of slaves
		but still recurses through the "through" member. Therefore, it visits all the
		nodes under the same engine as the root. Based on UniqueWalk, so each node only
		visited once. Restrict according to visibility category v. Note: setting v to
        IN_COMPARE_AND_REPLACE may not work as expected because couplings will be missed. */
	class VisibleWalk_iterator : public UniqueWalk::iterator
	{
	public:
		VisibleWalk_iterator( TreePtr<Node> &root, Agent::Path v_ ) : UniqueWalk::iterator(root), v(v_) {}        
		VisibleWalk_iterator() : UniqueWalk::iterator() {}
		virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const;
	private:
		virtual shared_ptr<ContainerInterface> GetChildContainer( TreePtr<Node> n ) const;
        Agent::Path v;
	};

	typedef ContainerFromIterator< VisibleWalk_iterator, TreePtr<Node>, Agent::Path > VisibleWalk;       
};

};
#endif
