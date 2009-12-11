#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "common/refcount.hpp"

class Transformation
{
public:
    // Apply this transformation to the supplied subtree
    virtual void operator()( shared_ptr<Node> context,  // The whole program, so declarations may be searched for
    		                 shared_ptr<Node> root )    // Root of the subtree we want to modify
    		                 = 0;
    void operator()( shared_ptr<Node> root )
    {
    	operator()(root, root);
    }
};

#endif
