#include "declarators.hpp"
using namespace Declarators;

// See how this function is used for an example of how it should be used.
TreePtr<Node> Declarator::Next( TreePtr<Node> inner, TreePtr<Node> &type )
{
	if( !inner )
		return nullptr; // We're done and it's anonymous
		
	if( auto declarator = dynamic_cast<const Declarator *>(inner.get()) )
		return declarator->DeclaratorReduce(type); // We're not done
	else
		return inner; // We're done and it's not anonymous (we can allow VN agent nodes here and indeed these make us drop out of declarator reduction)
}


UniDeclarator::UniDeclarator(TreePtr<Node> inner_) :
	inner(inner_)
{
}


TreePtr<Node> Pointer::DeclaratorReduce( TreePtr<Node> &type ) const
{
	auto node = MakeTreeNode<CPPTree::Pointer>();
	node->destination = type;
	node->constancy = MakeTreeNode<CPPTree::NonConst>(); // TODO do better
	type = node;
	return Next(inner, type);
}


TreePtr<Node> Reference::DeclaratorReduce( TreePtr<Node> &type ) const
{
	auto node = MakeTreeNode<CPPTree::Reference>();
	node->destination = type;
	node->constancy = MakeTreeNode<CPPTree::NonConst>(); // TODO do better
	type = node;
	return Next(inner, type);
}


Array::Array(TreePtr<Node> inner_, TreePtr<Node> size_) :
	UniDeclarator(inner_),
	size(size_)
{
}


TreePtr<Node> Array::DeclaratorReduce( TreePtr<Node> &type ) const
{
	auto node = MakeTreeNode<CPPTree::Array>();
	node->element = type;
	node->size = size;
	type = node;
	return Next(inner, type);
}


Function::Function(TreePtr<Node> inner_, list<TreePtr<Node>> params_) :
	UniDeclarator(inner_),
	params(params_)
{
}


TreePtr<Node> Function::DeclaratorReduce( TreePtr<Node> &type ) const
{
	auto node = MakeTreeNode<CPPTree::Function>();
	node->return_type = type;
	for( auto param : params )
		node->params.push_back(param);
	type = node;
	return Next(inner, type);
}
