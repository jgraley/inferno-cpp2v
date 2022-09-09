#include "common/common.hpp"
#include "common/magic.hpp"
#include "itemise.hpp"
#include "node.hpp"

#include "tree_ptr.hpp"

TreePtrInterface &TreePtrInterface::operator=( const TreePtrInterface &o )
{
	(void)Itemiser::Element::operator=( o ); 
	(void)Traceable::operator=( o );
	return *this;
}


bool TreePtrInterface::operator<(const TreePtrInterface &other) const
{
	return GetSS() < other.GetSS();
}


bool TreePtrInterface::operator==(const TreePtrInterface &other) const
{
	return get() == other.get();
}


bool TreePtrInterface::operator!=(const TreePtrInterface &other) const
{
	return get() != other.get();
}


string TreePtrInterface::GetTrace() const
{
	return GetName();
}    


