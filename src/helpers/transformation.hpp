#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "node/specialise_oostd.hpp"
#include "node/graphable.hpp"
#include <functional>

// TODO For in-place, use (*pcontext, *proot) rather than (context, *root). This 
// way the caller has to decide whether the context node is the same as the root node, 
// or some ancestor. If proot==pcontext, then writes through proot will magically 
// become visible through pcontext. If they differ, assume *proot is under *pcontext, 
// and therefore *pcontext does not need to change. Add an assert somewhere that 
// *proot is indeed somewhere under *pcontext (or they are the same).
// Then remove the if( context == *proot ) checks from TransformationVector
// and CompareReplace. Really, this TODO is just to use these two local 
// policies as the general policy.

class Transformation : public virtual Graphable
{
public:
    // Apply this transformation to tree at root, using context for decls etc.
	// Out-of-place version, neither context nor tree may be modified.
    virtual TreePtr<Node> operator()( TreePtr<Node> context,      // The whole program, so declarations may be searched for
    		                          TreePtr<Node> root ) = 0;   // Root of the subtree we want to modify

    TreePtr<Node> operator()( TreePtr<Node> root );

    // Apply this transformation to tree at *proot, using context for decls etc.
	// In-place version, node at proot will be updated to the new tree.
    virtual void operator()( TreePtr<Node> context,         // The whole program, so declarations may be searched for
    		                 TreePtr<Node> *proot ) = 0;    // Root of the subtree we want to modify

    void operator()( TreePtr<Node> *proot );
    
    virtual void SetStopAfter( vector<int> ssa, int depth=0 );
};


class InPlaceTransformation : public Transformation
{
public:
	using Transformation::operator();

	// Implement out-of-place in terms of in-place
    virtual TreePtr<Node> operator()( TreePtr<Node> context,
                                      TreePtr<Node> root ) final;
};


class OutOfPlaceTransformation : public Transformation
{
public:
	using Transformation::operator();

    // Implement in-place in terms of out-of-place
    virtual void operator()( TreePtr<Node> context,
    		                 TreePtr<Node> *proot ) final;
};


class TransformationVector : public vector< shared_ptr<Transformation> >,
                             public InPlaceTransformation
{
public:
    TransformationVector();
        
    virtual void operator()( TreePtr<Node> context,     // The whole program, so declarations may be searched for
		                     TreePtr<Node> *proot );    // Root of the subtree we want to modify
                        
    virtual void SetStopAfter( vector<int> ssa, int d=0 );
    
private:    
    vector<int> stop_after;
    int depth;
};


class Filter
{
public:
    // Test the subtree at the supplied root, producing true or false
    virtual bool IsMatch( TreePtr<Node> context,       // The whole program, so declarations may be searched for
                          TreePtr<Node> root ) = 0;    // Root of the subtree we want to test

    bool IsMatch( TreePtr<Node> root );
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
