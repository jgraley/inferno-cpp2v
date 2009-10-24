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
// implied transformation on programs passed to it.
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
// - NULL shared_ptr (or empty container) in replace pattern under a
//   substituted node means fill this in from substitute too (otherwise
//   use what is specified).
//
// - Identifiers (any node derived from Identifier) are kept unique
//   during replace by pointing directly to the identifier in the 
//   program tree rather than duplicating (only when substituting).
//
// - Soft search pattern nodes may be created which can support custom
//   matching rules by implementing a virtual DecidedCompare() function.
//   Ready made soft nodes are documented in soft_patterns.hpp
//
// - Sequence/Container support: sequences require matching ordering
//   and containers do not (only the presence of the matching elements).
//
// - Multi-node wildcards like * in sequences and containers (Star node).
//
// - TODO Recursive wildcards, arbitrary depth and arbitrary depth with
//   restricted intermediates (the Stuff node).
//
// - TODO slave search/replace so that a second SR can happen for each match
//   of the first one, and can borrow its match sets.
class SearchReplace : Pass
{  
public:
    enum Result { NOT_FOUND = (int)false,
    	          FOUND     = (int)true };

    typedef GenericContainer::iterator Choice;
    class Conjecture : public vector<Choice>
    {
    private:
    	int decisions_count;
    public:
    	void PrepareForDecidedCompare();
    	bool ShouldTryMore( Result r, int threshold );
    	Choice HandleDecision( Choice begin, Choice end );
    };

    // Key for a match set, viewed as an STL-type range.
    struct Key
    {
     	bool keyed; // begin and end only valid if this is true
     	SharedPtr<Node> surrogate_pointer;
     	Choice begin; // inclusive
    	Choice end;   // exclusive
    };

    // Match set - if required, construct a set of these, fill in the set
    // of shared pointers but don't worry about key, pass to SearchReplace constructor.
    struct MatchSet : public set< shared_ptr<Node> >
    {
         mutable Key key_x;    // This is filled in by the search and replace engine
         GenericContainer::iterator GetKeyBegin() const { ASSERT(key_x.keyed); return key_x.begin; }
         GenericContainer::iterator GetKeyEnd() const { ASSERT(key_x.keyed); return key_x.end; }
    };
    struct MatchKeys : set<MatchSet>
    {
    	enum Pass { KEYING, RESTRICTING } pass;
    	MatchKeys( set<MatchSet> &s ) :
    		set<MatchSet>(s)
    	{
    	}
        const MatchSet *FindMatchSet( shared_ptr<Node> node );
        Result KeyAndRestrict( shared_ptr<Node> x,
        		               shared_ptr<Node> pattern,
                               const SearchReplace *sr,
                               unsigned context_flags );
        Result KeyAndRestrict( GenericContainer::iterator x_begin,
        		               GenericContainer::iterator x_end,
        		               shared_ptr<Node> pattern,
        		               const SearchReplace *sr,
        		               unsigned context_flags,
        		               shared_ptr<Node> *x=NULL ); // internal use only
        void CheckMatchSetsKeyed();
        void ClearKeys();
        void SetPass( Pass p ) { pass = p; }
    };
    MatchKeys *matches;

    // The * wildcard can match more than one node of any type in a container
    // In a Sequence, only a contiguous subsequence of 0 or more elements will match
    // In a Collection, a sub-collection of 0 or more elements may be matched anywhere in the collection
    // Only one Star is allowed in a Collection. Star must be templated on a type that is allowed
    // in the collection. TODO if the type is narrower, restrict any matches!!
private:
    struct StarBase : virtual Node { NODE_FUNCTIONS };
public:
    template<class VALUE_TYPE>
    struct Star : private StarBase,
                  VALUE_TYPE { NODE_FUNCTIONS };

    // Constructor and destructor. Search and replace patterns and match sets are 
    // specified here, so that we have a fully confiugured functor.
    SearchReplace( shared_ptr<Node> sp=shared_ptr<Node>(), 
                   shared_ptr<Node> rp=shared_ptr<Node>(),
                   set<MatchSet> *m = NULL );
    void Configure( shared_ptr<Node> sp=shared_ptr<Node>(),
                    shared_ptr<Node> rp=shared_ptr<Node>(),
                    set<MatchSet> *m = NULL );
    ~SearchReplace();
    
