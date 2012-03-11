#ifndef LOWER_CONTROL_FLOW_HPP
#define LOWER_CONTROL_FLOW_HPP

#include "sr/search_replace.hpp"

namespace Steps {

/// Make sure every SystemC wait is followed by a goto
/** This is in preparation for state-out/fall-out. We will only reorganise the 
    program around gotos, and so we need to have a goto in place to re-organise.
    This means we can't then optimise away ineffectual gotos, to do that we would
    need to mark the goto in some way, or just ignore it if preceeded by a wait */
class GotoAfterWait : public SearchReplace
{
public:
    GotoAfterWait();
};

// Lower general if statements into a simplified form of if(x) goto y;
class IfToIfGoto : public SearchReplace
{
public:
    IfToIfGoto();
};

// Eliminate switch statements by replacing them with gotos,
// conditional gotos (if(x) goto Y;) and labels, supporting
// fall-through, default and GCC range-case extension.
class SwitchToIfGoto : public SearchReplace
{
public:
    SwitchToIfGoto();
};

// Eliminate Do loops by replacing them with the conditional
// goto pattern. Supports continue.
class DoToIfGoto : public SearchReplace
{
public:
    DoToIfGoto();
};

// Eliminate break statements by replacing them with gotos to
// the end of the relevent block. 
class BreakToGoto : public SearchReplace
{
public:
    BreakToGoto();
};

// Reduce || to an if, due to its conditional execution and sequence point behaviour
class LogicalOrToIf : public SearchReplace
{
public:
    LogicalOrToIf();
};

// Reduce && to an if, due to its conditional execution and sequence point behaviour
class LogicalAndToIf : public SearchReplace
{
public:
    LogicalAndToIf();
};

// Reduce ?: to an if, due to its conditional execution and sequence point behaviour
class MultiplexorToIf : public SearchReplace
{
public:
    MultiplexorToIf();
};

class ExtractCallParams : public SearchReplace
{
public:
    ExtractCallParams();
};



}; // end namespace

#endif 

