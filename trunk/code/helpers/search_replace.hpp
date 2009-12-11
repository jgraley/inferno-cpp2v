#ifndef SEARCH_REPLACE_HPP
#define SEARCH_REPLACE_HPP

#include "common/refcount.hpp"
#include "common/common.hpp"
//#include "parse/parse.hpp"  
//#include "render/render.hpp" // TODO remove this silly dependency
#include "common/read_args.hpp"
#include "walk.hpp"
#include "transformation.hpp"
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
//   program node if a Coupling is created that points to these nodes, and
//   passed to the constructor.
//
// - Nodes in the replace pattern may be substituted by program nodes
//   found during matching by creating a Coupling and inserting pointers
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
// - Slave search/replace so that a second SR can happen for each match
//   of the first one, and can borrow its match sets.
class RootedSearchReplace : Transformation
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
    struct Stuff : StuffBase, VALUE_TYPE { NODE_FUNCTIONS };

    enum Result { NOT_FOUND = (int)false,
    	          FOUND     = (int)true };

    // Base class for match set keys; this deals with individual node matches, and also with stars
    // by means of pointing "root" at a SubCollection or SubSequence
    struct Key
    {
    	virtual ~Key(){}  // be a virtual hierarchy
    	SharedPtr<Node> root; // Tree node at matched pattern; root of any replace subtree
    	SharedPtr<Node> replace_pattern; // Tree node at matched pattern; root of any replace subtree
    };

    struct StuffKey : Key
    {
     	SharedPtr<StuffBase> search_stuff; // TODO add search_pattern to Key base class and lose this (as with replace_pattern)
     	SharedPtr<Node> terminus;
    };

    struct Coupling : public set< shared_ptr<Node> >
    {
    };
    class CouplingKeys
    {
    public:
    	CouplingKeys()
    	{
    	}
        void Trace( const set<Coupling *> &matches ) const;
        Result KeyAndRestrict( shared_ptr<Node> x,
        		               shared_ptr<Node> pattern,
                               const RootedSearchReplace *sr,
                               bool can_key );
        Result KeyAndRestrict( shared_ptr<Key> key,
        		               shared_ptr<Node> pattern,
                               const RootedSearchReplace *sr,
                               bool can_key );
        shared_ptr<Node> KeyAndSubstitute( shared_ptr<Node> x, // source after soft nodes etc
        		                           shared_ptr<Node> pattern, // source
                                           const RootedSearchReplace *sr,
                                           bool can_key );
        shared_ptr<Node> KeyAndSubstitute( shared_ptr<Key> key,
        		                           shared_ptr<Node> pattern,
                                           const RootedSearchReplace *sr,
                                           bool can_key );
    private:
        const Coupling *FindCoupling( shared_ptr<Node> node, const set<Coupling *> &matches );
    	Map< const Coupling *, shared_ptr<Key> > keys_map;
    };
    set<Coupling *> matches;

    class Conjecture
    {
    private:
        typedef GenericContainer::iterator Choice;
    public:
    	void PrepareForDecidedCompare();
    	GenericContainer::iterator HandleDecision( GenericContainer::iterator begin,
    			                                   GenericContainer::iterator end );
    	RootedSearchReplace::Result Search( shared_ptr<Node> x,
    			                            shared_ptr<Node> pattern,
    			                            CouplingKeys *keys,
    			                            bool can_key,
    			                            const RootedSearchReplace *sr );
    private:
    	bool ShouldTryMore( Result r, int threshold );
    	int decision_index;
    	vector<Choice> choices;
    };

    // Constructor and destructor. Search and replace patterns and match sets are 
    // specified here, so that we have a fully confiugured functor.
    RootedSearchReplace( shared_ptr<Node> sp=shared_ptr<Node>(),
                         shared_ptr<Node> rp=shared_ptr<Node>(),
                         set<Coupling *> m = set<Coupling *>(),
                         vector<RootedSearchReplace *> slaves = vector<RootedSearchReplace *>() );
    void Configure( shared_ptr<Node> sp=shared_ptr<Node>(),
                    shared_ptr<Node> rp=shared_ptr<Node>(),
                    set<Coupling *> m = set<Coupling *>(),
                    vector<RootedSearchReplace *> slaves = vector<RootedSearchReplace *>() );
    ~RootedSearchReplace();
    
    // implementation ring: Do the actual search and replace
    Result SingleSearchReplace( shared_ptr<Node> *proot,
                                shared_ptr<Node> search_pattern,
                                shared_ptr<Node> replace_pattern,
                                CouplingKeys match_keys = CouplingKeys() );
    int RepeatingSearchReplace( shared_ptr<Node> *proot,
                                shared_ptr<Node> search_pattern,
                                shared_ptr<Node> replace_pattern,
                                CouplingKeys match_keys = CouplingKeys() );
    // Functor style interface for RepeatingSearchReplace; implements Pass interface.
    void operator()( shared_ptr<Node> context, shared_ptr<Node> *proot );

    // Stuff for soft nodes; support this base class in addition to whatever tree intermediate
    // is required. Call GetProgram() if program root needed; call DecidedCompare() to recurse
    // back into the general search algorithm.
    shared_ptr<Node> GetContext() const { ASSERT(pcontext&&*pcontext); return *pcontext; }
    struct SoftSearchPattern : virtual Node
    {
        virtual RootedSearchReplace::Result DecidedCompare( const RootedSearchReplace *sr,
        		                                      shared_ptr<Node> x,
        		                                      CouplingKeys *match_keys,
        		                                      bool can_key,
        		                                      Conjecture &conj ) const = 0;
    };
    struct SoftReplacePattern : virtual Node
    {
        virtual shared_ptr<Node> DuplicateSubtree( const RootedSearchReplace *sr,
        		                                   CouplingKeys *match_keys,
        		                                   bool can_key ) = 0;
    };

    // Some self-testing
    static void Test();
        
    shared_ptr<Node> search_pattern;
    shared_ptr<Node> replace_pattern;
    vector<RootedSearchReplace *> slaves;
    shared_ptr<Node> *pcontext;
    
