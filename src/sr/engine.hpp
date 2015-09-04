#ifndef ENGINE_HPP
#define ENGINE_HPP

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
public:
    // Constructor and destructor. Search and replace patterns and couplings are
    // specified here, so that we have a fully confiugured functor.
    Engine( bool is_s );
                    
    // Call this to set the patterns after construction. This should not be virtual since
    // the constructor calls it.
    virtual void Configure( TreePtr<Node> cp,
                            TreePtr<Node> rp = TreePtr<Node>(),
                            const Set<Agent *> &agents_already_configured = Set<Agent *>(),
                            const Engine *master = NULL /* if null, IT'S YOU!!!! */ );

protected:
    // A configure that doesn't know what the search and replace patterns are
    virtual void Configure( const Set<Agent *> &agents_already_configured,
                            const Engine *master /* if null, IT'S YOU!!!! */ );
    
public:
    void GatherCouplings( CouplingMap &coupling_keys ) const;
    int RepeatingCompareReplace( TreePtr<Node> *proot,
                                 const CouplingMap &initial_coupling_keys );
    
protected:
    bool SingleCompareReplace( TreePtr<Node> *proot,
                               const CouplingMap &initial_coupling_keys );
    bool Compare( const TreePtrInterface &x ) const;
    bool Compare( const TreePtrInterface &x,
                  Conjecture &conj,
                  CouplingMap &matching_coupling_keys,
                  const CouplingMap &initial_coupling_keys ) const;
    bool DecidedCompare( Agent *agent,
                         const TreePtrInterface &x,
                         bool can_key,
                         Conjecture &conj,
                         CouplingMap &coupling_keys ) const;
    bool AbnormalCompare( Agent *agent,
                          const TreePtrInterface &x, 
                          bool can_key,
                          CouplingMap &coupling_keys ) const;
    void KeyReplaceNodes( Conjecture &conj,
                          const CouplingMap &coupling_keys) const;
    TreePtr<Node> Replace() const;

public:
    virtual void GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *links ) const;

    static void SetMaxReps( int n, bool e ) { repetitions=n; rep_error=e; }
    const CompareReplace * GetOverallMaster() const;

    //friend class NormalAgent;
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
};

};
#endif
