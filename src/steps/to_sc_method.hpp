#ifndef TO_SC_METHOD_HPP
#define TO_SC_METHOD_HPP

#include "sr/vn_transformation.hpp"

/// Steps namespace contains all of inferno's transformation steps. 
namespace Steps {

using namespace SR;

/** Move automatic variables out to class scope as fields, but only
    when the compound block they are declared in does not make outgoing 
    calls which could lead to recursion. */
class AutosToModule : public VNTransformation
{
public:
    AutosToModule();
};

/** Move temp and static variables found in funcitons out into the enclosing class */
class TempsAndStaticsToModule : public VNTransformation
{
public:
    TempsAndStaticsToModule();
};

/** Move declarations found in funcitons out into the enclosing class */
class DeclsToModule : public VNTransformation
{
public:
    DeclsToModule();
};

/** Convertsion from Thread to Method. We require that there are 
    no local variables in the Thread, and that at it's top level it contains
    a do { ... } while(true) loop. We simply lose the loop, rename the 
    Thread to Module, and rename all WaitX to NextTriggerX. I think it's
    also required that every iteration of superloop calls Wait exactly once. */
class ThreadToMethod : public VNTransformation
{
public:
    ThreadToMethod();
};

/// Detect return statements and arrange for the function to fall down
/// to the bottom without doing anything more. only for void functions and
/// loops not supported.
class ExplicitiseReturns : public VNTransformation
{
public:
    ExplicitiseReturns();
};

}; // end namespace

#endif

