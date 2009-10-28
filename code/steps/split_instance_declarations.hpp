#ifndef SPLIT_INSTANCE_DECLARATIONS_HPP
#define SPLIT_INSTANCE_DECLARATIONS_HPP

#include "helpers/search_replace.hpp"

class SplitInstanceDeclarations : public Pass
{
public:
	SplitInstanceDeclarations();
	virtual void operator()( shared_ptr<Program> );
};

class MergeInstanceDeclarations : public Pass
{
public:
	MergeInstanceDeclarations();
	virtual void operator()( shared_ptr<Program> );
};

class HackUpIfs : public Pass
{
public:
	HackUpIfs();
	virtual void operator()( shared_ptr<Program> );
};

class CrazyNine : public Pass
{
public:
	CrazyNine();
	virtual void operator()( shared_ptr<Program> );
};

#endif

