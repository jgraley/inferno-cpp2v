#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "tree/generics.hpp"

class Transformation
{
public:
    // Apply this transformation to tree at root, using context for decls etc.
	// Out-of-place version, neither context nor tree may be modified.
    virtual TreePtr<Node> operator()( TreePtr<Node> context,       // The whole program, so declarations may be searched for
    		                            TreePtr<Node> root ) = 0;   // Root of the subtree we want to modify

    inline TreePtr<Node> operator()( TreePtr<Node> root )
    {
    	return operator()( root, root );
    }

    // Apply this transformation to tree at *proot, using context for decls etc.
	// In-place version, node at proot will be updated to the new tree.
    virtual void operator()( TreePtr<Node> context,         // The whole program, so declarations may be searched for
    		                 TreePtr<Node> *proot ) = 0;    // Root of the subtree we want to modify

    inline void operator()( TreePtr<Node> *proot )
    {
    	operator()( *proot, proot );
    }
};


class InPlaceTransformation : public Transformation
{
public:
	using Transformation::operator();

	// Implement out-of-place in terms of in-place
    virtual TreePtr<Node> operator()( TreePtr<Node> context,
                                        TreePtr<Node> root )
    {
        ASSERTFAIL("TODO copy subtree");
    }    		                             
};


class OutOfPlaceTransformation : public Transformation
{
public:
	using Transformation::operator();

    // Implement in-place in terms of out-of-place
    virtual void operator()( TreePtr<Node> context,
    		                 TreePtr<Node> *proot )
    {
        TreePtr<Node> result_root = operator()( context, *proot );
        *proot = result_root;
    }    		                             
};

#endif
