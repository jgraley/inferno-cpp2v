#include "common/common.hpp"
#include "common/trace.hpp"

#include <set>

#include "properties.hpp"

set<const TreePtrInterface *> Properties::GetDeclared() 
{
	// By default none of the children are declared
	return set<const TreePtrInterface *> {};
}
