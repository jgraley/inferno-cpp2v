/*
 * generate_stacks.hpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#ifndef GENERATE_STACKS_HPP
#define GENERATE_STACKS_HPP

#include "sr/search_replace.hpp"

// Replace automatic variables with staks based on arrays and 
// maintain the stack pointer correctly, supporting recursion.
class GenerateStacks : public SearchReplace
{
public:
	GenerateStacks();
};

class ExplicitiseReturn : public SearchReplace
{
public:
	ExplicitiseReturn();
};

#endif /* GENERATE_STACKS_HPP_ */
