
#include "sort_decls.hpp"
#include "tree/misc.hpp"
#include "helpers/simple_compare.hpp"

using namespace CPPTree;

#include "enumerate_node_types.hpp"
#include "tree/misc.hpp"

#include <map>
#include <list>
#include <string>

static const NameToNodeMapType name_to_node_map =
{
#define NODE(NS, NAME) { {#NS, #NAME}, NodeEnum::NS##_##NAME },
#include "node_types_data.inc"	
};
 
const NameToNodeMapType &NodeData::GetNameToNodeMap()
{
	return name_to_node_map;
}


