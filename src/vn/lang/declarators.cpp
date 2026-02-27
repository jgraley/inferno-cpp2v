#include "declarators.hpp"

#include "vn/agents/all.hpp"

using namespace Declarators;

// See how this function is used for an example of how it should be used.
Result Declarator::DoReduce( TreePtr<Node> child, TreePtr<Node> type )
{
	if( !child )
		return Result { Result::ABSTRACT, type, nullptr }; // We're done and it's abstract
		
	auto declarator = dynamic_cast<const Declarator *>(child.get());
	ASSERTS( declarator );
	return declarator->DeclaratorReduce(type); // We're not done
}


UniDeclarator::UniDeclarator(TreePtr<Node> child_) :
	child(child_)
{
}


Result Concrete::DeclaratorReduce( TreePtr<Node> type ) const
{
	// child can be NULL, in the case of ☆ and yet still not abstract	
	// No need for Next(), we're done and it's concrete
	if( child )
		return { Result::CONCRETE, type, child };
	else
		return { Result::WILDCARD, type, nullptr };
}


Result Pointer::DeclaratorReduce( TreePtr<Node> type ) const
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Pointer>>();
	node->destination = type;
	node->constancy = MakeTreeNode<CPPTree::NonConst>(); // TODO do better
	type = node;
	return DoReduce(child, type);
}


Result Reference::DeclaratorReduce( TreePtr<Node> type ) const
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Reference>>();
	node->destination = type;
	node->constancy = MakeTreeNode<StandardAgentWrapper<CPPTree::NonConst>>(); // TODO do better
	type = node;
	return DoReduce(child, type);
}


Array::Array(TreePtr<Node> child_, TreePtr<Node> size_) :
	UniDeclarator(child_),
	size(size_)
{
}


Result Array::DeclaratorReduce( TreePtr<Node> type ) const
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Array>>();
	node->element = type;
	node->size = size;
	type = node;
	return DoReduce(child, type);
}


Function::Function(TreePtr<Node> child_, list<TreePtr<Node>> params_) :
	UniDeclarator(child_),
	params(params_)
{
}


Result Function::DeclaratorReduce( TreePtr<Node> type ) const
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Function>>();
	node->return_type = type;
	for( auto param : params )
		node->params.push_back(param);
	type = node;
	return DoReduce(child, type);
}
