/*
 * generate_stacks.hpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#ifndef GENERATE_STACKS_HPP
#define GENERATE_STACKS_HPP

#include "helpers/search_replace.hpp"

class GenerateStacks
{
public:
	GenerateStacks();
	virtual void operator()( shared_ptr<Program> );
};

#endif /* GENERATE_STACKS_HPP_ */
