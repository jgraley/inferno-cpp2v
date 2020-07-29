#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "query.hpp"
#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "agent.hpp"
#include <set>

/// SR namespace contains the search and replace implementation
namespace SR 
{
class Agent;
class SlaveAgent;
class Conjecture;
class SpecialBase;
class CompareReplace;

/// Common implementation for search+replace, compare+replace and slaves
class Engine : public virtual Traceable
{  
private:
    struct CompareState
    {
        bool can_key;
        Conjecture *conj;
        CouplingMap *my_keys; // applies ACROSS PASSES
        const CouplingMap *master_keys;
        Set<Agent *> reached; // applies to CURRENT PASS only
    };
    
public:
    Engine( bool is_s );
                    
    // Call this to set the patterns after construction. This should not be virtual since
    // the constructor calls it.
    virtual void Configure( TreePtr<Node> cp,
                            TreePtr<Node> rp = TreePtr<Node>(),
                            const Set<Agent *> &master_agents = Set<Agent *>(),
                            const Engine *master = NULL /* if null, IT'S YOU!!!! */ );

protected:
    // A configure that doesn't know what the search and replace patterns are
    virtual void Configure( const Set<Agent *> &master_agents,
                            const Engine *master /* if null, IT'S YOU!!!! */ );
private:
    virtual void ConfigInstallRootAgents( TreePtr<Node> cp,
										  TreePtr<Node> rp );
    virtual void ConfigCategoriseSubs( const Set<Agent *> &master_agents );
    virtual void ConfigConfigureSubs( const Set<Agent *> &master_agents );

protected: 
    virtual shared_ptr<ContainerInterface> GetVisibleChildren() const { ASSERTFAIL(); } 
    
public:
    void GatherCouplings( CouplingMap *coupling_keys ) const;
    int RepeatingCompareReplace( TreePtr<Node> *proot,
                                 const CouplingMap *master_keys );
    
protected:
    bool SingleCompareReplace( TreePtr<Node> *proot,
                               const CouplingMap *master_keys );
    bool Compare( const TreePtrInterface *p_start_x,
                  const CouplingMap *master_keys = nullptr ) const;
    bool Compare( Agent *start_agent,
                  const TreePtrInterface *p_start_x,
                  const CouplingMap *master_keys ) const;
    bool Compare( Agent *start_agent,
                  const TreePtrInterface *p_start_x,
                  Conjecture *conj,
                  CouplingMap *my_keys,
                  const CouplingMap *master_keys ) const;
private:
    bool DecidedCompare( Agent *agent,
                         const TreePtrInterface *px,
                         CompareState &state ) const;
    bool CompareLinks( shared_ptr<const AgentQuery> query,
                       CompareState &state ) const;
    bool CompareEvaluatorLinks( shared_ptr<const AgentQuery> query,
							    const CouplingMap *my_keys ) const;
    void KeyReplaceNodes( Conjecture &conj,
                          const CouplingMap *coupling_keys) const;
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
    bool is_search;
    TreePtr<Node> pattern;
    Agent *root_agent;
    const Engine *master_ptr;
    Set<SlaveAgent *> my_slaves;   
    Set<Agent *> my_agents;   
    
    static int repetitions;
    static bool rep_error;
    
    vector<int> stop_after;
    int depth;    

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
