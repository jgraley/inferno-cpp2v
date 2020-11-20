#ifndef SCR_ENGINE_HPP
#define SCR_ENGINE_HPP

#include "query.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "common/mismatch.hpp"
#include <set>


/// SR namespace contains the search and replace implementation
namespace SR 
{
class Agent;
class AgentCommonNeedSCREngine;
class Conjecture;
class SpecialBase;
class CompareReplace;
class AndRuleEngine;
class QuotientSet;

/// Common implementation for search+replace, compare+replace and slaves
class SCREngine : public virtual Traceable
{      
public:
    SCREngine( bool is_search,
               const CompareReplace *overall_master,
               TreePtr<Node> cp,
               TreePtr<Node> rp = TreePtr<Node>(),
               const unordered_set<Agent *> &master_agents = unordered_set<Agent *>(),                            
               const SCREngine *master = nullptr); /* if null, you are overall master */ 
                    
private:
    const struct Plan : public virtual Traceable
    {            
        Plan( SCREngine *algo,
              bool is_search,
              const CompareReplace *overall_master,
              TreePtr<Node> cp,
              TreePtr<Node> rp,
              const unordered_set<Agent *> &master_agents,                            
              const SCREngine *master ); /* if null, you are overall master */ 
        void InstallRootAgents( TreePtr<Node> cp,
                                TreePtr<Node> rp );
        void CategoriseSubs( const unordered_set<Agent *> &master_agents, 
                             set<AgentCommonNeedSCREngine *> &my_agents_needing_engines );
        void CreateMyEngines( const unordered_set<Agent *> &master_agents,                       
                              const set<AgentCommonNeedSCREngine *> &my_agents_needing_engines );
        void ConfigureAgents();

        SCREngine * const algo;
        const bool is_search;    
        const CompareReplace *overall_master_ptr;
        TreePtr<Node> root_pattern;
        PatternLink root_plink;
        Agent *root_agent;
        const SCREngine *master_ptr;
        std::shared_ptr< set<Agent *> > my_agents;   
        std::shared_ptr< set<PatternLink> > my_agent_links;   
        std::map< AgentCommonNeedSCREngine *, shared_ptr<SCREngine> > my_engines;   
        shared_ptr<AndRuleEngine> and_rule_engine;
    } plan;
public:
    void GatherCouplings( CouplingKeysMap *coupling_keys ) const;    
    void ExtendDomain( PatternLink plink, set<XLink> &domain );
    void DetermineDomain( XLink root_xlink );
    void SingleCompareReplace( TreePtr<Node> *p_root_xnode,
                               const CouplingKeysMap *master_keys );
    int RepeatingCompareReplace( TreePtr<Node> *p_root_xnode,
                                 const CouplingKeysMap *master_keys );                              
private:
    void KeyReplaceNodes( const CouplingKeysMap *coupling_keys) const;
    TreePtr<Node> Replace() const;

public:
    virtual void GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *blocks ) const;

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
    
    set<XLink> domain;    
    shared_ptr<QuotientSet> domain_extension_classes;

	/** Walks the tree, avoiding the "search"/"compare" and "replace" members of slaves
		but still recurses through the "through" member. Therefore, it visits all the
		nodes under the same engine as the root. Based on UniqueWalk, so each node only
		visited once. */
	class VisibleWalk_iterator : public UniqueWalk::iterator
	{
	public:
		VisibleWalk_iterator( TreePtr<Node> &root ) : UniqueWalk::iterator(root) {}        
		VisibleWalk_iterator() : UniqueWalk::iterator() {}
		virtual shared_ptr<ContainerInterface::iterator_interface> Clone() const;
	private:
		virtual shared_ptr<ContainerInterface> GetChildContainer( TreePtr<Node> n ) const;
	};

	typedef ContainerFromIterator< VisibleWalk_iterator, TreePtr<Node> > VisibleWalk;       
};

};
#endif
