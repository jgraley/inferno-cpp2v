#ifndef LOWER_CONTROL_FLOW_HPP
#define LOWER_CONTROL_FLOW_HPP

#include "helpers/search_replace.hpp"

class IfToIfGoto : public InPlaceTransformation
{
public:
    using Transformation::operator();
    virtual void operator()( TreePtr<Node> context, TreePtr<Node> *proot );
};

#endif 
