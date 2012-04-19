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
class LabelVarsToEnum : public CompareReplace
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

/// Eliminate all but the last goto by placing state bodies under if
class ApplyGotoPolicy : public SearchReplace
{
public:
     ApplyGotoPolicy();
};

/// Deal with a state at the end that is not followed by a goto and which
/// will therefore end up exiting off the bottom of the function (absent a
/// return or terminator). Create a conditional goto so that exit can occur.
class ApplyGotoPolicyBottom : public SearchReplace
{
public:
     ApplyGotoPolicyBottom();
};

/// Group all labels at the top by placing state bodies under if
class ApplyLabelPolicy : public SearchReplace
{
public:
     ApplyLabelPolicy();
};

/// Move code above the uppermost label under the label but conditional
/// on the delta count being zero, i.e. no waits have occurred yet.
class ApplyTopPolicy : public SearchReplace
{
public:
     ApplyTopPolicy();
};

}; // end namespace

#endif

