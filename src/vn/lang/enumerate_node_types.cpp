
#include "sort_decls.hpp"
#include "tree/misc.hpp"
#include "helpers/simple_compare.hpp"

using namespace CPPTree;

#include "enumerate_node_types.hpp"
#include "tree/misc.hpp"

#include <map>
#include <list>
#include <string>

const NameToNodeMapType &NodeData::GetNameToNodeMap()
{
	if( name_to_node_map.empty() )
		InitialiseMap();
	
	return name_to_node_map;
}


void NodeData::InitialiseMap()
{
	name_to_node_map =
	{
#define NODE(NS, NAME) { {#NS, #NAME}, NodeEnum::NS##_##NAME },
#include "node_types_data.inc"	
	};
	
	ASSERT( !name_to_node_map.empty() );
}


NameToNodeMapType NodeData::name_to_node_map;
