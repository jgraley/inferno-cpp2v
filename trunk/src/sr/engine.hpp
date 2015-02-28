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

/// Main implementation of a compare and replace function, that matches at the root of the program tree
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
                            const Set<Agent *> &agents_already_configured=Set<Agent *>() );
    
    TreePtr<Node> pattern;
    const Engine *master_ptr;
    Set<Agent *> my_agents;
    
    virtual void GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *links ) const;

    static void SetMaxReps( int n, bool e ) { repetitions=n; rep_error=e; }
    const CompareReplace * GetOverallMaster() const;

private:
    static int repetitions;
    static bool rep_error;
    bool is_search;
private:

    friend class Conjecture;
public:
    bool Compare( const TreePtrInterface &x,
                  TreePtr<Node> pattern ) const;
public:
    TreePtr<Node> BuildReplace( TreePtr<Node> pattern ) const;
private:
    void KeyReplaceNodes( TreePtr<Node> pattern ) const;
    TreePtr<Node> ReplacePhase( TreePtr<Node> x ) const;

    bool SingleCompareReplace( TreePtr<Node> *proot );
public:
    int RepeatingCompareReplace( TreePtr<Node> *proot );
    
    friend class NormalAgent;
};

};
#endif