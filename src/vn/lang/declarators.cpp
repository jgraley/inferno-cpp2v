#include "declarators.hpp"

#include "vn/agents/all.hpp"

using namespace Declarators;

// See how this function is used for an example of how it should be used.
Result Declarator::DoReduce( TreePtr<Node> inner, TreePtr<Node> &type )
{
	if( !inner )
		return Result { true, nullptr }; // We're done and it's abstract
		
	auto declarator = dynamic_cast<const Declarator *>(inner.get());
	ASSERTS( declarator );
	return declarator->DeclaratorReduce(type); // We're not done
}


UniDeclarator::UniDeclarator(TreePtr<Node> inner_) :
	inner(inner_)
{
}


Result Concrete::DeclaratorReduce( TreePtr<Node> & ) const
{
	Result info;
	info.abstract = false;
	info.innermost = inner; // Can be NULL, in the case of ☆ and yet still not abstract
	
	// No need for Next(), we're done and it's concrete
	return info;
}


Result Pointer::DeclaratorReduce( TreePtr<Node> &type ) const
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Pointer>>();
	node->destination = type;
	node->constancy = MakeTreeNode<CPPTree::NonConst>(); // TODO do better
	type = node;
	return DoReduce(inner, type);
}


Result Reference::DeclaratorReduce( TreePtr<Node> &type ) const
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Reference>>();
	node->destination = type;
	node->constancy = MakeTreeNode<StandardAgentWrapper<CPPTree::NonConst>>(); // TODO do better
	type = node;
	return DoReduce(inner, type);
}


Array::Array(TreePtr<Node> inner_, TreePtr<Node> size_) :
	UniDeclarator(inner_),
	size(size_)
{
}


Result Array::DeclaratorReduce( TreePtr<Node> &type ) const
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Array>>();
	node->element = type;
	node->size = size;
	type = node;
	return DoReduce(inner, type);
}


Function::Function(TreePtr<Node> inner_, list<TreePtr<Node>> params_) :
	UniDeclarator(inner_),
	params(params_)
{
}


Result Function::DeclaratorReduce( TreePtr<Node> &type ) const
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Function>>();
	node->return_type = type;
	for( auto param : params )
		node->params.push_back(param);
	type = node;
	return DoReduce(inner, type);
}
