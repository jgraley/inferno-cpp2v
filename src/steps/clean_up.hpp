/*
 * clean_up.hpp
 *
 *  Created on: 26 Dec 2009
 *      Author: jgraley
 */

#ifndef CLEAN_UP_HPP
#define CLEAN_UP_HPP

#include "helpers/search_replace.hpp"

class Cleanup : public InPlaceTransformation
{
public:
    using Transformation::operator();
    virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

#endif 
