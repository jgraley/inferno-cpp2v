#ifndef ENUMERATE_NODE_TYPES_HPP
#define ENUMERATE_NODE_TYPES_HPP

#include "common/common.hpp"
#include "node/node.hpp"
#include <optional>

enum class NodeTag : unsigned int
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

struct ANDBlock : Traceable // Ugly: done so we can forward-declare
{
	virtual string What() const = 0;
};


class AvailableNodeData
{
public:		
	typedef ANDBlock Block;
	struct NamespaceBlock : Block
	{
		string What() const final { return "node name scope"; }
		string GetTrace() const { return Trace(sub_blocks); }

		map<string, unique_ptr<ANDBlock>> sub_blocks;
	};

	struct NodeBlock : Block
	{
		string What() const final 
		{ 
			if( !tag )
				return "no-node";
			auto m = AvailableNodeData().GetTagToNameMap();
			return Join(m.at(tag.value()), "::");			
		}
		string GetTrace() const 
		{ 
			string s = tag ? "#"+Trace((int)(tag.value())) : ""; 
			return s + ":" + What();
		}

		optional<NodeTag> tag;
	};

	typedef map<list<string>, NodeTag> NameToTagMapType;	
	typedef map<NodeTag, list<string>> TagToNameMapType;	
	const NameToTagMapType &GetNameToTagMap();
	const TagToNameMapType &GetTagToNameMap();
	const AvailableNodeData::NamespaceBlock *GetNodeNamesRoot();
	shared_ptr<Node> MakeNode(NodeTag t) const;
	shared_ptr<TreePtrInterface> MakeTreePtr(NodeTag t) const;
	NodeTag GetTagOfNode(TreePtr<Node> node) const; // Discards StandardAgentWrapper if present
	
	typedef map<NodeTag, TreePtr<Node>> TagToNodeMapType;
	typedef multimap<string, NodeTag> KeywordToTagMapType;
	optional<NodeTag> TryGetByKeywordIfToken( string keyword ) const;		
	
private:
	static void InitialiseMap();
	
	static NameToTagMapType name_to_tag_map;
	static TagToNameMapType tag_to_name_map;
	static AvailableNodeData::NamespaceBlock node_names_root;
	
	static TagToNodeMapType tag_to_node_map;
	static KeywordToTagMapType keyword_to_tag_map;
};

#endif
