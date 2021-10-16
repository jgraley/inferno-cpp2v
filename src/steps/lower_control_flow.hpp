#ifndef LOWER_CONTROL_FLOW_HPP
#define LOWER_CONTROL_FLOW_HPP

#include "sr/search_replace.hpp"
#include "sr/vn_transformation.hpp"

namespace Steps {

using namespace SR;

/** Detect an uncombable switch and mark it for lowering. This is 
    any switch with fall throughs.  */
class DetectUncombableSwitch : public VNTransformation
{
public:
    DetectUncombableSwitch();
};
        
/** Make all fors uncombable to make next step easier */
class MakeAllForUncombable : public SearchReplace
{
public:
    MakeAllForUncombable();
};
        
/** Detect a combable for and mark it for lowering. This is 
    any indefinite for. */
class DetectCombableFor : public SearchReplace
{
public:
    DetectCombableFor();
};
        
/** Make all breaks uncombable to make next step easier */
class MakeAllBreakUncombable : public SearchReplace
{
public:
    MakeAllBreakUncombable();
};
        
/** Detect a combable break and mark it for lowering. This is 
    a break at the top level of a combable switch. */
class DetectCombableBreak : public SearchReplace
{
public:
    DetectCombableBreak();
};
        
/** Convert for loops into while loops, preserving correct 
    behaviour of continue, which always means "jump to the 
    bottom of the body" */
class ForToWhile : public SearchReplace
{
public:
    ForToWhile();
};

/** Convert While loops to Do loops */
class WhileToDo : public SearchReplace
{
public:
    WhileToDo();
};

/** Lower general if statements into a simplified form of if(x) goto y; */
class IfToIfGoto : public SearchReplace
{
public:
    IfToIfGoto();
};

/** Eliminate switch statements by replacing them with gotos,
    conditional gotos (if(x) goto Y;) and labels, supporting
    fall-through, default and GCC range-case extension. */
class SwitchToIfGoto : public SearchReplace
{
public:
    SwitchToIfGoto();
};

/** Eliminate Do loops by replacing them with the conditional
    goto pattern. Supports continue. */
class DoToIfGoto : public SearchReplace
{
public:
    DoToIfGoto();
};

/** Eliminate break statements by replacing them with gotos to
    the end of the relevent block. */
class BreakToGoto : public SearchReplace
{
public:
    BreakToGoto();
};

/** Reduce || to an if, due to its conditional execution and sequence point behaviour */
class LogicalOrToIf : public SearchReplace
{
public:
    LogicalOrToIf();
};

/** Reduce && to an if, due to its conditional execution and sequence point behaviour */
class LogicalAndToIf : public SearchReplace
{
public:
    LogicalAndToIf();
};

/** Reduce ?: to an if, due to its conditional execution and sequence point behaviour */
class ConditionalOperatorToIf : public SearchReplace
{
public:
    ConditionalOperatorToIf();
};

/** Evaluate the arguments to a funciton into temps first, and then pass the temps
    into the call - done in case an argument itself contains a function call. */
class ExtractCallParams : public SearchReplace
{
public:
    ExtractCallParams();
};

}; // end namespace

#endif 

