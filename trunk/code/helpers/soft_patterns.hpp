#ifndef SOFT_PATTERNS_HPP
#define SOFT_PATTERNS_HPP

#include "search_replace.hpp"
#include "typeof.hpp"

struct SoftExpressonOfType : Expression,
                             SearchReplace::SoftSearchPattern
{
    NODE_FUNCTIONS;
    SharedPtr<Type> type_pattern;
        
    virtual SearchReplace::Result DecidedCompare( const SearchReplace *sr,
    		                                      shared_ptr<Node> x,
    		                                      SearchReplace::MatchKeys *match_keys,
    		                                      SearchReplace::Conjecture &conj ) const;
};

#endif
