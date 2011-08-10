/*
 * generate_stacks.hpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#ifndef GENERATE_STACKS_HPP
#define GENERATE_STACKS_HPP

#include "sr/search_replace.hpp"
#include "tree/typeof.hpp"
#include "tree/misc.hpp"

namespace Steps {

class ExplicitiseReturn : public SearchReplace
{
public:
	ExplicitiseReturn();
};

class UseTempsForParamsReturn : public SearchReplace
{
public:
	UseTempsForParamsReturn();
};

// Replace automatic variables with stacks based on arrays and 
// maintain the stack pointer correctly, supporting recursion.
class GenerateStacks : public SearchReplace
{
public:
	GenerateStacks();
};

// Find the (illegal) construct of a member variable declared
// in a member function and move out into the enclosing class.
class ExtractFields : public SearchReplace
{
public:
	ExtractFields();
};

}; // end namespace

#endif /* GENERATE_STACKS_HPP_ */
