#ifndef TYPEOF_HPP
#define TYPEOF_HPP

#include "transformation.hpp"
#include "search_replace.hpp"
#include "tree/tree.hpp"
#include "soft_patterns.hpp"

class TypeOf : public Transformation, public TransformTo<Expression>
{
public:
    TypeOf() : TransformTo<Expression>( this )
    {
    }
    virtual shared_ptr<Node> operator()( shared_ptr<Node> c, shared_ptr<Node> root )
    {
    	context = c;
    	shared_ptr<Expression> e = dynamic_pointer_cast<Expression>(root);
    	shared_ptr<Node> n;
    	if( e ) // if the tree at root is not an expression, return NULL
    		n = Get( e );
    	context = shared_ptr<Node>();
    	return n;
    }
    shared_ptr<Type> Get( shared_ptr<Expression> o );
    shared_ptr<Type> Get( shared_ptr<Operator> op, Sequence<Type> optypes );
    shared_ptr<Type> GetStandard( Sequence<Type> &optypes );
    shared_ptr<Type> GetStandard( Sequence<Numeric> &optypes );
    shared_ptr<Type> GetSpecial( shared_ptr<Operator> op, Sequence<Type> &optypes );
    
    // Is this call really a constructor call? If so return the object being
    // constructed. Otherwise, return NULL
    shared_ptr<Expression> IsConstructorCall( shared_ptr<Node> c, shared_ptr<Call> call );

private:
    shared_ptr<Node> context;
};


#endif
