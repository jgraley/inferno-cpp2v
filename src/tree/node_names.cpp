#include "node_names.hpp"
#include "misc.hpp"

#include <map>
#include <list>
#include <string>

const NodeNames::NameToNodeMapType &NodeNames::GetNameToNodeMap()
{
	if( name_to_node_map.empty() )
		InitialiseMap();
	
	return name_to_node_map;
}


void NodeNames::InitialiseMap()
{
	name_to_node_map =
	{
#define NODE(NS, NAME) { {#NS, #NAME}, NodeEnum::NS##_##NAME },
#include "node_names.inc"	
	};
	
	ASSERT( !name_to_node_map.empty() );
}


NodeNames::NameToNodeMapType NodeNames::name_to_node_map;
