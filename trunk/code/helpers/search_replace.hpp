#ifndef SEARCH_REPLACE_HPP
#define SEARCH_REPLACE_HPP

#include "tree/tree.hpp"
#include "common/refcount.hpp"
#include "common/common.hpp"
//#include "parse/parse.hpp"  
#include "render/render.hpp" // TODO remove this silly dependency
#include "common/read_args.hpp"
#include "walk.hpp"
#include "pass.hpp"
#include <set>

// In-tree search and replace utility. To use, you make a search pattern and a 
// replace pattern, each in the form of a subtree. You pass these into the 
// constructor and you now have a pass object (functor) that will apply the 
// implied trasformation on programs passed to it.
// 
// Additional functionality as follows:
// - Intermediate tree nodes like Numeric or Expression can be placed
//   in search pattern, in which case they act as wildcards matching
//   any subclass node (think of set-theory interpretation of inheritance)
//
// - A NULL shared_ptr is also a wildcard for anything.
//
// - Multiple nodes in the search pattern can be forced to match the same 
//   program node if a MatchSet is created that points to these nodes, and
//   passed to the constructor.
//
// - Nodes in the replace pattern may be substituted by program nodes
//   found during matching by creating a MatchSet and inserting pointers 
//   to the corresponding search and replace pattern nodes.
//
// - NULL shared_ptr in replace pattern under a substituted node means
//  fill this in from substitute too (otherwise use what is specified).
//
// - Identifiers (any node derived from Identifier) are kept unique
//   during replace by pointing directly to the identifier in the 
//   program tree rather than duplicating (only when substituting).
//
// - Soft search pattern nodes may be created which can support custom
//   matching rules by implementing a virtual IsMatchPattern() function.
//   Ready made soft nodes are documented in soft_patterns.hpp
//
// - TODO multi-node wildcards for * in sequences, arbitrary depth
//   and arbitrary depth with restricted intermediates (the Stuff node).
//
// - TODO slave search/replace so that a second SR can happen for each match
//   of the first one, and can borrow its match sets.
class SearchReplace : Pass
{  
public:
    // Match set - if required, construct a set of these, fill in the set
    // of shared pointers but don't worry about key, pass to SearchReplace constructor. 
    class MatchSet : public set< shared_ptr<Node> > 
    { 
        mutable shared_ptr<Node> key;    // This is filled in by the search and replace engine
        friend class SearchReplace;
    };

    // Constructor and destructor. Search and replace patterns and match sets are 
    // specified here, so that we have a fully confiugured functor.
    SearchReplace( shared_ptr<Node> sp=shared_ptr<Node>(), 
                   shared_ptr<Node> rp=shared_ptr<Node>(),
                   const set<MatchSet> *m = NULL );    
    ~SearchReplace();
    
    // Do the actual search and replace (functor style; implements Pass interface).
    void operator()( shared_ptr<Program> p );

    // Stuff for soft nodes; support this base class in addition to whatever tree intermediate
    // is required. Call GetProgram() if program root needed; call IsMatchPattern() to recurse 
    // back into the general search algorithm.
    bool IsMatchPattern( shared_ptr<Node> x, shared_ptr<Node> pattern ); // look at node and children
    shared_ptr<Program> GetProgram() const { ASSERT(program); return program; } 
    struct SoftSearchPattern : virtual Node
    {
        virtual bool IsMatchPattern( SearchReplace *sr, shared_ptr<Node> x ) = 0; 
    };

    // Some self-testing
    static void Test();
        
private:
    shared_ptr<Node> search_pattern;
    shared_ptr<Node> replace_pattern;
    const set<MatchSet> *matches;
    bool our_matches;
    shared_ptr<Program> program;
    
    bool IsMatchPatternLocal( shared_ptr<Node> x, shared_ptr<Node> pattern ); // only look inside node (type, value)
    bool IsMatchPatternNoKey( shared_ptr<Node> x, shared_ptr<Node> pattern ); // look at node and children
    bool IsMatchPattern( GenericSequence &x, GenericSequence &pattern ); // match for Sequences
    bool IsMatchPattern( GenericCollection &x, GenericCollection &pattern ); // match for Collections
    bool Search( shared_ptr<Node> program, GenericContainer::iterator &gp );
    void ClearPtrs( shared_ptr<Node> dest );
    void OverlayPtrs( shared_ptr<Node> dest, shared_ptr<Node> source, bool under_substitution );
    shared_ptr<Node> DuplicateSubtree( shared_ptr<Node> x, bool under_substitution=false );
    void Replace( GenericContainer::iterator target );    
    const MatchSet *FindMatchSet( shared_ptr<Node> node );
    void ClearKeys(); 
};



#endif
