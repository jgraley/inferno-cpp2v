#ifndef SOFT_PATTERNS_HPP
#define SOFT_PATTERNS_HPP

#include "search_replace.hpp"
#include "typeof.hpp"

template<class VALUE_TYPE>
struct SoftNot : VALUE_TYPE,
                 RootedSearchReplace::SoftSearchPattern
{
    NODE_FUNCTIONS
    SharedPtr<VALUE_TYPE> pattern;
private:
    virtual RootedSearchReplace::Result DecidedCompare( const RootedSearchReplace *sr,
    		                                            shared_ptr<Node> x,
    		                                            RootedSearchReplace::CouplingKeys *keys,
    		                                            bool can_key,
    		                                            RootedSearchReplace::Conjecture &conj ) const
    {
    	if( keys && can_key )
    	{
    		// Don't do a subtree search while keying - we'll only end up keying the wrong thing
    		// or terminating with NOT_FOUND prematurely
    		return RootedSearchReplace::FOUND;
    	}
    	else
    	{
    	    // Do not use the present conjecture since we would mess it up because
    	    // a. We didn't recurse during KEYING pass and
    	    // b. Search under not can terminate with NOT_FOUND, but parent search will continue
    	    // Consequently, we go in at Compare level, which creates a new conjecture. 
    		SearchReplace::Result r = sr->Compare( x, shared_ptr<Node>(pattern), keys, false );
			TRACE("SoftNot got %d, returning the opposite!\n", (int)r);
    		if( r==RootedSearchReplace::NOT_FOUND )
				return RootedSearchReplace::FOUND;
			else
				return RootedSearchReplace::NOT_FOUND;
    	}
    }
};

template<class VALUE_TYPE>
struct SoftAnd : VALUE_TYPE,
                 RootedSearchReplace::SoftSearchPattern
{
    NODE_FUNCTIONS
    mutable Collection<VALUE_TYPE> patterns; // TODO provide const iterators and remove mutable
private:
    virtual RootedSearchReplace::Result DecidedCompare( const RootedSearchReplace *sr,
    		                                            shared_ptr<Node> x,
    		                                            RootedSearchReplace::CouplingKeys *keys,
    		                                            bool can_key,
    		                                            RootedSearchReplace::Conjecture &conj ) const
    {
    	typedef GenericContainer::iterator iter; // TODO clean up this loop
    	iter it;
    	int i;
    	for( it = patterns.begin(), i = 0;
    		 it != patterns.end();
    		 ++it, ++i )
    	{
    		RootedSearchReplace::Result r = sr->DecidedCompare( x, shared_ptr<Node>(*it), keys, can_key, conj );
    		TRACE("AND[%d] got %d\n", i, r);
    	    if( !r )
    	    	return RootedSearchReplace::NOT_FOUND;
    	}

        return RootedSearchReplace::FOUND;
    }
};


struct SoftExpressonOfType : Expression,
                             RootedSearchReplace::SoftSearchPattern
{
    NODE_FUNCTIONS
    SharedPtr<Type> type_pattern;
private:
    virtual RootedSearchReplace::Result DecidedCompare( const RootedSearchReplace *sr,
    		                                            shared_ptr<Node> x,
    		                                            RootedSearchReplace::CouplingKeys *keys,
    		                                            bool can_key,
    		                                            RootedSearchReplace::Conjecture &conj ) const;
};

struct SoftIdentifierOfInstance : InstanceIdentifier,
                                  RootedSearchReplace::SoftSearchPattern
{
    NODE_FUNCTIONS
    SharedPtr<Instance> decl_pattern;
private:
    virtual RootedSearchReplace::Result DecidedCompare( const RootedSearchReplace *sr,
    		                                            shared_ptr<Node> x,
    		                                            RootedSearchReplace::CouplingKeys *keys,
    		                                            bool can_key,
    		                                            RootedSearchReplace::Conjecture &conj ) const;
};

// Make an identifer based on an existing one. New identfier is named using
// sprintf( format, source->name )
// You must key the source identifier to somehting in the search pattern (so it
// will get substitued to the real Specific identifier found in the tree) and
// you may substitute the SoftMakeIdentifier in the replace pattern if you need
// to specifiy it in more than one place. Note that SoftMakeIdentifier is stateless
// and cannot therefore keep track of uniqueness - you'll get a new one each time
// and must rely on a replace match set to get multiple reference to the same
// new identifier. Rule is: ONE of these per new identifier.
struct SoftMakeIdentifier : InstanceIdentifier, // TODO other kinds of identifier
                            RootedSearchReplace::SoftReplacePattern
{
	SoftMakeIdentifier( string s ) : format(s) {}
	SoftMakeIdentifier() : format("___UNNAMED___") {}
	NODE_FUNCTIONS
	string format;
	SharedPtr<Identifier> source;
private:
    virtual shared_ptr<Node> DuplicateSubtree( const RootedSearchReplace *sr,
    		                                   RootedSearchReplace::CouplingKeys *keys,
    		                                   bool can_key );
};


#endif
