#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "common/refcount.hpp"
#include "tree/generics.hpp"

class Transformation
{
public:
    // Apply this transformation to tree at root, using context for decls etc.
	// Out-of-place version, neither context nor tree may be modified.
    virtual shared_ptr<Node> operator()( shared_ptr<Node> context,  // The whole program, so declarations may be searched for
    		                             shared_ptr<Node> root )    // Root of the subtree we want to modify
    {
    	ASSERTFAIL("Out-of-place transformation not overridden");
    }

    // Apply this transformation to tree at *proot, using context for decls etc.
	// In-place version, node at proot will be updated to the new tree.
    virtual void operator()( shared_ptr<Node> context,  // The whole program, so declarations may be searched for
    		                 shared_ptr<Node> *proot )    // Root of the subtree we want to modify
    {
    	ASSERTFAIL("In-place transformation not overridden");
    }

    // Shorthand for the above
    void operator()( shared_ptr<Node> root )
    {
    	operator()(root, root);
    }

    void operator()( shared_ptr<Node> *proot )
    {
    	operator()(*proot, proot);
    }
};

#endif
