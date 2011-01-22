/*
 * clean_up.hpp
 *
 *  Created on: 26 Dec 2009
 *      Author: jgraley
 */

#ifndef CLEAN_UP_HPP
#define CLEAN_UP_HPP

#include "helpers/search_replace.hpp"

class CleanupCompoundMulti : public SearchReplace
{    
public:
    CleanupCompoundMulti();
};

class CleanupCompoundSingle : public SearchReplace
{
public:
    CleanupCompoundSingle();
};

class CleanupNop : public SearchReplace
{
public:
    CleanupNop();
};

class CleanupDuplicateLabels : public SearchReplace
{
public:
    CleanupDuplicateLabels();
};

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
