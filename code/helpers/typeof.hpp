#ifndef TYPEOF_HPP
#define TYPEOF_HPP

#include "tree/tree.hpp"
#include "helpers/walk.hpp"
#include "helpers/misc.hpp"



class TypeOf
{
public:
    TypeOf( shared_ptr<Node> c ) :
        context( c )
    {
    }
    
    shared_ptr<Type> Get( shared_ptr<Expression> o );
    shared_ptr<Type> Get( shared_ptr<Operator> op, Sequence<Type> optypes );
    shared_ptr<Type> GetStandard( Sequence<Type> &optypes );
    shared_ptr<Type> GetStandard( Sequence<Numeric> &optypes );
    shared_ptr<Type> GetSpecial( shared_ptr<Operator> op, Sequence<Type> &optypes );
    
    // Is this call really a constructor call? If so return the object being
    // constructed. Otherwise, return NULL
    shared_ptr<Expression> IsConstructorCall( shared_ptr<Call> call );

private:
    shared_ptr<Node> context;
};

#endif
