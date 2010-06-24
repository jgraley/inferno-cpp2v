#ifndef TYPEOF_HPP
#define TYPEOF_HPP

#include "transformation.hpp"
#include "search_replace.hpp"
#include "tree/tree.hpp"
#include "soft_patterns.hpp"

// TODO rename to something like GetType

class TypeOf : public OutOfPlaceTransformation, public TransformTo<Expression>
{
public:
	NODE_FUNCTIONS
    TypeOf() : TransformTo<Expression>( this )
    {
    }
    virtual SharedPtr<Node> operator()( SharedPtr<Node> c, SharedPtr<Node> root )
    {
    	context = c;
    	SharedPtr<Expression> e = dynamic_pointer_cast<Expression>(root);
    	SharedPtr<Node> n;
    	if( e ) // if the tree at root is not an expression, return NULL
    		n = Get( e );
    	context = SharedPtr<Node>();
    	return n;
    }
    SharedPtr<Type> Get( SharedPtr<Expression> o );
    SharedPtr<Type> Get( SharedPtr<Operator> op, Sequence<Type> optypes );
    SharedPtr<Type> GetStandard( Sequence<Type> &optypes );
    SharedPtr<Type> GetStandard( Sequence<Numeric> &optypes );
    SharedPtr<Type> GetSpecial( SharedPtr<Operator> op, Sequence<Type> &optypes );
    SharedPtr<Type> GetLiteral( SharedPtr<Literal> l );

    // Is this call really a constructor call? If so return the object being
    // constructed. Otherwise, return NULL
    SharedPtr<Expression> IsConstructorCall( SharedPtr<Node> c, SharedPtr<Call> call );

private:
    SharedPtr<Node> context;
};


#endif
