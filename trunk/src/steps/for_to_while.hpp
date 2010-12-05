/*
 * for_to_while.hpp
 *
 *  Created on: 26 Dec 2009
 *      Author: jgraley
 */

#ifndef FOR_TO_WHILE_HPP
#define FOR_TO_WHILE_HPP

#include "helpers/search_replace.hpp"

class ForToWhile : public InPlaceTransformation
{
public:
	using Transformation::operator();
	virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

class WhileToDo : public InPlaceTransformation
{
public:
	using Transformation::operator();
	virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

class Cleanup : public InPlaceTransformation
{
public:
	using Transformation::operator();
	virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

#endif 
