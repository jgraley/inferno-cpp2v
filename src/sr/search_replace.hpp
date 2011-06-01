#ifndef SEARCH_REPLACE_HPP
#define SEARCH_REPLACE_HPP

#include "common/common.hpp"
#include "common/read_args.hpp"
#include "node/walk.hpp"
#include "node/transformation.hpp"
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
//   program node if a the same node is pointed to by more than one
//   parent in the search pattern. Must be >= 1 node in normal 
//   context.
//
// - Nodes in the replace pattern may be substituted by program nodes
//   found during matching by allowing the replace pattern to point to 
//   a node also present in the search patter (in a normal context).
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
// - SlaveSearchReplace search/replace so that a second SR can happen 
//   for each match of the first one, and can borrow its couplings.
//
// - Boolean rules supported by NotMatch, MAtchAll, MatchAny and
//   MatchOdd. For all but MatchAll, pattern is abnormal context.
//
// - The base type supplied as template param to all special nodes
//   acts as a pre-restriction according to usual topological rules.
//
// - Green grass node only matches nodes that have not already been
//   replaced, to avoid spinning forever.
//
// - Overlay node allows a replace pattern to be overlayed over 
//   a substituted node. NULL shared_ptr (or empty container) in overly 
//   means fill this in from the substitute. Intemediate node types
//   mean fill in those members introduced in derived classes from the
//   substitute.
//

class Conjecture;
class SpecialBase;
class StuffBase;
class StarBase;

class CompareReplace : virtual public InPlaceTransformation, 
                       Filter // TODO extract Compare, and make that the filter
{  
public:
    // Constructor and destructor. Search and replace patterns and couplings are
    // specified here, so that we have a fully confiugured functor.
    CompareReplace( TreePtr<Node> cp = TreePtr<Node>(),
                    TreePtr<Node> rp = TreePtr<Node>(),
                    bool im = true );
                    
    // Call this to set the patterns after construction. This should not be virtual since
    // the constructor calls it.
    void Configure( TreePtr<Node> cp,
                    TreePtr<Node> rp = TreePtr<Node>() );                 
    
    // Stuff for soft nodes; support this base class in addition to whatever tree intermediate
    // is required. Call GetProgram() if program root needed; call DecidedCompare() to recurse
    // back into the general search algorithm.
    TreePtr<Node> GetContext() const { ASSERT(pcontext&&*pcontext); return *pcontext; }
    struct SoftSearchPattern
    {
        virtual Result DecidedCompare( const CompareReplace *sr,
        		                       TreePtr<Node> x,
        		                       bool can_key,
        		                       Conjecture &conj ) const = 0;
    };
    struct SoftReplacePattern
    {
        virtual TreePtr<Node> DuplicateSubtree( const CompareReplace *sr,
        		                                bool can_key ) = 0;
    };

    // Some self-testing
    static void Test();
        
    bool is_master;
    CouplingSet couplings;
    TreePtr<Node> compare_pattern;
    TreePtr<Node> replace_pattern;
    TreePtr<Node> *pcontext;
    shared_ptr< CouplingKeys > coupling_keys;
    mutable set< TreePtr<Node> > dirty_grass;

    virtual void GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *links ) const;

private:
    // LocalCompare ring
    bool LocalCompare( TreePtr<Node> x,
    		           TreePtr<Node> pattern ) const;

    // DecidedCompare ring
    Result DecidedCompare( SequenceInterface &x,
    		               SequenceInterface &pattern,
    		               bool can_key,
    		               Conjecture &conj ) const;
    Result DecidedCompare( CollectionInterface &x,
    		               CollectionInterface &pattern,
    		               bool can_key,
    		               Conjecture &conj ) const;
    Result DecidedCompare( TreePtr<Node> x,
    		               TreePtr<StuffBase> stuff_pattern,
    		               bool can_key,
    		               Conjecture &conj ) const;
public:
    Result DecidedCompare( TreePtr<Node> x,
    		               TreePtr<Node> pattern,
    		               bool can_key,
    		               Conjecture &conj ) const;
