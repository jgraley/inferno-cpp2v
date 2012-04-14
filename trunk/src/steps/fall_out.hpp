#ifndef FALL_OUT_HPP
#define FALL_OUT_HPP

#include "sr/search_replace.hpp"

namespace Steps {

/// Put all the labels in an array indexable by an enum
class PlaceLabelsInArray : public SearchReplace
{
public:
    PlaceLabelsInArray();
};

/// Put all the labels in an array indexable by an enum
class LabelVarsToEnum : public SearchReplace
{
public:
    LabelVarsToEnum();
};

/// Find c ? a[i] : a[j] and replace with a[ c ? i : j ]
class SwapSubscriptMultiplex : public SearchReplace
{
public:
    SwapSubscriptMultiplex();
};

/// Insert state variable as an enum
class AddStateEnumVar : public SearchReplace
{
public:
    AddStateEnumVar();
};

}; // end namespace

#endif

