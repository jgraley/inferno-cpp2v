#include "declarators.hpp"

#include "vn/agents/all.hpp"

using namespace Declarators;

// See how this function is used for an example of how it should be used.
Result Declarator::DoReduce( TreePtr<Node> child, TreePtr<Node> type_view, CVQuals cv_quals_view )
{
	if( !child )
		return Result { Result::ABSTRACT, type_view, cv_quals_view, nullptr };
		
	auto declarator = dynamic_cast<const Declarator *>(child.get());
	ASSERTS( declarator )("Tried to reduce ")(type_view)(" ")(child)(" as a declarator");
	return declarator->DeclaratorReduce(type_view, cv_quals_view); // We're not done
}


UniDeclarator::UniDeclarator(TreePtr<Node> child_) :
	child(child_)
{
}


Result Concrete::DeclaratorReduce( TreePtr<Node> type_view, CVQuals cv_quals_view ) const
{
	// child can be NULL, in the case of ☆ and yet still not abstract	
	// No need for Next(), we're done and it's concrete
	if( child )
		return { Result::CONCRETE, type_view, cv_quals_view, child }; 
	else
		return { Result::WILDCARD, type_view, cv_quals_view, nullptr }; 
}


Pointer::Pointer(CVQuals cv_quals_decl_, TreePtr<Node> child_) :
	Indirection(child_),
	cv_quals_decl(cv_quals_decl_)
{
}	


Result Pointer::DeclaratorReduce( TreePtr<Node> type_view, CVQuals cv_quals_view ) const
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Pointer>>();
	node->destination = type_view;
	node->constancy = cv_quals_view.constancy; 
	return DoReduce(child, node, cv_quals_decl);
}


Result Reference::DeclaratorReduce( TreePtr<Node> type_view, CVQuals cv_quals_view ) const
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Reference>>();
	node->destination = type_view;
	node->constancy = cv_quals_view.constancy; 
	return DoReduce(child, node, cv_quals_view); // Copy the constness => alias behaviour, not pointer behaviour.
}


Array::Array(TreePtr<Node> child_, TreePtr<Node> size_) :
	UniDeclarator(child_),
	size(size_)
{
}


Result Array::DeclaratorReduce( TreePtr<Node> type_view, CVQuals cv_quals_view ) const
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Array>>();
	node->element = type_view;
	node->size = size;
	return DoReduce(child, node, cv_quals_view); // Copy the constness for containment
}


Function::Function(TreePtr<Node> child_, list<TreePtr<Node>> params_, CVQuals cv_quals_decl_) :
	UniDeclarator(child_),
	params(params_),
	cv_quals_decl(cv_quals_decl_)
{
}


Result Function::DeclaratorReduce( TreePtr<Node> type_view, CVQuals cv_quals_view ) const
{
	auto node = MakeTreeNode<StandardAgentWrapper<CPPTree::Function>>();
	node->return_type = type_view;
	(void)cv_quals_view; // cv_quals_view would set constness of return value. 
	// Functions can in fact have const return values TODO
	for( auto param : params )
		node->params.push_back(param);
	return DoReduce(child, node, cv_quals_decl);
}
