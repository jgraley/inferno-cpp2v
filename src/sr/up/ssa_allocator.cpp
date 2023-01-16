#include "ssa_allocator.hpp"

using namespace SR;

SSAAllocator::~SSAAllocator()
{
    // To stop this error, you must Pop() any expected remaining 
    // Ids - these will be the Ids of your outputs.
    ASSERT( id_stack.empty() );
}


SSAAllocator::Reg SSAAllocator::Push()
{
    Reg id = next_id++;
    id_stack.push( id );
    return id;
}


SSAAllocator::Reg SSAAllocator::Pop()
{
    Reg id = id_stack.top();
    id_stack.pop();
    return id;
}


SSAAllocator::Reg SSAAllocator::Peek()
{
    return id_stack.top();
}
