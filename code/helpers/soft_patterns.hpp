#ifndef SOFT_PATTERNS_HPP
#define SOFT_PATTERNS_HPP

#include "search_replace.hpp"
#include "tree/tree.hpp"
#include "transformation.hpp"


struct NotMatchBase {}; // needed for graph plotter

// Match if the supplied patterns does not match (between you and me, it's just a NOT)
template<class PRE_RESTRICTION>
struct NotMatch : PRE_RESTRICTION,
                 RootedSearchReplace::SoftSearchPattern,
                 NotMatchBase
{
	SPECIAL_NODE_FUNCTIONS
	// Pattern is an abnormal context
    TreePtr<PRE_RESTRICTION> pattern;
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
    		Result r = sr->Compare( x, pattern, keys, false );
			TRACE("SoftNot got %d, returning the opposite!\n", (int)r);
    		if( r==NOT_FOUND )
				return FOUND;
			else
				return NOT_FOUND;
    	}
    }
};

struct MatchAllBase {};

// Match all of the supplied patterns (between you and me, it's an AND)
template<class PRE_RESTRICTION>
struct MatchAll : PRE_RESTRICTION,
                 RootedSearchReplace::SoftSearchPattern,
                 MatchAllBase
{
	SPECIAL_NODE_FUNCTIONS
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
    virtual Result DecidedCompare( const RootedSearchReplace *sr,
                                   TreePtr<Node> x,
                                   CouplingKeys *keys,
                                   bool can_key,
                                   Conjecture &conj ) const
    {
    	FOREACH( const TreePtr<PRE_RESTRICTION> i, patterns )
    	{
    		Result r = sr->DecidedCompare( x, TreePtr<Node>(i), keys, can_key, conj );
    	    if( !r )
    	    	return NOT_FOUND;
    	}
        return FOUND;
    }
};

struct MatchAnyBase {};

// Match zero or more of the supplied patterns (between you and me, it's an OR)
template<class PRE_RESTRICTION>
struct MatchAny : PRE_RESTRICTION,
                 RootedSearchReplace::SoftSearchPattern,
                 MatchAnyBase
{
	SPECIAL_NODE_FUNCTIONS
	// Patterns are an abnormal context
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
    virtual Result DecidedCompare( const RootedSearchReplace *sr,
    		                       TreePtr<Node> x,
    		                       CouplingKeys *keys,
    		                       bool can_key,
    		                       Conjecture &conj ) const
    {
    	FOREACH( const TreePtr<PRE_RESTRICTION> i, patterns )
    	{
    		Result r = sr->Compare( x, i, keys, false );
    	    if( r )
    	    	return FOUND;
    	}
        return NOT_FOUND;
    }
};


struct MatchOddBase {};

// Match an odd number of patterns (between you and me, it's an EOR)
template<class PRE_RESTRICTION>
struct MatchOdd : PRE_RESTRICTION,
                  RootedSearchReplace::SoftSearchPattern,
                  MatchOddBase
{
	SPECIAL_NODE_FUNCTIONS
	// Patterns are an abnormal context (if you are setting N==patterns.size(), then you want
	// MatchAll, whose patterns are not abnormal).
    mutable Collection<PRE_RESTRICTION> patterns; // TODO provide const iterators and remove mutable
private:
    virtual Result DecidedCompare( const RootedSearchReplace *sr,
    		                       TreePtr<Node> x,
    		                       CouplingKeys *keys,
    		                       bool can_key,
    		                       Conjecture &conj ) const
    {
    	int tot=0;
    	FOREACH( const TreePtr<PRE_RESTRICTION> i, patterns )
    	{
    		Result r = sr->Compare( x, i, keys, false );
    	    if( r )
    	    	tot++;
    	}
        return (tot%2) ? FOUND : NOT_FOUND;
    }
};


struct TransformToBase : RootedSearchReplace::SoftSearchPattern
{
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

template<class PRE_RESTRICTION>
struct TransformTo : TransformToBase, PRE_RESTRICTION
{
	SPECIAL_NODE_FUNCTIONS	
    TransformTo( Transformation *t ) : TransformToBase (t) {}
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
// TODO rename to BuildIdentifier per the docs; Consider supporting TransformTo
// in replace and having MAke/BuildIdentifier as a Transformation
struct SoftMakeIdentifier : InstanceIdentifier, 
                            RootedSearchReplace::SoftReplacePattern
{
    SoftMakeIdentifier( string s ) : format(s) {}
    SoftMakeIdentifier() : format("___UNNAMED___") {}
    NODE_FUNCTIONS
    string format;
    TreePtr<Identifier> source;
private:
    virtual TreePtr<Node> DuplicateSubtree( const RootedSearchReplace *sr,
                                               CouplingKeys *keys,
                                               bool can_key );
};

struct SoftMakeLabelIdentifier : LabelIdentifier,
                                 RootedSearchReplace::SoftReplacePattern
{
    SoftMakeLabelIdentifier( string s ) : format(s) {}
    SoftMakeLabelIdentifier() : format("___UNNAMED___") {}
    NODE_FUNCTIONS
    string format;
    TreePtr<Identifier> source;
private:
    virtual TreePtr<Node> DuplicateSubtree( const RootedSearchReplace *sr,
                                               CouplingKeys *keys,
                                               bool can_key );
};

#endif
