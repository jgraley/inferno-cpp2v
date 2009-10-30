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
// - Recursive wildcards, arbitrary depth and arbitrary depth with
//   restricted intermediates (the Stuff node).
//
// - TODO slave search/replace so that a second SR can happen for each match
//   of the first one, and can borrow its match sets.
class RootedSearchReplace : Pass
{  
public:
    // The * wildcard can match more than one node of any type in a container
    // In a Sequence, only a contiguous subsequence of 0 or more elements will match
    // In a Collection, a sub-collection of 0 or more elements may be matched anywhere in the collection
    // Only one Star is allowed in a Collection. Star must be templated on a type that is allowed
    // in the collection.
    struct StarBase : virtual Node { NODE_FUNCTIONS };
    template<class VALUE_TYPE>
    struct Star : StarBase, VALUE_TYPE { NODE_FUNCTIONS };

    // The Stuff wildcard can match a truncated subtree with special powers as listed by the members
    struct StuffBase : virtual Node
    {
    	NODE_FUNCTIONS;
    	SharedPtr<Node> restrictor; // Restricts the intermediate nodes in the truncated subtree
    	SharedPtr<Node> terminus; // A node somewhere under Stuff, that matches normally, truncating the subtree
    };
    template<class VALUE_TYPE>
    struct Stuff : StuffBase, VALUE_TYPE {	NODE_FUNCTIONS };

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

    // Base class for match set keys; this deals with individual node matches, and also with stars
    // by means of pointing "root" at a SubCollection or SubSequence
    struct Key
    {
    	virtual ~Key(){}  // be a virtual hierarchy
    	SharedPtr<Node> root; // Tree node at matched pattern; root of any replace subtree
    };

    struct StuffKey : Key
    {
     	SharedPtr<StuffBase> search_stuff;
     	SharedPtr<StuffBase> replace_stuff;
     	SharedPtr<Node> terminus;
    };

    // Match set - if required, construct a set of these, fill in the set
    // of shared pointers but don't worry about key, pass to RootedSearchReplace constructor.
    struct MatchSet : public set< shared_ptr<Node> >
    {
         mutable shared_ptr<Key> key;    // This is filled in by the search and replace engine
    };
    struct MatchKeys : set<MatchSet *>
    {
    	enum Pass { KEYING, RESTRICTING, DUPLICATING } pass;
    	MatchKeys( set<MatchSet *> &s ) :
    		set<MatchSet *>(s)
    	{
    	}
    	MatchKeys()
    	{
    	}
        const MatchSet *FindMatchSet( shared_ptr<Node> node );
        Result KeyAndRestrict( shared_ptr<Node> x,
        		               shared_ptr<Node> pattern,
                               const RootedSearchReplace *sr,
                               unsigned context_flags );
        Result KeyAndRestrict( shared_ptr<Key> key,
        		               shared_ptr<Node> pattern,
                               const RootedSearchReplace *sr,
                               unsigned context_flags );
        shared_ptr<Node> KeyAndDuplicate( shared_ptr<Node> x, // source after soft nodes etc
        		                          shared_ptr<Node> pattern, // source
                                          const RootedSearchReplace *sr );
        shared_ptr<Node> KeyAndDuplicate( shared_ptr<Key> key,
        		                          shared_ptr<Node> pattern,
                                          const RootedSearchReplace *sr );
        void CheckMatchSetsKeyed();
        void ClearKeys();
        void SetPass( Pass p ) { pass = p; }
    };
    MatchKeys matches;


    // Constructor and destructor. Search and replace patterns and match sets are 
    // specified here, so that we have a fully confiugured functor.
    RootedSearchReplace( shared_ptr<Node> sp=shared_ptr<Node>(),
                   shared_ptr<Node> rp=shared_ptr<Node>(),
                   set<MatchSet *> m = set<MatchSet *>() );
    void Configure( shared_ptr<Node> sp=shared_ptr<Node>(),
                    shared_ptr<Node> rp=shared_ptr<Node>(),
                    set<MatchSet *> m = set<MatchSet *>() );
    ~RootedSearchReplace();
    
    // Do the actual search and replace (functor style; implements Pass interface).
    Result SingleSearchReplace( shared_ptr<Program> p );
    void operator()( shared_ptr<Program> p );

    // Stuff for soft nodes; support this base class in addition to whatever tree intermediate
    // is required. Call GetProgram() if program root needed; call DecidedCompare() to recurse
    // back into the general search algorithm.
    shared_ptr<Program> GetProgram() const { ASSERT(program); return program; } 
    struct SoftSearchPattern : virtual Node
    {
        virtual RootedSearchReplace::Result DecidedCompare( const RootedSearchReplace *sr,
        		                                      shared_ptr<Node> x,
        		                                      MatchKeys *match_keys,
        		                                      Conjecture &conj,
        		                                      unsigned context_flags ) const = 0;
    };
    struct SoftReplacePattern : virtual Node
    {
        virtual shared_ptr<Node> DuplicateSubtree( const RootedSearchReplace *sr,
        		                                   MatchKeys *match_keys ) = 0;
    };

    // Some self-testing
    static void Test();
        
private:
    shared_ptr<Node> search_pattern;
    shared_ptr<Node> replace_pattern;
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
    Result DecidedCompare( shared_ptr<Node> x,
    		               shared_ptr<StuffBase> stuff_pattern,
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

    // Replace ring
    void ClearPtrs( shared_ptr<Node> dest ) const;
    void Overlay( shared_ptr<Node> dest,
    		      shared_ptr<Node> source,
    		      MatchKeys *match_keys,
    		      shared_ptr<Key> current_key ) const; // under substitution if not NULL
    void Overlay( GenericSequence *dest,
    		      GenericSequence *source,
    		      MatchKeys *match_keys,
    		      shared_ptr<Key> current_key ) const;
    void Overlay( GenericCollection *dest,
    	          GenericCollection *source,
    	          MatchKeys *match_keys,
    	          shared_ptr<Key> current_key ) const;
public:
    shared_ptr<Node> DuplicateSubtree( shared_ptr<Node> x,
    		                           MatchKeys *match_keys,
    		                           shared_ptr<Key> current_key=shared_ptr<Key>() ) const;
private:

    // Internal node classes
    struct SubSequence : Node,
                         Sequence<Node>
    {
    	NODE_FUNCTIONS
    };
    struct SubCollection : Node,
                           Collection<Node>
    {
    	NODE_FUNCTIONS
    };
};


class SearchReplace : public RootedSearchReplace
{
public:
    SearchReplace( shared_ptr<Node> sp = shared_ptr<Node>(),
                   shared_ptr<Node> rp = shared_ptr<Node>(),
                   set<MatchSet *> m = set<MatchSet *>() );
    void Configure( shared_ptr<Node> sp = shared_ptr<Node>(),
                    shared_ptr<Node> rp = shared_ptr<Node>(),
                    set<MatchSet *> m = set<MatchSet *>() );
private:
	MatchSet root_match;
};

#endif

