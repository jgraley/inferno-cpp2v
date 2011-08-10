#ifndef STATE_OUT_HPP
#define STATE_OUT_HPP

#include "sr/search_replace.hpp"

namespace Steps {

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

/** Labels not preceeded by a goto get a goto before them */
class AddGotoBeforeLabel : public SearchReplace
{
public:
    AddGotoBeforeLabel();
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

/** Just move the last part of a switch statement - from the last label (not case) 
    onward out of the switch if there are no breaks */
class SwitchCleanUp : public SearchReplace
{
public:
    SwitchCleanUp();
};

/** Insert break statements whenever there are equivalant gotos */
class InferBreak : public SearchReplace
{
public:
    InferBreak();
};

/** fix fallthroughs by duplicating the fallen-into statements */
class FixFallthrough : public SearchReplace
{
public:
    FixFallthrough();
};

/** Go to fallthrough machine style */
class MakeFallThroughMachine : public SearchReplace
{
public:
    MakeFallThroughMachine();
};

/** Add a flag to the state machine for yields */
class AddYieldFlag : public SearchReplace
{
public:
    AddYieldFlag();
};

/** Yield at bottom of superloop if not already yielded */
class AddInferredYield : public SearchReplace
{
public:
    AddInferredYield();
};

/** Move initial code into superloop, but only on first delta cycle */
class MoveInitIntoSuperLoop : public SearchReplace
{
public:
    MoveInitIntoSuperLoop();
};

}; // end namespace

#endif

