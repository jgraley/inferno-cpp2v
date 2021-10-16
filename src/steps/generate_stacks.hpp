/*
 * generate_stacks.hpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#ifndef GENERATE_STACKS_HPP
#define GENERATE_STACKS_HPP

#include "tree/typeof.hpp"
#include "tree/misc.hpp"
#include "sr/vn_transformation.hpp"

namespace Steps {

using namespace SR;

/** Ensure an actual return statement is present in void-returning functions */
class ExplicitiseReturn : public VNTransformation
{
public:
	ExplicitiseReturn();
};

/** Convey return value through a module-level variable */
class UseTempForReturnValue : public VNTransformation
{
public:
	UseTempForReturnValue();
};

/** Pass parameters in to functions via class-level temps */
class ReturnViaTemp : public VNTransformation
{
public:
    ReturnViaTemp();
};

/** Add a parameter to non-process functions giving the link address */
class AddLinkAddress : public VNTransformation
{
public:
    AddLinkAddress();
};

/** Pass parameters in to functions via class-level temps */
class ParamsViaTemps : public VNTransformation
{
public:
    ParamsViaTemps();
};

/** Replace automatic variables with stacks based on arrays and 
    maintain the stack pointer correctly, supporting recursion. */
class GenerateStacks : public VNTransformation
{
public:
	GenerateStacks();
};

/** Merge ordinary functions into the thread that calls them -
    this is not inlining, because a function only need be merged
    once for all the call points in the thread. Turn calls and
    return into gotos. */
class MergeFunctions : public VNTransformation
{
public:
	MergeFunctions();
};

}; // end namespace

#endif /* GENERATE_STACKS_HPP_ */
