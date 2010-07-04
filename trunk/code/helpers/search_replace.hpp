#ifndef SEARCH_REPLACE_HPP
#define SEARCH_REPLACE_HPP

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
// - Sequence/ContainerCommon support: sequences require matching ordering
//   and containers do not (only the presence of the matching elements).
//
// - Multi-node wildcards like * in sequences and containers (Star node).
//
// - Recursive wildcards, arbitrary depth and arbitrary depth with
//   restricted intermediates (the Stuff node).
//
// - Slave search/replace so that a second SR can happen for each match
//   of the first one, and can borrow its couplings.

// This just serves to complicate matters - just use bools directly
enum Result { NOT_FOUND = (int)false,
			  FOUND     = (int)true };


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
	TreePtr<Node> restrictor; // Restricts the intermediate nodes in the truncated subtree
	TreePtr<Node> terminus; // A node somewhere under Stuff, that matches normally, truncating the subtree
};
template<class VALUE_TYPE>
struct Stuff : StuffBase, VALUE_TYPE { NODE_FUNCTIONS };



// Base class for coupling keys; this deals with individual node matches, and also with stars
// by means of pointing "root" at a SubCollection or SubSequence
struct Key
{
	virtual ~Key(){}  // be a virtual hierarchy
	TreePtr<Node> root; // Tree node at matched pattern; root of any replace subtree
	TreePtr<Node> replace_pattern; // Tree node at matched pattern; root of any replace subtree
};

struct StuffKey : Key
{
	TreePtr<StuffBase> search_stuff; // TODO add search_pattern to Key base class and lose this (as with replace_pattern)
	TreePtr<Node> terminus;
};

struct Coupling : public set< TreePtr<Node> >
{
	inline Coupling() {}
/*	inline Coupling( const Sequence< Node > &seq )
	{
		Sequence< Node > s2 = seq;
		FOREACH( TreePtr<Node> v, s2 )
			insert( v );
	}*/
	template<typename L, typename R>
	inline Coupling( pair<L, R> p )
	{
		Coupling l( p.first );
		FOREACH( TreePtr<Node> v, l )
			insert( v );
		Coupling r( p.second );
		FOREACH( TreePtr<Node> v, r )
			insert( v );
	}
	inline Coupling( TreePtr<Node> n )
	{
		insert( n );
	}
};

typedef set<Coupling> CouplingSet;

class RootedSearchReplace;
class CouplingKeys
{
public:
	CouplingKeys()
	{
	}
	Result KeyAndRestrict( TreePtr<Node> x,
						   TreePtr<Node> pattern,
						   const RootedSearchReplace *sr,
						   bool can_key );
	Result KeyAndRestrict( shared_ptr<Key> key,
						   TreePtr<Node> pattern,
						   const RootedSearchReplace *sr,
						   bool can_key );
	TreePtr<Node> KeyAndSubstitute( TreePtr<Node> x, // source after soft nodes etc
									   TreePtr<Node> pattern, // source
									   const RootedSearchReplace *sr,
									   bool can_key );
	TreePtr<Node> KeyAndSubstitute( shared_ptr<Key> key,
									   TreePtr<Node> pattern,
									   const RootedSearchReplace *sr,
									   bool can_key );
private:
	Coupling FindCoupling( TreePtr<Node> node, const CouplingSet &couplings );
	Map< Coupling, shared_ptr<Key> > keys_map;
};

class Conjecture
{
private:
    typedef ContainerInterface::iterator Choice;
public:
	void PrepareForDecidedCompare();
	ContainerInterface::iterator HandleDecision( ContainerInterface::iterator begin,
			                                   ContainerInterface::iterator end );
	Result Search( TreePtr<Node> x,
				   TreePtr<Node> pattern,
				   CouplingKeys *keys,
				   bool can_key,
				   const RootedSearchReplace *sr );
private:
	bool ShouldTryMore( Result r, int threshold );
	int decision_index;
	vector<Choice> choices;
};


class RootedSearchReplace : InPlaceTransformation
{  
public:
    CouplingSet couplings;

