#ifndef STATE_OUT_HPP
#define STATE_OUT_HPP

#include "sr/search_replace.hpp"

// conditional branch follwed by goto becomes goto with a conditional (ternary
// op) on the address.
class CompactGotos : public SearchReplace
{
public:
    CompactGotos();
};

// any conditional branch becomes goto with a conditional (ternary
// op) on the address - generates new label for this.
class CompactGotosFinal : public SearchReplace
{
public:
    CompactGotosFinal();
};

// Add a variable for the current state as a label variable, and 
// use it for the gotos.
class AddStateLabelVar : public SearchReplace
{
public:
    AddStateLabelVar();
};


#endif