private:
    // MatchingDecidedCompare ring
    friend class Conjecture;
    Result MatchingDecidedCompare( TreePtr<Node> x,
    		                       TreePtr<Node> pattern,
    		                       bool can_key,
    		                       Conjecture &conj ) const;

    // Compare ring (now trivial)
public:
    Result Compare( TreePtr<Node> x,
    		        TreePtr<Node> pattern,
	                bool can_key = false ) const;
    virtual bool IsMatch( TreePtr<Node> context,       
                          TreePtr<Node> root );
private:
    // Replace ring
    void ClearPtrs( TreePtr<Node> dest ) const;
    TreePtr<Node> DoOverlayOrOverwrite( TreePtr<Node> dest,
    		                            TreePtr<Node> source,
    		                            bool can_key,
    		                            shared_ptr<Key> current_key ) const; 
    void DoOverlay( TreePtr<Node> dest,
    		        TreePtr<Node> source,
    		        bool can_key,
    		        shared_ptr<Key> current_key ) const; // under substitution if not NULL
    void DoOverlay( SequenceInterface *dest,
      		        SequenceInterface *source,
    		        bool can_key,
    		        shared_ptr<Key> current_key ) const;
    void DoOverlay( CollectionInterface *dest,
    	            CollectionInterface *source,
    	            bool can_key,
    	            shared_ptr<Key> current_key ) const;
    TreePtr<Node> DuplicateNode( TreePtr<Node> source,
    		                              bool can_key,
    		                              shared_ptr<Key> current_key=shared_ptr<Key>() ) const;
    TreePtr<Node> ApplySpecialAndCoupling( TreePtr<Node> source,
    		                               bool can_key,
    		                               shared_ptr<Key> current_key=shared_ptr<Key>() ) const;
public:
    TreePtr<Node> DuplicateSubtree( TreePtr<Node> source,
    		                        bool can_key,
    		                        shared_ptr<Key> current_key=shared_ptr<Key>() ) const;
private:
    TreePtr<Node> MatchingDuplicateSubtree( TreePtr<Node> x ) const;
    // implementation ring: Do the actual search and replace
    Result SingleCompareReplace( TreePtr<Node> *proot );
    int RepeatingCompareReplace( TreePtr<Node> *proot );
public:
    // Functor style interface for RepeatingSearchReplace; implements Pass interface.
    using Transformation::operator();
    void operator()( TreePtr<Node> context, 
                     TreePtr<Node> *proot );
private:
    // Internal node classes - NOTE these are not special nodes, and we use them like normal tree nodes
    struct SubSequence : Node,
                         Sequence<Node>
    {
    	NODE_FUNCTIONS_FINAL 
    };
    struct SubCollection : Node,
                           Collection<Node>
    {
    	NODE_FUNCTIONS_FINAL
    };
};


class SearchReplace : public CompareReplace
{
public:
    SearchReplace( TreePtr<Node> sp = TreePtr<Node>(),
                   TreePtr<Node> rp = TreePtr<Node>(),
                   bool im = true );
                   
    // Call this to set the patterns after construction. This should not be virtual since
    // the constructor calls it.
    void Configure( TreePtr<Node> sp,
                    TreePtr<Node> rp = TreePtr<Node>() );                 
                    
    virtual void GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *links ) const;
};


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
struct Special : SpecialBase, virtual PRE_RESTRICTION
{
    virtual shared_ptr< TreePtrInterface > GetPreRestrictionArchitype()
    {
        // Esta muchos indirection
        return shared_ptr<TreePtrInterface>( new TreePtr<PRE_RESTRICTION>( new PRE_RESTRICTION ));  
    }
};




struct SlaveBase : virtual Node, virtual InPlaceTransformation
{
    virtual TreePtr<Node> GetThrough() const = 0;
    virtual void MergeCouplings( const CouplingSet &cs, shared_ptr<CouplingKeys> ck ) = 0;
};

template<typename ALGO>
struct SlaveIntermediate : public SlaveBase, public ALGO                                 
{
	SlaveIntermediate( TreePtr<Node> sp, TreePtr<Node> rp ) :
		ALGO( sp, rp, false )
	{}
    virtual void MergeCouplings( const CouplingSet &cs, shared_ptr<CouplingKeys> ck )
    {
        // TODO is there a way of unioning sets without doing lots of insert()?
        FOREACH( Coupling c, cs )
            ALGO::couplings.insert( c ); 
        ALGO::coupling_keys = ck; 
    }
    virtual void GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *links ) const
    {
        labels->push_back("through");
        links->push_back(GetThrough());
        ALGO::GetGraphInfo( labels, links );
    }
};

