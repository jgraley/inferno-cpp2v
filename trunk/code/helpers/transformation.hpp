#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "common/refcount.hpp"
#include "tree/generics.hpp"

class Transformation
{
public:
    // Apply this transformation to tree at root, using context for decls etc.
	// Out-of-place version, neither context nor tree may be modified.
    virtual SharedPtr<Node> operator()( SharedPtr<Node> context,       // The whole program, so declarations may be searched for
    		                            SharedPtr<Node> root ) = 0;   // Root of the subtree we want to modify

    // Apply this transformation to tree at *proot, using context for decls etc.
	// In-place version, node at proot will be updated to the new tree.
    virtual void operator()( SharedPtr<Node> context,         // The whole program, so declarations may be searched for
    		                 SharedPtr<Node> *proot ) = 0;    // Root of the subtree we want to modify
};


class InPlaceTransformation : public Transformation
{
public:
    // Implement out-of-place in terms of in-place
    virtual SharedPtr<Node> operator()( SharedPtr<Node> context,       
                                        SharedPtr<Node> root )
    {
        ASSERTFAIL("Should never get called"); // Since final class should have overridden 
        // the other op() and C++ is stupid so won't call this alternatitve overload
        // on a base class
    }    		                             
};


class OutOfPlaceTransformation : public Transformation
{
public:
    // Implement in-place in terms of out-of-place
    virtual void operator()( SharedPtr<Node> context,
    		                 SharedPtr<Node> *proot )
    {
        ASSERTFAIL("Should never get called"); // Since final class should have overridden 
        // the other op() and C++ is stupid so won't call this alternatitve overload
        // on a base class

        //SharedPtr<Node> result_root = Transformation::operator()( context, *proot );
        //*proot = result_root; 
    }    		                             
};

#endif
