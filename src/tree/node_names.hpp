#ifndef ENUMERATE_NODE_TYPES_HPP
#define ENUMERATE_NODE_TYPES_HPP

#include "common/common.hpp"
#include "node/node.hpp"
#include <optional>

enum class NodeEnum
{
#define NODE(NS, NAME) NS##_##NAME,
#include "node_names.inc"	
#define PREFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define POSTFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define INFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#include "operator_data.inc"
#undef NODE
};

struct NodeNameBlock : Traceable
{
	map<string, unique_ptr<NodeNameBlock>> sub_blocks;
	optional<NodeEnum> leaf_enum;
	string GetTrace() const { return Trace(sub_blocks)+":"+(leaf_enum?Trace((int)(leaf_enum.value())):"NULL"); }
};


class NodeNames
{
public:	
	typedef map<list<string>, NodeEnum> NameToNodeMapType;	
	const NameToNodeMapType &GetNameToEnumMap();
	const NodeNameBlock *GetRootBlock();
	shared_ptr<Node> MakeNode(NodeEnum ne) const;
	shared_ptr<TreePtrInterface> MakeTreePtr(NodeEnum ne) const;
	
private:
	static void InitialiseMap();
	static NameToNodeMapType name_to_node_map;
	static NodeNameBlock root_block;
};

#endif
