#ifndef TYPEOF_HPP
#define TYPEOF_HPP

#include "tree/tree.hpp"

class TypeOf
{
public:
    shared_ptr<Type> Get( shared_ptr<Operand> o )
    {
        if( shared_ptr<Instance> i = dynamic_pointer_cast<Instance>(o) ) // object or funciton instance
            return i->type; 
            
        else if( shared_ptr<Unary> ou = dynamic_pointer_cast<Unary>(o) ) // operator
        {
            // Get the types of all the operands to the operator first
            shared_ptr<Type> optype = Get(ou->operands[0]);
                           
            // then handle based on the kind of operator
            if( dynamic_pointer_cast<Dereference>(ou) )
            {
                shared_ptr<Pointer> o2 = dynamic_pointer_cast<Pointer>( optype );
                ASSERT( o2 && "dereferenciung non-pointer" );
                return o2->destination;
            }
            else
            {
                ASSERT(!"Unknown operator, please add");         
            }
        }
        
        else if( shared_ptr<Lookup> l = dynamic_pointer_cast<Lookup>(o) ) // a.b; just return type of b
           return Get( l->member );
           
        else 
            ASSERT(!"Unknown expression, please add");             
    }
};

#endif
