#ifndef TYPEOF_HPP
#define TYPEOF_HPP

#include "transformation.hpp"
#include "tree/tree.hpp"

class TypeOf : public OutOfPlaceTransformation
{
public:
    virtual TreePtr<Node> operator()( TreePtr<Node> c, TreePtr<Node> root )
    {
    	context = c;
    	TreePtr<Expression> e = dynamic_pointer_cast<Expression>(root);
    	TreePtr<Node> n;
    	if( e ) // if the tree at root is not an expression, return NULL
    		n = Get( e );
    	context = TreePtr<Node>();
    	return n;
    }
    // TODO make these private
    TreePtr<Type> Get( TreePtr<Expression> o );
    TreePtr<Type> Get( TreePtr<Operator> op, Sequence<Type> optypes );
    TreePtr<Type> GetStandard( Sequence<Type> &optypes );
    TreePtr<Type> GetStandard( Sequence<Numeric> &optypes );
    TreePtr<Type> GetSpecial( TreePtr<Operator> op, Sequence<Type> &optypes );
    TreePtr<Type> GetLiteral( TreePtr<Literal> l );

    // Is this call really a constructor call? If so return the object being
    // constructed. Otherwise, return NULL
    TreePtr<Expression> IsConstructorCall( TreePtr<Node> c, TreePtr<Call> call );

    static TypeOf instance; 
private:
    TreePtr<Node> context;
};

#endif
