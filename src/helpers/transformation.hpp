#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "node/specialise_oostd.hpp"
#include "node/graphable.hpp"
#include <functional>

class TreeKit
{
public:	
	typedef pair<TreePtr<Node>, const TreePtrInterface *> LinkInfo;
	
	virtual set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const = 0;
};


class ReferenceTreeKit : public TreeKit
{
public:	
	ReferenceTreeKit( TreePtr<Node> context_ );
	set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const override;
	
private:
	TreePtr<Node> context;
};


class Transformation : public virtual Graphable
{
public:
    // Apply this transformation to tree at root, using context for decls etc.
    virtual TreePtr<Node> operator()( TreePtr<Node> context,      // The whole program, for searches
    		                          TreePtr<Node> node ) = 0;   // Root of the subtree we want to modify
    		                          
    virtual TreePtr<Node> operator()( const TreeKit &kit, // Handy functions
    		                          TreePtr<Node> node ) = 0;    // Root of the subtree we want to modify    		                          
};

#endif
