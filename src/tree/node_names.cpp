#include "node_names.hpp"
#include "misc.hpp"
#include "cpptree.hpp"
#include "sctree.hpp"
#include "localtree.hpp"

#include <map>
#include <list>
#include <string>

const NodeNames::NameToNodeMapType &NodeNames::GetNameToEnumMap()
{
	if( name_to_node_map.empty() )
		InitialiseMap();
	
	return name_to_node_map;
}


shared_ptr<Node> NodeNames::MakeNode(NodeEnum ne) const 
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


shared_ptr<TreePtrInterface> NodeNames::MakeTreePtr(NodeEnum ne) const
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


void NodeNames::InitialiseMap()
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
}


NodeNames::NameToNodeMapType NodeNames::name_to_node_map;
