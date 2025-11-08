#ifndef ENUMERATE_NODE_TYPES_HPP
#define ENUMERATE_NODE_TYPES_HPP

#include "common/common.hpp"
#include "node/node.hpp"

enum class NodeEnum
{
#define NODE(NS, NAME) NS##_##NAME,
#include "node_names.inc"	
#define PREFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define POSTFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define INFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define OTHER(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#include "operator_data.inc"
#undef NODE
};

// This should secure us a lexicographical log-time lookup for a scoped name.
// 2-level for now, but could be expanded. We can then switch on the enum
// which the compiler can probably make pretty quick (if all the cases are 
// equivalent code, it should be a calculated jump).

class NodeNames
{
public:	
	typedef map<list<string>, NodeEnum> NameToNodeMapType;	
	const NameToNodeMapType &GetNameToEnumMap();
	shared_ptr<Node> MakeNode(NodeEnum ne) const;
	shared_ptr<TreePtrInterface> MakeTreePtr(NodeEnum ne) const;
	
private:
	static void InitialiseMap();
	static NameToNodeMapType name_to_node_map;
};

#endif