    // Constructor and destructor. Search and replace patterns and couplings are
    // specified here, so that we have a fully confiugured functor.
    RootedSearchReplace( TreePtr<Node> sp=TreePtr<Node>(),
                         TreePtr<Node> rp=TreePtr<Node>(),
                         CouplingSet m = CouplingSet(),
                         vector<RootedSearchReplace *> slaves = vector<RootedSearchReplace *>() );
    void Configure( TreePtr<Node> sp=TreePtr<Node>(),
                    TreePtr<Node> rp=TreePtr<Node>(),
                    CouplingSet m = CouplingSet(),
                    vector<RootedSearchReplace *> slaves = vector<RootedSearchReplace *>() );
    ~RootedSearchReplace();
    
    // implementation ring: Do the actual search and replace
    Result SingleSearchReplace( TreePtr<Node> *proot,
                                TreePtr<Node> search_pattern,
                                TreePtr<Node> replace_pattern,
                                CouplingKeys match_keys = CouplingKeys() );
    int RepeatingSearchReplace( TreePtr<Node> *proot,
                                TreePtr<Node> search_pattern,
                                TreePtr<Node> replace_pattern,
                                CouplingKeys match_keys = CouplingKeys() );
    // Functor style interface for RepeatingSearchReplace; implements Pass interface.
    void operator()( TreePtr<Node> context, TreePtr<Node> *proot );

    // Stuff for soft nodes; support this base class in addition to whatever tree intermediate
    // is required. Call GetProgram() if program root needed; call DecidedCompare() to recurse
    // back into the general search algorithm.
    TreePtr<Node> GetContext() const { ASSERT(pcontext&&*pcontext); return *pcontext; }
    struct SoftSearchPattern : virtual Node
    {
        virtual Result DecidedCompare( const RootedSearchReplace *sr,
        		                                      TreePtr<Node> x,
        		                                      CouplingKeys *match_keys,
        		                                      bool can_key,
        		                                      Conjecture &conj ) const = 0;
    };
    struct SoftReplacePattern : virtual Node
    {
        virtual TreePtr<Node> DuplicateSubtree( const RootedSearchReplace *sr,
        		                                   CouplingKeys *match_keys,
        		                                   bool can_key ) = 0;
    };

    // Some self-testing
    static void Test();
        
    TreePtr<Node> search_pattern;
    TreePtr<Node> replace_pattern;
    vector<RootedSearchReplace *> slaves;
    TreePtr<Node> *pcontext;
    
private:
    // LocalCompare ring
    bool LocalCompare( TreePtr<Node> x,
    		           TreePtr<Node> pattern ) const;

    // DecidedCompare ring
    Result DecidedCompare( SequenceInterface &x,
    		               SequenceInterface &pattern,
    		               CouplingKeys *match_keys,
    		               bool can_key,
    		               Conjecture &conj ) const;
    Result DecidedCompare( CollectionInterface &x,
    		               CollectionInterface &pattern,
    		               CouplingKeys *match_keys,
    		               bool can_key,
    		               Conjecture &conj ) const;
    Result DecidedCompare( TreePtr<Node> x,
    		               TreePtr<StuffBase> stuff_pattern,
    		               CouplingKeys *match_keys,
    		               bool can_key,
    		               Conjecture &conj ) const;
public:
    Result DecidedCompare( TreePtr<Node> x,
    		               TreePtr<Node> pattern,
    		               CouplingKeys *match_keys,
    		               bool can_key,
    		               Conjecture &conj ) const;
private:
    // MatchingDecidedCompare ring
    friend class Conjecture;
    Result MatchingDecidedCompare( TreePtr<Node> x,
    		                       TreePtr<Node> pattern,
    		                       CouplingKeys *match_keys,
    		                       bool can_key,
    		                       Conjecture &conj ) const;

