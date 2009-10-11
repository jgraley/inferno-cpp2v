#ifndef SPLIT_INSTANCE_DECLARATIONS_HPP
#define SPLIT_INSTANCE_DECLARATIONS_HPP

#include "helpers/search_replace.hpp"

class SplitInstanceDeclarations : public SearchReplace
{
public:
	SplitInstanceDeclarations();

private:
	set<MatchSet> sms;
};

#endif

