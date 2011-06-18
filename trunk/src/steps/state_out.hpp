#ifndef STATE_OUT_HPP
#define STATE_OUT_HPP

#include "sr/search_replace.hpp"

/** conditional branch follwed by goto becomes goto with a conditional (ternary
    op) on the address. */
class CompactGotos : public SearchReplace
{
public:
    CompactGotos();
};

/** any conditional branch becomes goto with a conditional (ternary
    op) on the address - generates new label for this. */
class CompactGotosFinal : public SearchReplace
{
public:
    CompactGotosFinal();
};

/** Insert a bootstrapping state transition at the top of the function. */
class EnsureBootstrap : public SearchReplace
{
public:
    EnsureBootstrap();
};

/** Add a variable for the current state as a label variable, and 
    use it for the gotos. */
class AddStateLabelVar : public SearchReplace
{
public:
    AddStateLabelVar();
};

/** If there is no infinite loop enclosing the whole function body
    (excluding initial assignments), insert one. */
class EnsureSuperLoop : public SearchReplace
{
public:
    EnsureSuperLoop();
};

/** If there is an infinite loop with a goto at the top of it, replace any
    identical gotos elsewhere in the loop body with continues */
class ShareGotos : public SearchReplace
{
public:
    ShareGotos();
};

/** If there's a structure of gotos and labels that looks like a switch statement
    then insert a switch statement, turn labels into enums and the type of the
    control variable from void * to the new enum */
class InsertSwitch : public SearchReplace
{
public:
    InsertSwitch();
};


#endif

