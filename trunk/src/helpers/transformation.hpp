#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "node/node.hpp"
#include "common/trace.hpp"

// TODO For in-place, use (*pcontext, *proot) rather than (context, *root). This 
// way the caller has to decide whether the context node is the same as the root node, 
// or some ancestor. If proot==pcontext, then writes through proot will magically 
// become visible through pcontext. If they differ, assume *proot is under *pcontext, 
// and therefore *pcontext does not need to change. Add an assert somewhere that 
// *proot is indeed somewhere under *pcontext (or they are the same).
// Then remove the if( context == *proot ) checks from TransformationVector
// and CompareReplace. Really, this TODO is just to use these two local 
// policies as the general policy.

class Transformation : public Traceable
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
    
    // Provide information for the graph plotter, so it can draw the tx as a node
    // with 0 or more links to TreePtrs
    virtual void GetGraphInfo( vector<string> *labels, 
                               vector< TreePtr<Node> > *links ) const 
    {
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

class TransformationVector : public vector< shared_ptr<Transformation> >,
                             public InPlaceTransformation
{
    virtual void operator()( TreePtr<Node> context,     // The whole program, so declarations may be searched for
		                     TreePtr<Node> *proot )     // Root of the subtree we want to modify
    {
        TreePtr<Node> *pcontext;
        if( context = *proot )
            pcontext = proot;
        else 
            pcontext = &context;
        FOREACH( shared_ptr<Transformation> t, *this )
            (*t)(*pcontext, proot);
    }		                           
};



class Filter
{
public:
    // Test the subtree at the supplied root, producing true or false
    virtual bool IsMatch( TreePtr<Node> context,       // The whole program, so declarations may be searched for
                          TreePtr<Node> root ) = 0;   // Root of the subtree we want to test

    inline bool IsMatch( TreePtr<Node> root )
    {
        return IsMatch( root, root );
    }
};


#endif
