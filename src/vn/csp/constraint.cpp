#include "constraint.hpp"
#include "query.hpp"
#include "agents/agent.hpp"

using namespace CSP;

string Constraint::GetTrace() const
{
    return GetName() + GetSerialString();
}