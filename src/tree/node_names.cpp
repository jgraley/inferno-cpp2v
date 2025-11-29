#include "node_names.hpp"
#include "misc.hpp"
#include "cpptree.hpp"
#include "sctree.hpp"
#include "localtree.hpp"

#include <map>
#include <list>
#include <string>

const AvailableNodeData::NameToNodeMapType &AvailableNodeData::GetNameToEnumMap()
{
	if( name_to_node_map.empty() )
		InitialiseMap();
	
	return name_to_node_map;
}


const AvailableNodeData::Block *AvailableNodeData::GetRootBlock()
{
	if( root_block.sub_blocks.empty() )
		InitialiseMap();	
		
	return &root_block;
}


shared_ptr<Node> AvailableNodeData::MakeNode(NodeEnum ne) const 
{
	switch(ne)
	{
#define NODE(NS, NAME) \
	case NodeEnum::NS##_##NAME: \
		return shared_ptr<Node>( new NS::NAME ); 
#include "node_names.inc"			
#define PREFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define POSTFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define INFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#include "operator_data.inc"
#undef NODE
	}
	
	// By design we should have a case for every value of the node enum
	ASSERTFAIL("Invalid value for node enum"); 
}


shared_ptr<TreePtrInterface> AvailableNodeData::MakeTreePtr(NodeEnum ne) const
{
	switch(ne)
	{
#define NODE(NS, NAME) \
	case NodeEnum::NS##_##NAME: \
		return make_shared<TreePtr<NS::NAME>>(); 
#include "node_names.inc"			
#define PREFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define POSTFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define INFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#include "operator_data.inc"
#undef NODE
	}

	// By design we should have a case for every value of the node enum
	ASSERTFAIL("Invalid value for node enum");	
}


void AvailableNodeData::InitialiseMap()
{
	name_to_node_map =
	{
#define NODE(NS, NAME) { {#NS, #NAME}, NodeEnum::NS##_##NAME },
#include "node_names.inc"	
#define PREFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define POSTFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define INFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#include "operator_data.inc"
#undef NODE
	};
	ASSERT( !name_to_node_map.empty() );
		
	for( auto p : name_to_node_map )
	{
		list<string> flat_list = p.first;
		NodeEnum node_enum = p.second;
		
		if( root_block.sub_blocks.count(flat_list.front())==0 )
		{
			auto sb = make_unique<ScopeBlock>();
			root_block.sub_blocks[flat_list.front()] = move(sb);
		}
	
		Block *block = root_block.sub_blocks.at( flat_list.front() ).get();
		auto scope_block = dynamic_cast<ScopeBlock *>(block);
		ASSERT(scope_block);

		if( !scope_block->sub_blocks[flat_list.back()] ) // can create -> NULL
			scope_block->sub_blocks.at(flat_list.back()) = make_unique<LeafBlock>();

		LeafBlock *node_block = dynamic_cast<LeafBlock *>(scope_block->sub_blocks.at(flat_list.back()).get());
		ASSERT( node_block );
		node_block->node_enum = node_enum;		
	}
	
	map<list<string>, IdentifierEnum> ident_name_map = 
	{
#define NODE(NS, NAME) { {#NS, #NAME}, IdentifierEnum::NS##_##NAME },
#include "identifier_names.inc"	
#undef NODE	
	};
	
	for( auto p : ident_name_map )
	{
		list<string> flat_list = p.first;
		IdentifierEnum identifier_discriminator_enum = p.second;		
		
		if( root_block.sub_blocks.count(flat_list.front())==0 )
		{
			auto sb = make_unique<ScopeBlock>();
			root_block.sub_blocks[flat_list.front()] = move(sb);
		}
		
		Block *block = root_block.sub_blocks.at( flat_list.front() ).get();
		auto scope_block = dynamic_cast<ScopeBlock *>(block);
		ASSERT(scope_block);

		if( !scope_block->sub_blocks[flat_list.back()] ) // can create -> NULL
			scope_block->sub_blocks.at(flat_list.back()) = make_unique<LeafBlock>();

		LeafBlock *node_block = dynamic_cast<LeafBlock *>(scope_block->sub_blocks.at(flat_list.back()).get());
		ASSERT( node_block );
		node_block->identifier_discriminator_enum = identifier_discriminator_enum;
		
		ASSERT( !name_to_node_map.empty() );
	}
	
	
}


AvailableNodeData::NameToNodeMapType AvailableNodeData::name_to_node_map;
AvailableNodeData::ScopeBlock AvailableNodeData::root_block;
