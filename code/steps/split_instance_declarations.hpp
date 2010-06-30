#ifndef SPLIT_INSTANCE_DECLARATIONS_HPP
#define SPLIT_INSTANCE_DECLARATIONS_HPP

#include "helpers/search_replace.hpp"

class SplitInstanceDeclarations : public InPlaceTransformation
{
public:
	using Transformation::operator();
	virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

class MergeInstanceDeclarations : public InPlaceTransformation
{
public:
	using Transformation::operator();
	virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

class HackUpIfs : public InPlaceTransformation
{
public:
	using Transformation::operator();
	virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

class CrazyNine : public InPlaceTransformation
{
public:
	using Transformation::operator();
	virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

#endif

