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

	struct LeafBlock : Block
	{
		string What() const final 
		{ 
			list<string> ls;
			if( node_enum )
				ls.push_back( "node-name" );
			if( identifier_discriminator_enum )
				ls.push_back( "identifier-discriminator" );
			return Join(ls, "/");
		}
		string GetTrace() const 
		{ 
			string s = node_enum ? "node#"+Trace((int)(node_enum.value())) : "no-node"; 
			s += ",";
			s += identifier_discriminator_enum ? "id-disc#"+Trace((int)(identifier_discriminator_enum.value())) : "no-id-disc";
			return s;
		}

		optional<NodeEnum> node_enum;
		optional<IdentifierEnum> identifier_discriminator_enum;
	};

	typedef map<list<string>, NodeEnum> NameToNodeMapType;	
	const NameToNodeMapType &GetNameToEnumMap();
	const AvailableNodeData::NamespaceBlock *GetGlobalNamespaceBlock();
	shared_ptr<Node> MakeNode(NodeEnum ne) const;
	shared_ptr<TreePtrInterface> MakeTreePtr(NodeEnum ne) const;
	bool IsType(const LeafBlock *block) const;
	
private:
	static void InitialiseMap();
	static NameToNodeMapType name_to_node_map;
	static AvailableNodeData::NamespaceBlock global_namespace_block;
};

#endif
