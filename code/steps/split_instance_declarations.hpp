#ifndef SPLIT_INSTANCE_DECLARATIONS_HPP
#define SPLIT_INSTANCE_DECLARATIONS_HPP

#include "helpers/search_replace.hpp"

class SplitInstanceDeclarations : public Transformation
{
public:
	virtual void operator()( shared_ptr<Node> context, shared_ptr<Node> root );
};

class MergeInstanceDeclarations : public Transformation
{
public:
	virtual void operator()( shared_ptr<Node> context, shared_ptr<Node> root );
};

class HackUpIfs : public Transformation
{
public:
	virtual void operator()( shared_ptr<Node> context, shared_ptr<Node> root );
};

class CrazyNine : public Transformation
{
public:
	virtual void operator()( shared_ptr<Node> context, shared_ptr<Node> root );
};

#endif

