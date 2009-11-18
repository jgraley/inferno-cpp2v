/*
 * generate_stacks.hpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#ifndef GENERATE_STACKS_HPP
#define GENERATE_STACKS_HPP

#include "helpers/search_replace.hpp"

class GenerateStacks : public Transformation
{
public:
	virtual void operator()( shared_ptr<Node> context, shared_ptr<Node> root );
};

#endif /* GENERATE_STACKS_HPP_ */
