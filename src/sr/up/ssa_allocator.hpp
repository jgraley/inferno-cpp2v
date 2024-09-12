#ifndef SSA_ALLOCATOR_HPP
#define SSA_ALLOCATOR_HPP

#include "common/common.hpp"

namespace SR
{ 

class SSAAllocator
{
public:
    typedef int Reg;
    
    ~SSAAllocator();
    
    Reg Push();
    Reg Pop();
    Reg Peek();
    
    stack<Reg> id_stack;
    Reg next_id = 0;
};    
    
}
    
#endif

