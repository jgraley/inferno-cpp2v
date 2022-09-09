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


string TreePtrInterface::GetTrace() const
{
	return GetName();
}    


// Similar signature to shared_ptr operator==, and we restrict the pointers
// to having the same subbase and base target
bool operator==( const TreePtrInterface &x,
		         const TreePtrInterface &y)
{
	return x.get() == y.get();
}

bool operator!=( const TreePtrInterface &x,
		         const TreePtrInterface &y)
{
	return x.get() != y.get();
}

bool operator<( const TreePtrInterface &x,
		        const TreePtrInterface &y)
{
	return x.get() < y.get();
}
