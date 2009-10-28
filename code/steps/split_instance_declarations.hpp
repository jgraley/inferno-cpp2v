#ifndef SPLIT_INSTANCE_DECLARATIONS_HPP
#define SPLIT_INSTANCE_DECLARATIONS_HPP

#include "helpers/search_replace.hpp"

class SplitInstanceDeclarations : public Pass
{
public:
	virtual void operator()( shared_ptr<Program> );
};

class MergeInstanceDeclarations : public Pass
{
public:
	virtual void operator()( shared_ptr<Program> );
};

class HackUpIfs : public Pass
{
public:
	virtual void operator()( shared_ptr<Program> );
};

class CrazyNine : public Pass
{
public:
	virtual void operator()( shared_ptr<Program> );
};

#endif

