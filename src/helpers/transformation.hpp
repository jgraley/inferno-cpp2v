#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "node/specialise_oostd.hpp"
#include "node/graphable.hpp"
#include <functional>

class TreeKit
{
public:	
    class UnknownNode : public Exception {};
	
    // Convention is that second points to one of first's TreePtrs
    typedef pair<TreePtr<Node>, const TreePtrInterface *> LinkInfo;
	
	virtual set<LinkInfo> GetDeclarers( TreePtr<Node> node ) const = 0;
    virtual ~TreeKit();
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
#if 0
    // Convention is that *first points to *second or first is NULL
    template< class VALUE_TYPE >
    using NodeInfo = pair<const TreePtrInterface *, TreePtr<VALUE_TYPE>>;
// Handy macros for a node and a child pointer or just a node
#define NODE_AND_CHILD( N, C ) make_pair(&(N->C), N->C)
#define NODE_ONLY( N ) make_pair(nullptr, N)
#else
    template< class VALUE_TYPE >
    using NodeInfo = TreePtr<VALUE_TYPE>;
#define NODE_AND_CHILD( N, C ) (N->C)
#define NODE_ONLY( N ) (N)
#define CHANGE_NODE( N, NI ) (N)
#define GET_NODE( NI ) (NI)
#endif

    // Apply this transformation to tree at root, using context for decls etc.
    virtual NodeInfo<Node> operator()( const TreeKit &kit, // Handy functions
    		                           TreePtr<Node> node ) = 0;    // Root of the subtree we want to modify    		                          
};

#endif
