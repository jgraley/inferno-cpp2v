#ifndef ENUMERATE_NODE_TYPES_HPP
#define ENUMERATE_NODE_TYPES_HPP

#include "common/common.hpp"

enum class NodeEnum
{
#define NODE(NS, NAME) NS##_##NAME,
#include "node_names.inc"	
};

// This should secure us a lexicographical log-time lookup for a scoped name.
// 2-level for now, but could be expanded. You can then switch on the enum
// which the compiler can probably make pretty quick (if all the cases are 
// equivalent code, it should be a calculated jump).

class NodeNames
{
public:	
	typedef map<list<string>, NodeEnum> NameToNodeMapType;	
	const NameToNodeMapType &GetNameToNodeMap();
	
private:
	static void InitialiseMap();
	static NameToNodeMapType name_to_node_map;
};

#endif
