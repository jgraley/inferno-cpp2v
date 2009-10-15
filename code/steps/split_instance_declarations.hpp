#ifndef SPLIT_INSTANCE_DECLARATIONS_HPP
#define SPLIT_INSTANCE_DECLARATIONS_HPP

#include "helpers/search_replace.hpp"

class SplitInstanceDeclarations : public Pass
{
public:
	SplitInstanceDeclarations();
	virtual void operator()( shared_ptr<Program> );

private:
	set<SearchReplace::MatchSet> sms0;
	set<SearchReplace::MatchSet> sms1;
	SearchReplace sr0;
	SearchReplace sr1;
};

class MergeInstanceDeclarations : public Pass
{
public:
	MergeInstanceDeclarations();
	virtual void operator()( shared_ptr<Program> );

private:
	set<SearchReplace::MatchSet> sms1;
	SearchReplace sr1;
};

#endif

