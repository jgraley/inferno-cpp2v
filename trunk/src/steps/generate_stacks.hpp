/*
 * generate_stacks.hpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#ifndef GENERATE_STACKS_HPP
#define GENERATE_STACKS_HPP

#include "helpers/search_replace.hpp"

class GenerateStacks : public InPlaceTransformation
{
public:
	using Transformation::operator();
	virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

#endif /* GENERATE_STACKS_HPP_ */
