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


class NodeNames
{
public:	
	struct Block : Traceable
	{
		virtual string What() const = 0;
	};
	
	struct ScopeBlock : Block
	{
		map<string, unique_ptr<NodeNames::Block>> sub_blocks;
		string What() const final { return "node name scope"; }
		string GetTrace() const { return Trace(sub_blocks); }
	};

	struct NodeBlock : Block
	{
		optional<NodeEnum> node_enum;
		bool is_identifier_type;
		string What() const final 
		{ 
			list<string> ls;
			if( node_enum )
				ls.push_back( "node" );
			if( is_identifier_type )
				ls.push_back( "identifier" );
			return Join(ls, "/") + " name";
		}
		string GetTrace() const 
		{ 
			string s = node_enum ? "node#"+Trace((int)(node_enum.value())) : "no-node"; 
			s += ",";
			s += is_identifier_type ? "id-type" : "no-id-type";
			return s;
		}
	};

	typedef map<list<string>, NodeEnum> NameToNodeMapType;	
	const NameToNodeMapType &GetNameToEnumMap();
	const NodeNames::Block *GetRootBlock();
	shared_ptr<Node> MakeNode(NodeEnum ne) const;
	shared_ptr<TreePtrInterface> MakeTreePtr(NodeEnum ne) const;
	
private:
	static void InitialiseMap();
	static NameToNodeMapType name_to_node_map;
	static NodeNames::ScopeBlock root_block;
};

#endif
