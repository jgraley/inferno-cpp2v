
#include "tree/tree.hpp"
#include "helpers/walk.hpp"
#include "helpers/misc.hpp"
#include "typeof.hpp"


shared_ptr<Type> TypeOf::Get( shared_ptr<Expression> o )
{
    ASSERT(o);
    
    if( shared_ptr<InstanceIdentifier> ii = dynamic_pointer_cast<InstanceIdentifier>(o) ) // object or function instance
    {        
        shared_ptr<Instance> i = GetDeclaration(program, ii);
        return i->type; 
    }
        
    else if( shared_ptr<Operator> op = dynamic_pointer_cast<Operator>(o) ) // operator
    {
        // Get the types of all the operands to the operator first
        Sequence<Type> optypes;
        FOREACH( shared_ptr<Expression> o, op->operands )
            optypes.push_back( Get(o) );
            
        // then handle based on the kind of operator
        if( dynamic_pointer_cast<Dereference>(op) )
        {
            if( shared_ptr<Pointer> o2 = dynamic_pointer_cast<Pointer>( optypes[0] ) )
                return o2->destination;
            else if( shared_ptr<Array> o2 = dynamic_pointer_cast<Array>( optypes[0] ) )
                return o2->element;
            else 
                ASSERTFAIL( "dereferenciung non-pointer" );
        }
        else if( dynamic_pointer_cast<AddressOf>(op) )
        {
            shared_ptr<Pointer> p( new Pointer );
            p->destination = optypes[0];
            return p;
        }
        else
        {
            ASSERTFAIL("Unknown operator, please add");         
        }
    }
    
    else if( shared_ptr<Lookup> l = dynamic_pointer_cast<Lookup>(o) ) // a.b; just return type of b
    {
        return Get( l->member );
    }
               
    else 
    {
        ASSERTFAIL("Unknown expression, please add");             
    }
}

// Is this call really a constructor call? If so return the object being
// constructed. Otherwise, return NULL
shared_ptr<Expression> TypeOf::IsConstructorCall( shared_ptr<Call> call )
{
    shared_ptr<Lookup> lf = dynamic_pointer_cast<Lookup>(call->function);            
    if(!lf)
        return shared_ptr<Expression>();
        
    ASSERT(lf->member);
    if( dynamic_pointer_cast<Constructor>( Get( lf->member ) ) )
        return lf->base;
    else
        return shared_ptr<Expression>();
}