private:
    // LocalCompare ring
    bool LocalCompare( shared_ptr<Node> x,
    		           shared_ptr<Node> pattern ) const;

    // DecidedCompare ring
    Result DecidedCompare( GenericSequence &x,
    		               GenericSequence &pattern,
    		               CouplingKeys *match_keys,
    		               bool can_key,
    		               Conjecture &conj ) const;
    Result DecidedCompare( GenericCollection &x,
    		               GenericCollection &pattern,
    		               CouplingKeys *match_keys,
    		               bool can_key,
    		               Conjecture &conj ) const;
    Result DecidedCompare( shared_ptr<Node> x,
    		               shared_ptr<StuffBase> stuff_pattern,
    		               CouplingKeys *match_keys,
    		               bool can_key,
    		               Conjecture &conj ) const;
public:
    Result DecidedCompare( shared_ptr<Node> x,
    		               shared_ptr<Node> pattern,
    		               CouplingKeys *match_keys,
    		               bool can_key,
    		               Conjecture &conj ) const;
private:
    // MatchingDecidedCompare ring
    Result MatchingDecidedCompare( shared_ptr<Node> x,
    		                       shared_ptr<Node> pattern,
    		                       CouplingKeys *match_keys,
    		                       bool can_key,
    		                       Conjecture &conj ) const;

    // Compare ring (now trivial)
public:
    Result Compare( shared_ptr<Node> x,
    		        shared_ptr<Node> pattern,
    		        CouplingKeys *match_keys = NULL,
    		        bool can_key = false ) const;
private:
    // Replace ring
    void ClearPtrs( shared_ptr<Node> dest ) const;
    void Overlay( shared_ptr<Node> dest,
    		      shared_ptr<Node> source,
    		      CouplingKeys *match_keys,
    		      bool can_key,
    		      shared_ptr<Key> current_key ) const; // under substitution if not NULL
    void Overlay( GenericSequence *dest,
    		      GenericSequence *source,
    		      CouplingKeys *match_keys,
    		      bool can_key,
    		      shared_ptr<Key> current_key ) const;
    void Overlay( GenericCollection *dest,
    	          GenericCollection *source,
    	          CouplingKeys *match_keys,
    	          bool can_key,
    	          shared_ptr<Key> current_key ) const;
public:
    shared_ptr<Node> DuplicateSubtree( shared_ptr<Node> x,
    		                           CouplingKeys *match_keys,
    		                           bool can_key,
    		                           shared_ptr<Key> current_key=shared_ptr<Key>() ) const;
private:
    shared_ptr<Node> MatchingDuplicateSubtree( shared_ptr<Node> x,
    		                                   CouplingKeys *match_keys ) const;
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
                   set<Coupling *> m = set<Coupling *>(),
                   vector<RootedSearchReplace *> slaves = vector<RootedSearchReplace *>() );
    void Configure( shared_ptr<Node> sp = shared_ptr<Node>(),
                    shared_ptr<Node> rp = shared_ptr<Node>(),
                    set<Coupling *> m = set<Coupling *>(),
                    vector<RootedSearchReplace *> slaves = vector<RootedSearchReplace *>() );
private:
	Coupling root_match;
};

#endif