    // Compare ring (now trivial)
public:
    Result Compare( TreePtr<Node> x,
    		        TreePtr<Node> pattern,
    		        CouplingKeys *match_keys = NULL,
    		        bool can_key = false ) const;
private:
    // Replace ring
    void ClearPtrs( TreePtr<Node> dest ) const;
    void Overlay( TreePtr<Node> dest,
    		      TreePtr<Node> source,
    		      CouplingKeys *match_keys,
    		      bool can_key,
    		      shared_ptr<Key> current_key ) const; // under substitution if not NULL
    void Overlay( SequenceInterface *dest,
    		      SequenceInterface *source,
    		      CouplingKeys *match_keys,
    		      bool can_key,
    		      shared_ptr<Key> current_key ) const;
    void Overlay( CollectionInterface *dest,
    	          CollectionInterface *source,
    	          CouplingKeys *match_keys,
    	          bool can_key,
    	          shared_ptr<Key> current_key ) const;
public:
    TreePtr<Node> DuplicateSubtree( TreePtr<Node> x,
    		                           CouplingKeys *match_keys,
    		                           bool can_key,
    		                           shared_ptr<Key> current_key=shared_ptr<Key>() ) const;
private:
    TreePtr<Node> MatchingDuplicateSubtree( TreePtr<Node> x,
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
    SearchReplace( TreePtr<Node> sp = TreePtr<Node>(),
                   TreePtr<Node> rp = TreePtr<Node>(),
                   CouplingSet m = CouplingSet(),
                   vector<RootedSearchReplace *> slaves = vector<RootedSearchReplace *>() );
    void Configure( TreePtr<Node> sp = TreePtr<Node>(),
                    TreePtr<Node> rp = TreePtr<Node>(),
                    CouplingSet m = CouplingSet(),
                    vector<RootedSearchReplace *> slaves = vector<RootedSearchReplace *>() );
};

struct NotMatchBase {};

template<class VALUE_TYPE>
struct NotMatch : VALUE_TYPE,
                 RootedSearchReplace::SoftSearchPattern,
                 NotMatchBase
{
    NODE_FUNCTIONS
    TreePtr<VALUE_TYPE> pattern;
private:
    virtual Result DecidedCompare( const RootedSearchReplace *sr,
    		                                            TreePtr<Node> x,
    		                                            CouplingKeys *keys,
    		                                            bool can_key,
    		                                            Conjecture &conj ) const
    {
    	if( keys && can_key )
    	{
    		// Don't do a subtree search while keying - we'll only end up keying the wrong thing
    		// or terminating with NOT_FOUND prematurely
    		return FOUND;
    	}
    	else
    	{
    	    // Do not use the present conjecture since we would mess it up because
    	    // a. We didn't recurse during KEYING pass and
    	    // b. Search under not can terminate with NOT_FOUND, but parent search will continue
    	    // Consequently, we go in at Compare level, which creates a new conjecture.
    		Result r = sr->Compare( x, TreePtr<Node>(pattern), keys, false );
			TRACE("SoftNot got %d, returning the opposite!\n", (int)r);
    		if( r==NOT_FOUND )
				return FOUND;
			else
				return NOT_FOUND;
    	}
    }
};

struct MatchAllBase {};

template<class VALUE_TYPE>
struct MatchAll : VALUE_TYPE,
                 RootedSearchReplace::SoftSearchPattern,
                 MatchAllBase
{
    NODE_FUNCTIONS
    mutable Collection<VALUE_TYPE> patterns; // TODO provide const iterators and remove mutable
private:
    virtual Result DecidedCompare( const RootedSearchReplace *sr,
    		                                            TreePtr<Node> x,
    		                                            CouplingKeys *keys,
    		                                            bool can_key,
    		                                            Conjecture &conj ) const
    {
    	typedef ContainerInterface::iterator iter; // TODO clean up this loop
    	iter it;
    	int i;
    	for( it = patterns.begin(), i = 0;
    		 it != patterns.end();
    		 ++it, ++i )
    	{
    		Result r = sr->DecidedCompare( x, TreePtr<Node>(*it), keys, can_key, conj );
    		TRACE("AND[%d] got %d\n", i, r);
    	    if( !r )
    	    	return NOT_FOUND;
    	}

        return FOUND;
    }
};


struct TransformToBase : RootedSearchReplace::SoftSearchPattern
{
    NODE_FUNCTIONS
    TreePtr<Node> pattern;
    TransformToBase( Transformation *t ) :
    	transformation(t)
    {
    }

private:
    virtual Result DecidedCompare( const RootedSearchReplace *sr,
    		                                            TreePtr<Node> x,
    		                                            CouplingKeys *keys,
    		                                            bool can_key,
    		                                            Conjecture &conj ) const;
    Transformation *transformation;
};

template<class VALUE_TYPE>
struct TransformTo : TransformToBase, VALUE_TYPE
{
	NODE_FUNCTIONS
    TransformTo( Transformation *t ) : TransformToBase (t) {}
};


inline CouplingSet operator,( Coupling l, Coupling r )
{
	CouplingSet cs;
    cs.insert( l );
    cs.insert( r );
    return cs;
}

inline CouplingSet operator,( CouplingSet l, Coupling r )
{
    l.insert( r );
    return l;
}


#endif

