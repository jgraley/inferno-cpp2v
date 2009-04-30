#ifndef TYPEOF_HPP
#define TYPEOF_HPP

#include "tree/tree.hpp"
#include "helpers/walk.hpp"
#include "helpers/misc.hpp"



class TypeOf
{
public:
    shared_ptr<Type> Get( shared_ptr<Program> program, shared_ptr<Expression> o );
    
    // Is this call really a constructor call? If so return the object being
    // constructoed. Otherwise, return NULL
    static shared_ptr<Expression> IsConstructorCall( shared_ptr<Program> program, shared_ptr<Call> call );
};

#endif