template<typename ALGO, class PRE_RESTRICTION>
struct Slave : SlaveIntermediate<ALGO>, Special<PRE_RESTRICTION>
{
	SPECIAL_NODE_FUNCTIONS

	// SlaveSearchReplace must be constructed using constructor
	Slave( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp, TreePtr<Node> rp ) :
		through( t ),
		SlaveIntermediate<ALGO>( sp, rp )
	{
	}

	TreePtr<PRE_RESTRICTION> through;
	virtual TreePtr<Node> GetThrough() const
	{
		return TreePtr<Node>( through );
	}
};

// Partial specialisation is an arse in C++
template<class PRE_RESTRICTION>
struct SlaveCompareReplace : Slave<CompareReplace, PRE_RESTRICTION> 
{
    SlaveCompareReplace( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
        Slave<CompareReplace, PRE_RESTRICTION>( t, sp, rp ) {}
};
template<class PRE_RESTRICTION>
struct SlaveSearchReplace : Slave<SearchReplace, PRE_RESTRICTION>
{
    SlaveSearchReplace( TreePtr<PRE_RESTRICTION> t, TreePtr<Node> sp=TreePtr<Node>(), TreePtr<Node> rp=TreePtr<Node>() ) :
        Slave<SearchReplace, PRE_RESTRICTION>( t, sp, rp ) {}
};




// The * wildcard can match more than one node of any type in a container
// In a Sequence, only a contiguous subsequence of 0 or more elements will match
// In a Collection, a sub-collection of 0 or more elements may be matched anywhere in the collection
// Only one Star is allowed in a Collection. Star must be templated on a type that is allowed
// in the collection. TODO a restrict pattern
struct StarBase : virtual Node {};
template<class PRE_RESTRICTION>
struct Star : StarBase, Special<PRE_RESTRICTION> { SPECIAL_NODE_FUNCTIONS };




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




// The Stuff wildcard can match a truncated subtree with special powers as listed by the members
struct StuffBase : virtual Node
{
    TreePtr<Node> recurse_restriction; // Restricts the intermediate nodes in the truncated subtree
    TreePtr<Node> terminus; // A node somewhere under Stuff, that matches normally, truncating the subtree
    CompareReplace recurse_comparer; // TODO only need the compare half, maybe split it out?
    
    // Do the itemiser by hand since it gets confused by the CompareReplace object
};
template<class PRE_RESTRICTION>
struct Stuff : StuffBase, Special<PRE_RESTRICTION> 
{
    virtual vector< Itemiser::Element * > Itemise( const Itemiser *itemise_object = 0 ) const
    {
        vector< Itemiser::Element * > v;
        v.push_back( (Itemiser::Element *)(&recurse_restriction) );
        v.push_back( (Itemiser::Element *)(&terminus) );
        TRACE("%p %p\n", v[0], v[1] );
        return v;
    }
    virtual Itemiser::Element *ItemiseIndex( int i ) const  
    { 
        return Itemise()[i];
    } 
    virtual int ItemiseSize() const  
    { 
        return Itemise().size();
    }
};
struct StuffKey : Key
{
    TreePtr<Node> terminus;
};




struct OverlayBase : virtual Node
{
    virtual TreePtr<Node> GetThrough() const = 0;
    virtual TreePtr<Node> GetOverlay() const = 0;    
};

template<class PRE_RESTRICTION>
struct Overlay : OverlayBase, Special<PRE_RESTRICTION>
{
    SPECIAL_NODE_FUNCTIONS
    TreePtr<PRE_RESTRICTION> through;
    TreePtr<PRE_RESTRICTION> overlay;
    virtual TreePtr<Node> GetThrough() const 
    {
        return (TreePtr<Node>)through;
    }
    virtual TreePtr<Node> GetOverlay() const
    {
        return (TreePtr<Node>)overlay;
    }
};

#endif

