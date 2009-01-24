
#include "tree/tree.hpp"

class TypeOf
{
public:
    shared_ptr<Type> Get( shared_ptr<Operand> o )
    {
        if( shared_ptr<Instance> i = dynamic_pointer_cast<Instance>(o) ) // object or funciton instance
            return i->type; 
            
        else if( shared_ptr<Operator> op = dynamic_pointer_cast<Operator>(o) ) // operator
        {
            // Get the types of all the operands to the operator first
            Sequence<Type> optypes;
            FOREACH( shared_ptr<Operand> o, op->operands )
                optypes.push_back( Get(o) );
                
            // then handle based on the kind of operator
            switch( op->kind )
            {
            case clang::tok::star:
                {
                    shared_ptr<Pointer> o2 = dynamic_pointer_cast<Pointer>( optypes[0] );
                    ASSERT( o2 && "dereferenciung non-pointer" );
                    return o2->destination;
                }
                
            default:
                 ASSERT(!"Unknown operator, please add");         
            }
        }
        
        else if( shared_ptr<Lookup> l = dynamic_pointer_cast<Lookup>(o) ) // a.b; just return type of b
           return Get( l->member );
           
        else 
            ASSERT(!"Unknown expression, please add");             
    }
};

