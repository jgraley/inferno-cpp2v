#ifndef LOWER_CONTROL_FLOW_HPP
#define LOWER_CONTROL_FLOW_HPP

#include "helpers/search_replace.hpp"

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

#endif 
