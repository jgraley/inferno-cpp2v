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
    int RepeatingCompareReplace( TreePtr<Node> *proot );
    
protected:
    bool SingleCompareReplace( TreePtr<Node> *proot );
    bool Compare( const TreePtrInterface &x ) const;
    void KeyReplaceNodes() const;
    TreePtr<Node> Replace() const;

public:
    virtual void GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *links ) const;

    static void SetMaxReps( int n, bool e ) { repetitions=n; rep_error=e; }
    const CompareReplace * GetOverallMaster() const;

    //friend class NormalAgent;
    friend class Conjecture;
    
private:
    bool is_search;
    TreePtr<Node> pattern;
    Agent *root_agent;
    const Engine *master_ptr;
    Set<Engine *> my_engines;   
    Set<Agent *> my_agents;   
    
    static int repetitions;
    static bool rep_error;
};

};
#endif