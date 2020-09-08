#include "constraint.hpp"
#include "query.hpp"
#include "agent.hpp"

using namespace CSP;

const Value CSP::NullValue = nullptr;
 
 

void CSP::CheckConsistent( VariableId variable, Value value )
{
    ASSERT( value != NullValue );    // not NULL
    ASSERT( variable->IsLocalMatch(value.get()) ); // Consistent with variable's type (i.e. in LocalMatch-implied domain)
}
