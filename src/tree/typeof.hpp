#ifndef TYPEOF_HPP
#define TYPEOF_HPP

#include "helpers/transformation.hpp"
#include "cpptree.hpp"

class TypeOf : public OutOfPlaceTransformation
{
public:
    virtual TreePtr<Node> operator()( TreePtr<Node> c, TreePtr<Node> root )
    {
    	context = c;
    	TreePtr<CPPTree::Expression> e = dynamic_pointer_cast<CPPTree::Expression>(root);
    	TreePtr<Node> n;
    	if( e ) // if the tree at root is not an expression, return NULL
    		n = Get( e );
    	context = TreePtr<Node>();
    	return n;
    }
    // TODO make these private
    TreePtr<CPPTree::Type> Get( TreePtr<CPPTree::Expression> o );
    TreePtr<CPPTree::Type> Get( TreePtr<CPPTree::Operator> op, Sequence<CPPTree::Type> optypes );
    TreePtr<CPPTree::Type> GetStandard( Sequence<CPPTree::Type> &optypes );
    TreePtr<CPPTree::Type> GetStandard( Sequence<CPPTree::Numeric> &optypes );
    TreePtr<CPPTree::Type> GetSpecial( TreePtr<CPPTree::Operator> op, Sequence<CPPTree::Type> &optypes );
    TreePtr<CPPTree::Type> GetLiteral( TreePtr<CPPTree::Literal> l );

    // Is this call really a constructor call? If so return the object being
    // constructed. Otherwise, return NULL
    TreePtr<CPPTree::Expression> IsConstructorCall( TreePtr<Node> c, TreePtr<CPPTree::Call> call );

    static TypeOf instance; 
private:
    TreePtr<Node> context;
};

#endif
