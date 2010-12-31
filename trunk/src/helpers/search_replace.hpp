#ifndef SEARCH_REPLACE_HPP
#define SEARCH_REPLACE_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "walk.hpp"
#include "transformation.hpp"
#include "coupling.hpp"
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
//   passed to the constructor. Must be >= 1 node in normal (not abnormal)
//   context.
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
// - Multi-node wildcards like * in sequences and collections (Star node).
//
// - Recursive wildcards, arbitrary depth and arbitrary depth with
//   restricted intermediates (the Stuff node). Restriction can be a
//   general tree (in abnormal context)
//
// - Slave search/replace so that a second SR can happen for each match
//   of the first one, and can borrow its couplings.
//
// - Boolean rules supported by NotMatch, MAtchAll, MatchAny, MatchN and
//   MatchOne. For all but MatchAll, pattern is abnormal context.
//
// - The base type supplied as template param to all special nodes
//   acts as a pre-restriction according to usual topological rules.
//

class Conjecture;


// --- General note on SPECIAL_NODE_FUNCTIONS and PRE_RESTRICTION ---
// Special nodes (that is nodes defined here with special S&R behaviour)
// derive from a normal tree node via templating. This base class is
// the PRE_RESTRICTION node, and we want it for 2 reasons:
// 1. To allow normal nodes to point to special nodes, they must
//    expose a normal interface, which can vary depending on usage
//    so must be templated.
// 2. We are able to provide a "free" and-rule restriction on all
//    special nodes by restricting to non-strict subclasses of the
//    pre-restrictor.
// In order to make 2. work, we need to *avoid* overriding IsLocalMatch()
// or IsSubclass() on special nodes, so that the behaviour of the 
// PRE_RESTRICTION is preserved wrt comparisons. So all special nodes
// including speicialisations of TransformTo etc should use 
// SPECIAL_NODE_FUNCTIONS instead of NODE_FUNCTIONS.
// Itemise is known required (for eg graph plotting), other bounces
// are TBD.
#define SPECIAL_NODE_FUNCTIONS ITEMISE_FUNCTION  

struct SpecialBase
{
    virtual shared_ptr< TreePtrInterface > GetPreRestrictionArchitype() = 0;
};
template<class PRE_RESTRICTION>
struct Special : SpecialBase, PRE_RESTRICTION
{
    virtual shared_ptr< TreePtrInterface > GetPreRestrictionArchitype()
    {
	    // Esta muchos indirection
	    return shared_ptr<TreePtrInterface>( new TreePtr<PRE_RESTRICTION>( new PRE_RESTRICTION ));	
    }
};


// The * wildcard can match more than one node of any type in a container
// In a Sequence, only a contiguous subsequence of 0 or more elements will match
// In a Collection, a sub-collection of 0 or more elements may be matched anywhere in the collection
// Only one Star is allowed in a Collection. Star must be templated on a type that is allowed
// in the collection.
struct StarBase : virtual Node {};
template<class PRE_RESTRICTION>
struct Star : StarBase, Special<PRE_RESTRICTION> { SPECIAL_NODE_FUNCTIONS };


// The Stuff wildcard can match a truncated subtree with special powers as listed by the members
struct StuffBase : virtual Node
{
	TreePtr<Node> recurse_restriction; // Restricts the intermediate nodes in the truncated subtree
	TreePtr<Node> terminus; // A node somewhere under Stuff, that matches normally, truncating the subtree
};
template<class PRE_RESTRICTION>
struct Stuff : StuffBase, Special<PRE_RESTRICTION> { SPECIAL_NODE_FUNCTIONS };
struct StuffKey : Key
{
	TreePtr<Node> terminus;
};


struct GreenGrassBase : virtual Node
{
	virtual TreePtr<Node> GetThrough() const = 0;
};
template<class PRE_RESTRICTION>
struct GreenGrass : GreenGrassBase, Special<PRE_RESTRICTION>
{
	SPECIAL_NODE_FUNCTIONS
	TreePtr<PRE_RESTRICTION> through;
	virtual TreePtr<Node> GetThrough() const
	{
		return TreePtr<Node>( through );
	}
};


class RootedSearchReplace : InPlaceTransformation // TODO rename RootedSearchReplace -> MatchReplace
{  
public:
    // Constructor and destructor. Search and replace patterns and couplings are
    // specified here, so that we have a fully confiugured functor.
    RootedSearchReplace( TreePtr<Node> sp,
                         TreePtr<Node> rp=TreePtr<Node>(),
                         CouplingSet m = CouplingSet() );
    RootedSearchReplace( TreePtr<Node> sp,
                         TreePtr<Node> rp,
                         int ); // for use by slave wrappers
    ~RootedSearchReplace();
    
