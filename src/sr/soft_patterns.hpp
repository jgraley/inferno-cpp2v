#ifndef SOFT_PATTERNS_HPP
#define SOFT_PATTERNS_HPP

#include "search_replace.hpp"
#include "tree/tree.hpp"
#include "helpers/transformation.hpp"


struct NotMatchBase {}; // needed for graph plotter

// Match if the supplied patterns does not match (between you and me, it's just a NOT)
template<class PRE_RESTRICTION>
struct NotMatch : Special<PRE_RESTRICTION>,
                 CompareReplace::SoftSearchPattern,
                 NotMatchBase
{
	SPECIAL_NODE_FUNCTIONS
	// Pattern is an abnormal context
    TreePtr<PRE_RESTRICTION> pattern;
private:
    virtual Result DecidedCompare( const CompareReplace *sr,
    		                       TreePtr<Node> x,
    		                       bool can_key,
    		                       Conjecture &conj ) const
    {
        ASSERT( pattern );
    	if( can_key )
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
    		Result r = sr->Compare( x, pattern, false );
			TRACE("SoftNot got %d, returning the opposite!\n", (int)r);
    		if( r==NOT_FOUND )
				return FOUND;
			else
				return NOT_FOUND;
    	}
    }
};

struct MatchAllBase : virtual Node
{
    virtual CollectionInterface &GetPatterns() = 0;
};

// Match all of the supplied patterns (between you and me, it's an AND)
template<class PRE_RESTRICTION>
struct MatchAll : Special<PRE_RESTRICTION>,
                 CompareReplace::SoftSearchPattern,
                 MatchAllBase
{
	SPECIAL_NODE_FUNCTIONS
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
    virtual Result DecidedCompare( const CompareReplace *sr,
                                   TreePtr<Node> x,
                                   bool can_key,
                                   Conjecture &conj ) const
    {
    	FOREACH( const TreePtr<PRE_RESTRICTION> i, patterns )
    	{
    	    ASSERT( i );
    		Result r = sr->DecidedCompare( x, TreePtr<Node>(i), can_key, conj );
    	    if( !r )
    	    	return NOT_FOUND;
    	}
        return FOUND;
    }
    CollectionInterface &GetPatterns() { return patterns; } // TODO try covariant?
};

struct MatchAnyBase {};

// Match zero or more of the supplied patterns (between you and me, it's an OR)
template<class PRE_RESTRICTION>
struct MatchAny : Special<PRE_RESTRICTION>,
                 CompareReplace::SoftSearchPattern,
                 MatchAnyBase
{
	SPECIAL_NODE_FUNCTIONS
	// Patterns are an abnormal context
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
    virtual Result DecidedCompare( const CompareReplace *sr,
    		                       TreePtr<Node> x,
    		                       bool can_key,
    		                       Conjecture &conj ) const
    {
    	FOREACH( const TreePtr<PRE_RESTRICTION> i, patterns )
    	{
    	    ASSERT( i );
    		Result r = sr->Compare( x, i, false );
    	    if( r )
    	    	return FOUND;
    	}
        return NOT_FOUND;
    }
};


struct MatchOddBase {};

// Match an odd number of patterns (between you and me, it's an EOR)
template<class PRE_RESTRICTION>
struct MatchOdd : Special<PRE_RESTRICTION>,
                  CompareReplace::SoftSearchPattern,
                  MatchOddBase
{
	SPECIAL_NODE_FUNCTIONS
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
    virtual Result DecidedCompare( const CompareReplace *sr,
    		                       TreePtr<Node> x,
    		                       bool can_key,
    		                       Conjecture &conj ) const
    {
    	int tot=0;
    	FOREACH( const TreePtr<PRE_RESTRICTION> i, patterns )
    	{
    	    ASSERT( i );
    	    Result r = sr->Compare( x, i, false );
    	    if( r )
    	    	tot++;
    	}
        return (tot%2) ? FOUND : NOT_FOUND;
    }
};


struct TransformOfBase : CompareReplace::SoftSearchPattern 
{
    TreePtr<Node> pattern; 
    Transformation *transformation;
    TransformOfBase( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) :
    	transformation(t),
    	pattern(p)
    {
    }

private:
    virtual Result DecidedCompare( const CompareReplace *sr,
    		                                            TreePtr<Node> x,
    		                                            bool can_key,
    		                                            Conjecture &conj ) const;
};

template<class PRE_RESTRICTION>
struct TransformOf : TransformOfBase, Special<PRE_RESTRICTION>
{
	SPECIAL_NODE_FUNCTIONS	
    TransformOf( Transformation *t, TreePtr<Node> p=TreePtr<Node>() ) : 
        TransformOfBase(t, p) 
    {
    }
};


// Make an identifer based on an existing one. New identfier is named using
// sprintf( format, source->name )
// You must key the source identifier to somehting in the search pattern (so it
// will get substitued to the real Specific identifier found in the tree) and
// you may couple the SoftMakeIdentifier in the replace pattern if you need
// to specify it in more than one place. Note that SoftMakeIdentifier is stateless
// and cannot therefore keep track of uniqueness - you'll get a new one each time
// and must rely on a replace coupling to get multiple reference to the same
// new identifier. Rule is: ONE of these per new identifier.

// TODO allow multiple sources for the printf, use in eg merging successive labels
// TODO do this via a transformation as with TransformOf/TransformOf
struct BuildIdentifierBase : CompareReplace::SoftReplacePattern
{
    BuildIdentifierBase( string s ) : format(s) {}
    TreePtr<Identifier> source;
    string GetNewName( const CompareReplace *sr );
    string format;
};

struct BuildInstanceIdentifier : Special<InstanceIdentifier>,                             
                                 BuildIdentifierBase
{
    BuildInstanceIdentifier( string s ) : BuildIdentifierBase(s) {}
    BuildInstanceIdentifier() : BuildIdentifierBase("unnamed") {}
    SPECIAL_NODE_FUNCTIONS
private:
    virtual TreePtr<Node> DuplicateSubtree( const CompareReplace *sr )
    {
	string newname = GetNewName( sr );
	return TreePtr<SpecificInstanceIdentifier>( new SpecificInstanceIdentifier( newname ) );
    }                                               
};

struct BuildLabelIdentifier : Special<LabelIdentifier>,                             
                              BuildIdentifierBase
{
    BuildLabelIdentifier( string s ) : BuildIdentifierBase(s) {}
    BuildLabelIdentifier() : BuildIdentifierBase("UNNAMED") {}
    SPECIAL_NODE_FUNCTIONS
private:
    virtual TreePtr<Node> DuplicateSubtree( const CompareReplace *sr )
    {
	string newname = GetNewName( sr );
	return TreePtr<SpecificLabelIdentifier>( new SpecificLabelIdentifier( newname ) );
    }                                               
};

#endif
