/*
 * clean_up.hpp
 *
 *  Created on: 26 Dec 2009
 *      Author: jgraley
 */

#ifndef CLEAN_UP_HPP
#define CLEAN_UP_HPP

#include "sr/search_replace.hpp"

// Find compound statements inside compund statements and flatten
class CleanupCompoundMulti : public SearchReplace
{    
public:
    CleanupCompoundMulti();
};

// Find compound blocks with only a single statement, and flatten
class CleanupCompoundSingle : public SearchReplace
{
public:
    CleanupCompoundSingle();
};

// Get rid of Nops
class CleanupNop : public SearchReplace
{
public:
    CleanupNop();
};

// Simplify the case where two lables appear together and are therefore 
// duplicates. Just have one label.
class CleanupDuplicateLabels : public SearchReplace
{
public:
    CleanupDuplicateLabels();
};

// Find a goto to a lable just before the label, and remove the goto
class CleanupIneffectualGoto : public SearchReplace
{
public:
    CleanupIneffectualGoto();
};

class CleanupUnusedLabels : public SearchReplace
{
public:
    CleanupUnusedLabels();
};

#endif 
