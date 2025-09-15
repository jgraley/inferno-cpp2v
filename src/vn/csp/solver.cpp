#include "solver.hpp"

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
