#ifndef COLOURED_HPP
#define COLOURED_HPP

#include "common/common.hpp"
#include "common/trace.hpp"

#include <string>

using namespace std;

class Coloured
{
public:
    /** 
     * Colours are GraphVis colours as listed at 
     * http://www.graphviz.org/doc/info/colors.html
     */
    virtual string GetColour() const
    {
        return "";
    };
};    

#endif
