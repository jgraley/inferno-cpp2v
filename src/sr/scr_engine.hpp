#ifndef SCR_ENGINE_HPP
#define SCR_ENGINE_HPP

#include "query.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "common/mismatch.hpp"
#include "agent.hpp"
#include "and_rule_engine.hpp"
#include <set>


/// SR namespace contains the search and replace implementation
namespace SR 
{
class Agent;
class Conjecture;
class SpecialBase;
class CompareReplace;

/// Common implementation for search+replace, compare+replace and slaves
class SCREngine : public virtual Traceable
{      
public:
    SCREngine( bool is_search,
               const CompareReplace *overall_master,
               TreePtr<Node> cp,
               TreePtr<Node> rp = TreePtr<Node>(),
               const set<Agent *> &master_agents = set<Agent *>(),                            
               const SCREngine *master = NULL); /* if null, you are overall master */ 
                    
private:
    virtual void ConfigInstallRootAgents( TreePtr<Node> cp,
										  TreePtr<Node> rp );
    virtual void ConfigCategoriseSubs( const set<Agent *> &master_agents, set<AgentCommonNeedSCREngine *> &my_agents_needing_engines );
    virtual void ConfigCreateMyEngines( const set<Agent *> &master_agents, 
                                        const set<AgentCommonNeedSCREngine *> &my_agents_needing_engines );
    virtual void ConfigConfigureSubs();
    
public:
    void GatherCouplings( CouplingMap *coupling_keys ) const;
    int RepeatingCompareReplace( TreePtr<Node> *proot,
                                 const CouplingMap *master_keys );
    
    void SingleCompareReplace( TreePtr<Node> *proot,
                               const CouplingMap *master_keys );
    void Compare( TreePtr<Node> start_x );

private:
    void KeyReplaceNodes( const CouplingMap *coupling_keys) const;
    TreePtr<Node> Replace() const;

public:
    virtual void GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *blocks ) const;

    static void SetMaxReps( int n, bool e ) { repetitions=n; rep_error=e; }
    const CompareReplace * GetOverallMaster() const;

    //friend class StandardAgent;
    friend class Conjecture;

    virtual void SetStopAfter( vector<int> ssa, int d=0 )
    {
        stop_after = ssa;
        depth = d;
    }
        
private:
    const bool is_search;
    
    const CompareReplace *overall_master_ptr;
    TreePtr<Node> pattern;
    Agent *root_agent;
    const SCREngine *master_ptr;
    std::shared_ptr< set<Agent *> > my_agents;   
    std::map<AgentCommonNeedSCREngine *, SCREngine> my_engines;   
    
    static int repetitions;
    static bool rep_error;
    
    vector<int> stop_after;
    int depth;    

    shared_ptr<AndRuleEngine> and_rule_engine;

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
