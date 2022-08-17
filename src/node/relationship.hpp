#ifndef RELATIONSHIP_HPP
#define RELATIONSHIP_HPP

#include <set>

using namespace std;

class TreePtrInterface;

class ChildRelationship
{
public:	
	// Return double pointers (via TreePtrInterface) to disambiguate in 
	// case we have a decl and usage that could (in principle) be the 
	// same identifier node.
	virtual set<const TreePtrInterface *> GetDeclared();
};

#endif
