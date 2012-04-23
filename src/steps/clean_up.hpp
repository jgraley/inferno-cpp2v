/*
 * clean_up.hpp
 *
 *  Created on: 26 Dec 2009
 *      Author: jgraley
 */

#ifndef CLEAN_UP_HPP
#define CLEAN_UP_HPP

#include "sr/search_replace.hpp"

namespace Steps {

/// Find compound statements inside compund statements and flatten
class CleanupCompoundExpression : public SearchReplace
{    
public:
    CleanupCompoundExpression();
};
   
/// Find compound statements inside compund statements and flatten
class CleanupCompoundMulti : public SearchReplace
{    
public:
    CleanupCompoundMulti();
};

/// Find compound blocks with only a single statement, and flatten
class CleanupCompoundSingle : public SearchReplace
{
public:
    CleanupCompoundSingle();
};

/// Get rid of Nops
class CleanupNop : public SearchReplace
{
public:
    CleanupNop();
};

/** Simplify the case where two lables appear together and are therefore 
    duplicates. Just have one label. */
class CleanupDuplicateLabels : public SearchReplace
{
public:
    CleanupDuplicateLabels();
};

/// Find a goto to a lable just before the label, and remove the goto
class CleanupIneffectualGoto : public SearchReplace
{
public:
    CleanupIneffectualGoto();
};

/// Find a goto to a lable just before the label, and remove the goto
class CleanupIneffectualLabels : public SearchReplace
{
public:
    CleanupIneffectualLabels();
};

/** Find labels never referenced (ie neither jumped to nor used as a variable) 
    and dispense with them */
class CleanupUnusedLabels : public SearchReplace
{
public:
    CleanupUnusedLabels();
};

/** Remove dead code in switch statements */
class CleanUpDeadCode : public SearchReplace
{
public:
    CleanUpDeadCode();
};

/** turn a compound expression that does not end in an expression into an ordinary compound */
class ReduceVoidCompoundExpression : public SearchReplace
{
public:
    ReduceVoidCompoundExpression();
};

/** Remove instances not used anywhere - except Callables and instances of InheritanceRecord since
    these might do something useful even when not referenced.*/
class CleanupUnusedVariables : public CompareReplace
{
public:
    CleanupUnusedVariables();
};

/// Simplify if(x) if(y) z using &&
class CleanupNestedIf : public SearchReplace
{
public:
    CleanupNestedIf();
};

}; // end namespace

#endif 

