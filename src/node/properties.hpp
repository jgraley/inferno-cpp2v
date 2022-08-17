#ifndef PROPERTIES_HPP
#define PROPERTIES_HPP


#include <set>

using namespace std;

class TreePtrInterface;

class Properties
{
public:	
	// Return double pointers (via TreePtrInterface) to disambiguate in 
	// case we have a decl and usage that could (in principle) be the 
	// same identifier node.
	virtual set<const TreePtrInterface *> GetDeclared();
};

#endif
