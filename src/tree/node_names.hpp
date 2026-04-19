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

enum class IdentifierEnum
{
#define NODE(NS, NAME) NS##_##NAME,
#include "identifier_names.inc"	
#undef NODE
};


class AvailableNodeData
{
public:	
	struct Block : Traceable
	{
		virtual string What() const = 0;
	};
	
	struct NamespaceBlock : Block
	{
		string What() const final { return "node name scope"; }
		string GetTrace() const { return Trace(sub_blocks); }

		map<string, unique_ptr<AvailableNodeData::Block>> sub_blocks;
	};

	struct NodeBlock : Block
	{
		string What() const final 
		{ 
			if( !node_enum )
				return "no-node";
			auto m = AvailableNodeData().GetEnumToNameMap();
			return Join(m.at(node_enum.value()), "::");			
		}
		string GetTrace() const 
		{ 
			string s = node_enum ? "#"+Trace((int)(node_enum.value())) : ""; 
			return s + ":" + What();
		}

		optional<NodeEnum> node_enum;
	};

	typedef map<list<string>, NodeEnum> NameToNodeMapType;	
	typedef map<NodeEnum, list<string>> NodeToNameMapType;	
	const NameToNodeMapType &GetNameToEnumMap();
	const NodeToNameMapType &GetEnumToNameMap();
	const AvailableNodeData::NamespaceBlock *GetNodeNamesRoot();
	shared_ptr<Node> MakeNode(NodeEnum ne) const;
	shared_ptr<TreePtrInterface> MakeTreePtr(NodeEnum ne) const;
	bool IsQualifier(const NodeBlock *block) const;
	bool IsDeclaration(const NodeBlock *block) const;
	bool IsType(const NodeBlock *block) const;
	
private:
	static void InitialiseMap();
	static NameToNodeMapType name_to_node_map;
	static NodeToNameMapType node_to_name_map;
	static AvailableNodeData::NamespaceBlock node_names_root;
};

#endif
