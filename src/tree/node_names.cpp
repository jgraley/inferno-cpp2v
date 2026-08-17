#include "node_names.hpp"
#include "misc.hpp"
#include "cpptree.hpp"
#include "sctree.hpp"
#include "localtree.hpp"

#include <map>
#include <list>

const AvailableNodeData::NameToTagMapType &AvailableNodeData::GetNameToTagMap()
{
	if( name_to_tag_map.empty() )
		InitialiseMap();
	
	return name_to_tag_map;
}


const AvailableNodeData::TagToNameMapType &AvailableNodeData::GetTagToNameMap()
{
	if( tag_to_name_map.empty() )
		InitialiseMap();
	
	return tag_to_name_map;
}


const AvailableNodeData::NamespaceBlock *AvailableNodeData::GetNodeNamesRoot()
{
	if( node_names_root.sub_blocks.empty() )
		InitialiseMap();	
		
	return &node_names_root;
}


shared_ptr<Node> AvailableNodeData::MakeNode(NodeTag t) const 
{
	switch(t)
	{
#define NODE(NS, NAME) \
	case NodeTag::NS##_##NAME: \
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


shared_ptr<TreePtrInterface> AvailableNodeData::MakeTreePtr(NodeTag t) const
{
	switch(t)
	{
#define NODE(NS, NAME) \
	case NodeTag::NS##_##NAME: \
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


bool AvailableNodeData::IsQualifier(const NodeBlock *block) const
{
	ASSERT( block->tag );
	shared_ptr<Node> spn = MakeNode(block->tag.value());
	return !!dynamic_cast<const CPPTree::Qualifier *>(spn.get());
}


bool AvailableNodeData::IsMemberInit(const NodeBlock *block) const
{
	ASSERT( block->tag );
	shared_ptr<Node> spn = MakeNode(block->tag.value());
	return !!dynamic_cast<const CPPTree::MemberInitialiser *>(spn.get());
}


#define EXCLUDE_LABEL_FROM_DECLARATION
bool AvailableNodeData::IsDeclaration(const NodeBlock *block) const
{
	ASSERT( block->tag );
	shared_ptr<Node> spn = MakeNode(block->tag.value());
#ifdef EXCLUDE_LABEL_FROM_DECLARATION	
	return dynamic_cast<const CPPTree::Declaration *>(spn.get()) && !dynamic_cast<const CPPTree::LabelDeclaration *>(spn.get());
#else	
	return !!dynamic_cast<const CPPTree::Declaration *>(spn.get());
#endif	
}


bool AvailableNodeData::IsType(const NodeBlock *block) const
{
	ASSERT( block->tag );
	shared_ptr<Node> spn = MakeNode(block->tag.value());
	return !!dynamic_cast<const CPPTree::Type *>(spn.get());		
}


TreePtr<Node> AvailableNodeData::TryGetByKeywordIfToken( string keyword ) const
{
	if( keyword_to_node_map.empty() )
		InitialiseMap();

    auto [it_begin, it_end] = keyword_to_node_map.equal_range(keyword);
    set<TreePtr<Node>> found;
    for( auto it = it_begin; it != it_end; ++it ) try 
    {
		TreePtr<Node> node = it->second;
		(void)node->GetToken();
		found.insert( node );
	} catch( Syntax::UnimplementedToken & ) {}
				
	if( found.empty() )
		return nullptr;
	else	
		return SoloElementOf( found );
}


TreePtr<Node> AvailableNodeData::Clone( TreePtr<Node> archetype ) const
{
    shared_ptr<Cloner> dup_dest = archetype->Clone();
    TreePtr<Node> dest( dynamic_pointer_cast<Node>( dup_dest ) );
            
    return dest;
}


void AvailableNodeData::InitialiseMap()
{
	name_to_tag_map =
	{
#define NODE(NS, NAME) { {#NS, #NAME}, NodeTag::NS##_##NAME },
#include "node_names.inc"	
#define PREFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define POSTFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define INFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#include "operator_data.inc"
#undef NODE
	};
	ASSERT( !name_to_tag_map.empty() );
	
	for( auto p : name_to_tag_map )
	{
		tag_to_name_map[p.second] = p.first;
		
		list<string> flat_list = p.first;
		NodeTag tag = p.second;
		
		if( node_names_root.sub_blocks.count(flat_list.front())==0 )
		{
			auto sb = make_unique<NamespaceBlock>();
			node_names_root.sub_blocks[flat_list.front()] = move(sb);
		}
	
		Block *block = node_names_root.sub_blocks.at( flat_list.front() ).get();
		auto namespace_block = dynamic_cast<NamespaceBlock *>(block);
		ASSERT(namespace_block);

		if( !namespace_block->sub_blocks[flat_list.back()] ) // can create -> NULL
			namespace_block->sub_blocks.at(flat_list.back()) = make_unique<NodeBlock>();

		NodeBlock *node_block = dynamic_cast<NodeBlock *>(namespace_block->sub_blocks.at(flat_list.back()).get());
		ASSERT( node_block );
		node_block->tag = tag;		
	}

#define NODE(NS, NAME) tag_to_node_map[NodeTag::NS##_##NAME] = MakeTreeNode<NS::NAME>();
#include "node_names.inc"	
#define PREFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define POSTFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#define INFIX(TOK, TEXT, NAME, BASE, CAT, PROD, ASSOC) NODE(CPPTree, NAME)
#include "operator_data.inc"
#undef NODE

	for( pair p : tag_to_node_map )
	{		
		try {		
			keyword_to_node_map[p.second->GetKeyword(Syntax::Policy())] = p.second;
		} catch( Syntax::UnimplementedKeyword & ) {}			
	}
}


AvailableNodeData::NameToTagMapType AvailableNodeData::name_to_tag_map;
AvailableNodeData::TagToNameMapType AvailableNodeData::tag_to_name_map;
AvailableNodeData::NamespaceBlock AvailableNodeData::node_names_root;

AvailableNodeData::TagToNodeMapType AvailableNodeData::tag_to_node_map;
AvailableNodeData::KeywordToNodeMapType AvailableNodeData::keyword_to_node_map;
