#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "node/specialise_oostd.hpp"
#include "node/graphable.hpp"
#include <functional>

class Transformation : public virtual Graphable
{
public:
    // Apply this transformation to tree at root, using context for decls etc.
	// Out-of-place version, neither context nor tree may be modified.
    virtual TreePtr<Node> operator()( TreePtr<Node> context,      // The whole program, so declarations may be searched for
    		                          TreePtr<Node> root ) = 0;   // Root of the subtree we want to modify
};


class Filter
{
public:
    // Test the subtree at the supplied root, producing true or false
    virtual bool IsMatch( TreePtr<Node> context,       // The whole program, so declarations may be searched for
                          TreePtr<Node> root ) = 0;    // Root of the subtree we want to test
};


// If you wish to use a lambda...
struct LambdaFilter : public Filter
{
    typedef function<bool(TreePtr<Node>, TreePtr<Node>)> Lambda;
    LambdaFilter( const Lambda &lambda_ ) : lambda( lambda_ ) {}
    virtual bool IsMatch( TreePtr<Node> context,
                          TreePtr<Node> root )
    {
        return lambda(context, root);
    }
    Lambda lambda;
};

#endif