    // Stuff for soft nodes; support this base class in addition to whatever tree intermediate
    // is required. Call GetProgram() if program root needed; call DecidedCompare() to recurse
    // back into the general search algorithm.
    TreePtr<Node> GetContext() const { ASSERT(pcontext&&*pcontext); return *pcontext; }
    struct SoftSearchPattern
    {
        virtual Result DecidedCompare( const RootedSearchReplace *sr,
        		                                      TreePtr<Node> x,
        		                                      CouplingKeys *match_keys,
        		                                      bool can_key,
        		                                      Conjecture &conj ) const = 0;
    };
    struct SoftReplacePattern
    {
        virtual TreePtr<Node> DuplicateSubtree( const RootedSearchReplace *sr,
        		                                   CouplingKeys *match_keys,
        		                                   bool can_key ) = 0;
    };

    // Some self-testing
    static void Test();
        
    CouplingSet couplings;
    TreePtr<Node> search_pattern;
    TreePtr<Node> replace_pattern;
    TreePtr<Node> *pcontext;
    mutable set< TreePtr<Node> > dirty_grass;
    
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
    // implementation ring: Do the actual search and replace
    Result SingleSearchReplace( TreePtr<Node> *proot,
                                TreePtr<Node> search_pattern,
                                TreePtr<Node> replace_pattern,
                                CouplingKeys match_keys );
public:
    int RepeatingSearchReplace( TreePtr<Node> *proot,
                                TreePtr<Node> search_pattern,
                                TreePtr<Node> replace_pattern,
                                CouplingKeys match_keys );

    virtual void DefaultRepeatingSearchReplace( TreePtr<Node> *proot,
						             			CouplingKeys match_keys = CouplingKeys() );
    // Functor style interface for RepeatingSearchReplace; implements Pass interface.
    void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
private:
    // Internal node classes
    struct SubSequence : Node,
                         Sequence<Node>
    {
    	NODE_FUNCTIONS // Need these for Clone/Duplicate, called during replace TODO add to SPECIAL_NODE_FUNCTIONS and use that
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
    SearchReplace( TreePtr<Node> sp,
                   TreePtr<Node> rp = TreePtr<Node>(),
                   CouplingSet m = CouplingSet() );
    SearchReplace( TreePtr<Node> sp,
                   TreePtr<Node> rp,
                   int ); // for use by slave wrappers
    virtual void DefaultRepeatingSearchReplace( TreePtr<Node> *proot,
							          		    CouplingKeys match_keys = CouplingKeys() );
};

// TODO extract common base for slaves, and use in DuplicateSubtree() and maybe elsewhere
struct RootedSlaveBase : virtual Node,
                         public RootedSearchReplace
{
	RootedSlaveBase( TreePtr<Node> sp, TreePtr<Node> rp=TreePtr<Node>() ) :
		RootedSearchReplace( sp, rp, 0 )
	{}
	virtual TreePtr<Node> GetThrough() const = 0;
};
template<class PRE_RESTRICTION>
struct RootedSlave : RootedSlaveBase, Special<PRE_RESTRICTION>
{
	SPECIAL_NODE_FUNCTIONS

	// Slave must be constructed using constructor
	RootedSlave( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
		through( t ),
		RootedSlaveBase( sp, rp )
	{
	}

	TreePtr<PRE_RESTRICTION> through;
	virtual TreePtr<Node> GetThrough() const
	{
		return TreePtr<Node>( through );
	}
};

struct SlaveBase : virtual Node,
                   public SearchReplace
{
	SlaveBase( TreePtr<Node> sp, TreePtr<Node> rp=TreePtr<Node>() ) :
		SearchReplace( sp, rp, 0 )
	{}
	virtual TreePtr<Node> GetThrough() const = 0;
};
template<class PRE_RESTRICTION>
struct Slave : SlaveBase, Special<PRE_RESTRICTION>
{
	SPECIAL_NODE_FUNCTIONS

	// Slave must be constructed using constructor
	Slave( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
		through( t ),
		SlaveBase( sp, rp )
	{
	}

	TreePtr<PRE_RESTRICTION> through;
	virtual TreePtr<Node> GetThrough() const
	{
		return TreePtr<Node>( through );
	}
};

struct OverlayBase : virtual Node
{
};

template<class PRE_RESTRICTION>
struct Overlay : virtual Special<PRE_RESTRICTION>
{
};

#endif

