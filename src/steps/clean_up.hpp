/*
 * clean_up.hpp
 *
 *  Created on: 26 Dec 2009
 *      Author: jgraley
 */

#ifndef CLEAN_UP_HPP
#define CLEAN_UP_HPP

#include "sr/search_replace.hpp"
#include "sr/vn_step.hpp"

namespace Steps {

using namespace SR;

/// Find compound statements inside compund statements and flatten
class CleanupStatementExpression : public VNStep
{    
public:
    CleanupStatementExpression();
};
   
/// Find compound statements inside compund statements and flatten
class CleanupCompoundMulti : public VNStep
{    
public:
    CleanupCompoundMulti();
};

/// Find compound blocks with only a single statement, and flatten
class CleanupCompoundSingle : public VNStep
{
public:
    CleanupCompoundSingle();
};

/// Get rid of Nops
class CleanupNop : public VNStep
{
public:
    CleanupNop();
};

/** Simplify the case where two lables appear together and are therefore 
    duplicates. Just have one label. */
class CleanupDuplicateLabels : public VNStep
{
public:
    CleanupDuplicateLabels();
};

/// Find a goto to a lable just before the label, and remove the goto
class CleanupIneffectualGoto : public VNStep
{
public:
    CleanupIneffectualGoto();
};

/// Find a goto to a lable just before the label, and remove the goto
class CleanupIneffectualLabels : public VNStep
{
public:
    CleanupIneffectualLabels();
};

/** Find labels never referenced (ie neither jumped to nor used as a variable) 
    and dispense with them */
class CleanupUnusedLabels : public VNStep
{
public:
    CleanupUnusedLabels();
};

/** Remove dead code in switch statements */
class CleanUpDeadCode : public VNStep
{
public:
    CleanUpDeadCode();
};

/** turn a compound expression that does not end in an expression into an ordinary compound */
class CleanupVoidStatementExpression : public VNStep
{
public:
    CleanupVoidStatementExpression();
};

/** Remove instances not used anywhere - except Callables and instances of InheritanceRecord since
    these might do something useful even when not referenced.*/
class CleanupUnusedVariables : public VNStep
{
public:
    CleanupUnusedVariables();
};

/// Simplify if(x) if(y) z using &&
class CleanupNestedIf : public VNStep
{
public:
    CleanupNestedIf();
};

}; // end namespace

#endif 

