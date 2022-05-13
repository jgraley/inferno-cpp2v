#include "solver.hpp"
#include "solver_holder.hpp"

using namespace CSP;


Solver::Solver() 
{
}
 

Solver::~Solver() 
{
}
 
 
string Solver::GetTrace() const
{
    return GetName() + GetSerialString();
}