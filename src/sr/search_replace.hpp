#ifndef SEARCH_REPLACE_HPP
#define SEARCH_REPLACE_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include <set>

/// SR namespace contains the search and replace implementation
namespace SR 
{
class Agent;
class Conjecture;
class SpecialBase;
class SCREngine;

/// Main implementation of a compare and replace function, that matches at the root of the program tree
class CompareReplace : public virtual Traceable,
                       virtual public InPlaceTransformation
{  
public:
    // Constructor and destructor. Search and replace patterns and couplings are
    // specified here, so that we have a fully confiugured functor.
    CompareReplace( TreePtr<Node> cp = TreePtr<Node>(),
                    TreePtr<Node> rp = TreePtr<Node>(),
                    bool is_search=false );
                    
    virtual void Configure( TreePtr<Node> cp,
                            TreePtr<Node> rp = TreePtr<Node>() );
    static void SetMaxReps( int n, bool e );
                            
    using Transformation::operator();
    
    // Functor style interface for RepeatingSearchReplace; implements Pass interface.
    void operator()( TreePtr<Node> context, 
                     TreePtr<Node> *proot );

    // Stuff for soft nodes; support this base class in addition to whatever tree intermediate
    // is required. Call GetProgram() if program root needed; call DecidedCompare() to recurse
    // back into the general search algorithm.
    TreePtr<Node> GetContext() const { ASSERT(pcontext && *pcontext); return *pcontext; }
    mutable set< TreePtr<Node> > dirty_grass;    
    virtual void GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *blocks ) const;    
    virtual void SetStopAfter( vector<int> ssa, int d=0 );
    SCREngine *GetRootEngine();
    
private:
    TreePtr<Node> *pcontext;
    const bool is_search;
    shared_ptr<SCREngine> scr_engine;
};


/// Main implementation of a search and replace function, that matches anywhere in program tree
class SearchReplace : public CompareReplace
{
public:
    SearchReplace( TreePtr<Node> sp = TreePtr<Node>(),
                   TreePtr<Node> rp = TreePtr<Node>() );
};

};


#endif

