/*
 * clean_up.hpp
 *
 *  Created on: 26 Dec 2009
 *      Author: jgraley
 */

#ifndef CLEAN_UP_HPP
#define CLEAN_UP_HPP

#include "helpers/search_replace.hpp"

class CleanupCompoundMulti : public InPlaceTransformation
{
public:
    using Transformation::operator();
    virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

class CleanupCompoundSingle : public InPlaceTransformation
{
public:
    using Transformation::operator();
    virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

class CleanupNop : public InPlaceTransformation
{
public:
    using Transformation::operator();
    virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

class CleanupDuplicateLabels : public InPlaceTransformation
{
public:
    using Transformation::operator();
    virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

class CleanupIneffectualGoto : public InPlaceTransformation
{
public:
    using Transformation::operator();
    virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

class CleanupUnusedLabels : public InPlaceTransformation
{
public:
    using Transformation::operator();
    virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

#endif 
