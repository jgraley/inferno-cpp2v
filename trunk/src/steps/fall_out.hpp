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

/// Change all types from Labeley to the state enum, and move lmap lookups from && to goto
class LabelTypeToEnum : public SearchReplace
{
public:
    LabelTypeToEnum();
};

/// Variable-by-variable, change type from Labeley to the state enum, and move lmap lookups though the variable
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

/// Eliminate all but the last goto by placing state bodies under if. Only
/// act on states that are combable i.e. do not yield
class ApplyCombGotoPolicy : public SearchReplace
{
public:
     ApplyCombGotoPolicy();
};

class ApplyYieldGotoPolicy : public SearchReplace
{
public:
     ApplyYieldGotoPolicy();
};

/// Deal with a state at the end that is not followed by a goto and which
/// will therefore end up exiting off the bottom of the function (absent a
/// return or terminator). Create a conditional goto so that exit can occur.
class ApplyBottomPolicy : public SearchReplace
{
public:
     ApplyBottomPolicy();
};

/// Group all labels at the top by placing state bodies under if
class ApplyLabelPolicy : public SearchReplace
{
public:
     ApplyLabelPolicy();
};

/// Move code above the uppermost label under the label but conditional
/// on the delta count being zero, i.e. no yields have occurred yet. Only
/// works if there is already a yield before the first goto 
class ApplyTopPolicy : public SearchReplace
{
public:
     ApplyTopPolicy();
};

/// Ensure we always yield before the first goto (since reset does not
/// comb into non-reset code).
class EnsureResetYield : public SearchReplace
{
public:
     EnsureResetYield();
};

/// Detect a superloop formed from the remaingin gotos. There must be only one
/// label, and there must be nothing above or below the superloop. Intermediate
/// gotos become continues. Parameter chooses whether to handle conditional goto
/// at the bottom. You always want to run this with false, and again with true
/// if you want to support exiting the loop.
class DetectSuperLoop : public SearchReplace
{
public:
     DetectSuperLoop( bool is_conditional_goto );
};

class InsertInferredYield : public SearchReplace
{
public:
     InsertInferredYield();
};

/*
class RemoveLabelSubscript : public SearchReplace
{
public:
     RemoveLabelSubscript();
};

class LabelInstanceToEnum : public SearchReplace
{
public:
     LabelInstanceToEnum();
};
*/
}; // end namespace

#endif