    // Do the actual search and replace (functor style; implements Pass interface).
    void operator()( shared_ptr<Program> p );

    // Stuff for soft nodes; support this base class in addition to whatever tree intermediate
    // is required. Call GetProgram() if program root needed; call DecidedCompare() to recurse
    // back into the general search algorithm.
    shared_ptr<Program> GetProgram() const { ASSERT(program); return program; } 
    struct SoftSearchPattern : virtual Node
    {
        virtual SearchReplace::Result DecidedCompare( const SearchReplace *sr,
        		                                      shared_ptr<Node> x,
        		                                      MatchKeys *match_keys,
        		                                      Conjecture &conj,
        		                                      unsigned context_flags ) const = 0;
    };

    // Some self-testing
    static void Test();
        
private:
    shared_ptr<Node> search_pattern;
    shared_ptr<Node> replace_pattern;
    bool our_matches;
    shared_ptr<Program> program;
    
    // LocalCompare ring
    bool LocalCompare( shared_ptr<Node> x,
    		           shared_ptr<Node> pattern ) const;

    // DecidedCompare ring
    Result DecidedCompare( GenericSequence &x,
    		               GenericSequence &pattern,
    		               MatchKeys *match_keys,
    		               Conjecture &conj,
    		               unsigned context_flags ) const;
    Result DecidedCompare( GenericCollection &x,
    		               GenericCollection &pattern,
    		               MatchKeys *match_keys,
    		               Conjecture &conj,
    		               unsigned context_flags ) const;
public:
    Result DecidedCompare( shared_ptr<Node> x,
    		               shared_ptr<Node> pattern,
    		               MatchKeys *match_keys,
    		               Conjecture &conj,
    		               unsigned context_flags ) const;
private:
    // MatchingDecidedCompare ring
    Result MatchingDecidedCompare( shared_ptr<Node> x,
    		                       shared_ptr<Node> pattern,
    		                       MatchKeys *match_keys,
    		                       Conjecture &conj ) const;

    // Compare ring
    Result Compare( shared_ptr<Node> x,
    		        shared_ptr<Node> pattern,
    		        MatchKeys *match_keys,
    		        Conjecture &conj,
    		        int threshold ) const;
    Result Compare( shared_ptr<Node> x,
    		        shared_ptr<Node> pattern,
    		        MatchKeys *match_keys = NULL ) const;
    Result Compare( GenericContainer::iterator x_begin,
                    GenericContainer::iterator x_end,
                    GenericContainer::iterator pattern_begin,
                    GenericContainer::iterator pattern_end,
    		        MatchKeys *match_keys = NULL ) const;

    // Search ring
    bool Search( shared_ptr<Node> program,
    		     GenericContainer::iterator &gp,
    		     MatchKeys *match_keys = NULL ) const;

    // Replace ring
    void ClearPtrs( shared_ptr<Node> dest );
    void Overlay( shared_ptr<Node> dest,
    		      shared_ptr<Node> source,
    		      MatchKeys *match_keys,
    		      bool under_substitution );
    void Overlay( GenericSequence *dest,
    		      GenericSequence *source,
    		      MatchKeys *match_keys,
    		      bool under_substitution );
    void Overlay( GenericCollection *dest,
    	          GenericCollection *source,
    	          MatchKeys *match_keys,
    	          bool under_substitution );
    shared_ptr<Node> DuplicateSubtree( shared_ptr<Node> x,
    		                           MatchKeys *match_keys,
    		                           bool under_substitution=false );
    void Replace( GenericContainer::iterator target,
    		      MatchKeys *match_keys );

    // Internal node classes; need this because the sub-collection matching a star
    // is not generally contiguous, and therefore not a range.
    struct SubCollection : Node,
                           Collection<Node>
    {
    	NODE_FUNCTIONS
    };
};

#endif

