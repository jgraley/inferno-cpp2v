#include "declarators.hpp"

using namespace Declarators;

UniDeclarator::UniDeclarator(TreePtr<Base> inner_) :
	inner(inner_)
{
}


Array::Array(TreePtr<Base> inner_, TreePtr<CPPTree::Initialiser> size_) :
	UniDeclarator(inner_),
	size(size_)
{
}


Function::Function(TreePtr<Base> inner_, Collection<CPPTree::Declaration> params_) :
	UniDeclarator(inner_),
	params(params_)
{
}
