#ifndef STATE_OUT_HPP
#define STATE_OUT_HPP

#include "sr/vn_transformation.hpp"

namespace Steps {

using namespace SR;

/// Make sure every SystemC wait is followed by a goto
/** This is in preparation for state-out/fall-out. We will only reorganise the 
    program around gotos, and so we need to have a goto in place to re-organise.
    This means we can't then optimise away ineffectual gotos, to do that we would
    need to mark the goto in some way, or just ignore it if preceeded by a wait */
class GotoAfterWait : public VNTransformation
{
public:
    GotoAfterWait();
};

/** conditionasl branch with no else clause gets goto in both 
    clauses - generates new label for this. */
class NormaliseConditionalGotos : public VNTransformation
{
public:
    NormaliseConditionalGotos();
};

/** conditional branch follwed by goto becomes goto with a conditional (ternary
    op) on the address. */
class CompactGotos : public VNTransformation
{
public:
    CompactGotos();
};

/** Labels not preceeded by a goto get a goto before them */
class AddGotoBeforeLabel : public VNTransformation
{
public:
    AddGotoBeforeLabel();
};

/** Insert a bootstrapping state transition at the top of the function. */
class EnsureBootstrap : public VNTransformation
{
public:
    EnsureBootstrap();
};

/** Add a variable for the current state as a label variable, and 
    use it for the gotos. */
class AddStateLabelVar : public VNTransformation
{
public:
    AddStateLabelVar();
};

/** If there is no infinite loop enclosing the whole function body
    (excluding initial assignments), insert one. */
class EnsureSuperLoop : public VNTransformation
{
public:
    EnsureSuperLoop();
};

/** If there is an infinite loop with a goto at the top of it, replace any
    identical gotos elsewhere in the loop body with continues */
class ShareGotos : public VNTransformation
{
public:
    ShareGotos();
};

/** Just move the last part of a switch statement - from the last label (not case) 
    onward out of the switch if there are no breaks */
class SwitchCleanUp : public VNTransformation
{
public:
    SwitchCleanUp();
};

/** Insert break statements whenever there are equivalant gotos */
class InferBreak : public VNTransformation
{
public:
    InferBreak();
};

/** fix fallthroughs by duplicating the fallen-into statements */
class FixFallthrough : public VNTransformation
{
public:
    FixFallthrough();
};

/** Add a flag to the state machine for yields */
class AddYieldFlag : public VNTransformation
{
public:
    AddYieldFlag();
};

/** Yield at bottom of superloop if not already yielded */
class AddInferredYield : public VNTransformation
{
public:
    AddInferredYield();
};

/** Move initial code into superloop, but only on first delta cycle */
class MoveInitIntoSuperLoop : public VNTransformation
{
public:
    MoveInitIntoSuperLoop();
};

/** Optimise loops that contain yields by rotating until yield reaches the bottom */
class LoopRotation : public VNTransformation
{
public:
    LoopRotation();
};

}; // end namespace

#endif

