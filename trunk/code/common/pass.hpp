#ifndef PASS_HPP
#define PASS_HPP

#include "tree/tree.hpp"
#include "refcount.hpp"

class Pass
{
public:
    // Apply this pass to the supplied program
    virtual void operator()( RCPtr<Program> ) = 0; 
    
    // Note: cannot template virtual functions so alternative
    // entry points for eg functions or classes must be added
    // explicitly.
};

#endif
