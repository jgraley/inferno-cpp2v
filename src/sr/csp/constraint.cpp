#include "constraint.hpp"
#include "query.hpp"
#include "agents/agent.hpp"

using namespace CSP;

void CSP::CheckLocalMatch( VariableId variable, Value value )
{
    ASSERT( variable.GetChildAgent()->IsLocalMatch(value.get()) ); // Consistent with variable's type (i.e. in LocalMatch-implied domain)
}
