#ifndef LOWER_CONTROL_FLOW_HPP
#define LOWER_CONTROL_FLOW_HPP

#include "helpers/search_replace.hpp"

class IfToIfGoto : public SearchReplace
{
public:
    IfToIfGoto();
};

class SwitchToIfGoto : public SearchReplace
{
public:
    SwitchToIfGoto();
};

class DoToIfGoto : public SearchReplace
{
public:
    DoToIfGoto();
};

class BreakToGoto : public SearchReplace
{
public:
    BreakToGoto();
};

#endif 
