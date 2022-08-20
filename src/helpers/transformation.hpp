#ifndef TRANSFORMATION_HPP
#define TRANSFORMATION_HPP

#include "node/specialise_oostd.hpp"
#include "node/graphable.hpp"
#include <functional>

class Transformation : public virtual Graphable
{
public:
    // Apply this transformation to tree at root, using context for decls etc.
    virtual TreePtr<Node> operator()( TreePtr<Node> context,      // The whole program, for searches
    		                          TreePtr<Node> root ) = 0;   // Root of the subtree we want to modify
};

#endif
