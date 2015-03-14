#ifndef RENDERABLE_HPP
#define RENDERABLE_HPP

#include "common/common.hpp"

/// Interface for being able to reutnr a string for rendering (as opposed to debug)
class Renderable : virtual Traceable
{
public:
    virtual string GetRender() const
    {
        // If no render supplied just return the name - right for graphs
        // and a debugging aid in C++ renders
        return GetName(); 
    }
};

#endif