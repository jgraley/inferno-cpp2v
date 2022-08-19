/*
 * generate_stacks.hpp
 *
 *  Created on: 27 Oct 2009
 *      Author: jgraley
 */

#ifndef GENERATE_STACKS_HPP
#define GENERATE_STACKS_HPP

#include "tree/hastype.hpp"
#include "tree/misc.hpp"
#include "sr/vn_step.hpp"

namespace Steps {

using namespace SR;

/** Ensure an actual return statement is present in void-returning functions */
class ExplicitiseReturn : public VNStep
{
public:
	ExplicitiseReturn();
};

/** Convey return value through a module-level variable */
class UseTempForReturnValue : public VNStep
{
public:
	UseTempForReturnValue();
};

/** Pass parameters in to functions via class-level temps */
class ReturnViaTemp : public VNStep
{
public:
    ReturnViaTemp();
};

/** Add a parameter to non-process functions giving the link address */
class AddLinkAddress : public VNStep
{
public:
    AddLinkAddress();
};

/** Pass parameters in to functions via class-level temps */
class ParamsViaTemps : public VNStep
{
public:
    ParamsViaTemps();
};

/** Replace automatic variables with stacks based on arrays and 
    maintain the stack pointer correctly, supporting recursion. */
class GenerateStacks : public VNStep
{
public:
	GenerateStacks();
};

/** Merge ordinary functions into the thread that calls them -
    this is not inlining, because a function only need be merged
    once for all the call points in the thread. Turn calls and
    return into gotos. */
class MergeFunctions : public VNStep
{
public:
	MergeFunctions();
};

}; // end namespace

#endif /* GENERATE_STACKS_HPP_ */
