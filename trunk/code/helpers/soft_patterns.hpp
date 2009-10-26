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
    		                                            RootedSearchReplace::MatchKeys *match_keys,
    		                                            RootedSearchReplace::Conjecture &conj,
    		                                            unsigned context_flags ) const
    {
    	if( match_keys && match_keys->pass == RootedSearchReplace::MatchKeys::KEYING )
    	{
    		// Don't do a subtree search while keying - we'll only end up keying the wrong thing
    		// or terminating with NOT_FOUND prematurely
    		return RootedSearchReplace::FOUND;
    	}
    	else
    	{
    		SearchReplace::Result r = sr->DecidedCompare( x, pattern, match_keys, conj, context_flags );
			TRACE("SoftNot got %d, returning the opposite!\n", (int)r);
    		if( r==RootedSearchReplace::NOT_FOUND )
				return RootedSearchReplace::FOUND;
			else
				return RootedSearchReplace::NOT_FOUND;
    	}
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
    		                                            RootedSearchReplace::MatchKeys *match_keys,
    		                                            RootedSearchReplace::Conjecture &conj,
    		                                            unsigned context_flags ) const;
};

#endif
