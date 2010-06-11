#ifndef SPLIT_INSTANCE_DECLARATIONS_HPP
#define SPLIT_INSTANCE_DECLARATIONS_HPP

#include "helpers/search_replace.hpp"

class SplitInstanceDeclarations : public Transformation
{
public:
	virtual void operator()( SharedPtr<Node> context, SharedPtr<Node> *proot );
};

class MergeInstanceDeclarations : public Transformation
{
public:
	virtual void operator()( SharedPtr<Node> context, SharedPtr<Node> *proot );
};

class HackUpIfs : public Transformation
{
public:
	virtual void operator()( SharedPtr<Node> context, SharedPtr<Node> *proot );
};

class CrazyNine : public Transformation
{
public:
	virtual void operator()( SharedPtr<Node> context, SharedPtr<Node> *proot );
};

#endif

