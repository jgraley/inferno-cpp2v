#include "common/common.hpp"
#include "common/trace.hpp"

#include <set>

#include "relationship.hpp"

set<const TreePtrInterface *> ChildRelationship::GetDeclared() 
{
    // By default none of the children are declared
    return set<const TreePtrInterface *> {};
}
