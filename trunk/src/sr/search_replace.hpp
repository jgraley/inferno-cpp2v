#ifndef SEARCH_REPLACE_HPP
#define SEARCH_REPLACE_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "helpers/walk.hpp"
#include "helpers/transformation.hpp"
#include "agent.hpp"
#include "engine.hpp"
#include <set>

/// SR namespace contains the search and replace implementation
namespace SR 
{
class Agent;
class Conjecture;
class SpecialBase;

/// Main implementation of a compare and replace function, that matches at the root of the program tree
class CompareReplace : public Engine,
                       virtual public InPlaceTransformation, 
                       public Filter 
{  
public:
    // Constructor and destructor. Search and replace patterns and couplings are
    // specified here, so that we have a fully confiugured functor.
    CompareReplace( TreePtr<Node> cp = TreePtr<Node>(),
                    TreePtr<Node> rp = TreePtr<Node>(),
                    bool search=false );
                    
    
    using Transformation::operator();
    // Functor style interface for RepeatingSearchReplace; implements Pass interface.
    void operator()( TreePtr<Node> context, 
                     TreePtr<Node> *proot );

    // Stuff for soft nodes; support this base class in addition to whatever tree intermediate
    // is required. Call GetProgram() if program root needed; call DecidedCompare() to recurse
    // back into the general search algorithm.
    TreePtr<Node> GetContext() const { ASSERT(pcontext&&*pcontext); return *pcontext; }
    TreePtr<Node> *pcontext;
    mutable Set< TreePtr<Node> > dirty_grass;    
    virtual void GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *blocks ) const;    
    friend class Conjecture;
    virtual bool IsMatch( TreePtr<Node> context,       
                          TreePtr<Node> root );
    virtual void SetStopAfter( vector<int> ssa, int d=0 )
    {
        Engine::SetStopAfter( ssa, d );
    }
};


/// Main implementation of a search and replace function, that matches anywhere in program tree
class SearchReplace : public CompareReplace
{
public:
    SearchReplace( TreePtr<Node> sp = TreePtr<Node>(),
                   TreePtr<Node> rp = TreePtr<Node>() );
};

};

// Legacy clients of SR have assumed that all these includes come from
// search_replace.hpp. This would not be so under the agents scheme. 
// We compromise by putting them here. Putting them after the code clarifies
// that there is no dependency; just the will to be nice to client code.
#include "standard_agent.hpp"
#include "search_container_agent.hpp"
#include "star_agent.hpp"
#include "green_grass_agent.hpp"
#include "overlay_agent.hpp"
#include "slave_agent.hpp"
#include "boolean_agents.hpp"
#include "pointer_is_agent.hpp"
#include "transform_of_agent.hpp"

#endif

