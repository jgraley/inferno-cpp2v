#ifndef SOFT_PATTERNS_HPP
#define SOFT_PATTERNS_HPP

#include "search_replace.hpp"
#include "typeof.hpp"

template<class VALUE_TYPE>
struct SoftNot : VALUE_TYPE,
                 SearchReplace::SoftSearchPattern
{
    NODE_FUNCTIONS
    SharedPtr<VALUE_TYPE> pattern;
private:
    virtual SearchReplace::Result DecidedCompare( const SearchReplace *sr,
    		                                      shared_ptr<Node> x,
    		                                      SearchReplace::MatchKeys *match_keys,
    		                                      SearchReplace::Conjecture &conj,
    		                                      unsigned context_flags ) const
    {
    	if( match_keys && match_keys->pass == SearchReplace::MatchKeys::KEYING )
    	{
    		// Don't do a subtree search while keying - we'll only end up keying the wrong thing
    		// or terminating with NOT_FOUND prematurely
    		return SearchReplace::FOUND;
    	}
    	else
    	{
    		SearchReplace::Result r = sr->DecidedCompare( x, pattern, match_keys, conj, context_flags );
			TRACE("SoftNot got %d, returning the opposite!\n", (int)r);
    		if( r==SearchReplace::NOT_FOUND )
				return SearchReplace::FOUND;
			else
				return SearchReplace::NOT_FOUND;
    	}
    }
};

struct SoftExpressonOfType : Expression,
                             SearchReplace::SoftSearchPattern
{
    NODE_FUNCTIONS
    SharedPtr<Type> type_pattern;
private:
    virtual SearchReplace::Result DecidedCompare( const SearchReplace *sr,
    		                                      shared_ptr<Node> x,
    		                                      SearchReplace::MatchKeys *match_keys,
    		                                      SearchReplace::Conjecture &conj,
    		                                      unsigned context_flags ) const;
};

#endif
