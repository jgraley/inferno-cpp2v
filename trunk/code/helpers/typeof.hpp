#ifndef TYPEOF_HPP
#define TYPEOF_HPP

#include "tree/tree.hpp"
#include "helpers/walk.hpp"
#include "helpers/misc.hpp"



class TypeOf
{
public:
    shared_ptr<Type> Get( shared_ptr<Program> program, shared_ptr<Operand> o )
    {
        ASSERT(o);
        
        if( shared_ptr<InstanceIdentifier> ii = dynamic_pointer_cast<InstanceIdentifier>(o) ) // object or funciton instance
        {        
            shared_ptr<Instance> i = GetDeclaration(program, ii);
            return i->type; 
        }
            
        else if( shared_ptr<Operator> op = dynamic_pointer_cast<Operator>(o) ) // operator
        {
            // Get the types of all the operands to the operator first
            Sequence<Type> optypes;
            FOREACH( shared_ptr<Operand> o, op->operands )
                optypes.push_back( Get(program, o) );
                
            // then handle based on the kind of operator
            if( dynamic_pointer_cast<Dereference>(op) )
            {
                shared_ptr<Pointer> o2 = dynamic_pointer_cast<Pointer>( optypes[0] );
                ASSERT( o2 && "dereferenciung non-pointer" );
                return o2->destination;
            }
            else
            {
                 ASSERT(!"Unknown operator, please add");         
            }
        }
        
        else if( shared_ptr<Lookup> l = dynamic_pointer_cast<Lookup>(o) ) // a.b; just return type of b
           return Get( program, l->member );
           
        else 
        {
            ASSERT(!"Unknown expression, please add");             
        }
    }
};

// Is this call really a constructor call? If so return the object being
// constructoed. Otherwise, return NULL
shared_ptr<Operand> IsConstructorCall( shared_ptr<Program> program, shared_ptr<Call> call )
{
    shared_ptr<Lookup> lf = dynamic_pointer_cast<Lookup>(call->function);            
    if(!lf)
        return shared_ptr<Operand>();
        
    ASSERT(lf->member);
    if( dynamic_pointer_cast<Constructor>( TypeOf().Get( program, lf->member ) ) )
        return lf->base;
    else
        return shared_ptr<Operand>();
}

#endif
