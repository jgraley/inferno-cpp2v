
#include "tree/tree.hpp"
#include "helpers/walk.hpp"
#include "helpers/misc.hpp"
#include "typeof.hpp"


shared_ptr<Type> TypeOf::Get( shared_ptr<Expression> o )
{
    ASSERT(o);
    
    if( shared_ptr<SpecificInstanceIdentifier> ii = dynamic_pointer_cast<SpecificInstanceIdentifier>(o) ) // object or function instance
    {        
        shared_ptr<Instance> i = GetDeclaration(program, ii);
        return i->type; 
    }
        
    else if( shared_ptr<NonCommutativeOperator> op = dynamic_pointer_cast<NonCommutativeOperator>(o) ) // operator
    {
        // Get the types of all the operands to the operator first
        Sequence<Type> optypes;
        FOREACH( shared_ptr<Expression> o, op->operands )
            optypes.push_back( Get(o) );
        return Get( op, optypes );
    }

    else if( shared_ptr<CommutativeOperator> op = dynamic_pointer_cast<CommutativeOperator>(o) ) // operator
    {
        // Get the types of all the operands to the operator first
        Sequence<Type> optypes;
        FOREACH( shared_ptr<Expression> o, op->operands )
                 optypes.push_back( Get(o) );
        return Get( op, optypes );
    }
    
    else if( shared_ptr<Call> c = dynamic_pointer_cast<Call>(o) )
    {
        shared_ptr<Type> t = Get(c->function); // get type of the function itself
        ASSERT( dynamic_pointer_cast<Subroutine>(t) )( "Trying to call something that is not a kind of Subroutine");
        if( shared_ptr<Function> f = dynamic_pointer_cast<Function>(t) )
        	return f->return_type;
        else
        	return shared_new<Void>();

    }
    else if( shared_ptr<Lookup> l = dynamic_pointer_cast<Lookup>(o) ) // a.b; just return type of b
    {
        return Get( l->member );
    }
               
    else 
    {
        ASSERT(0)("Unknown expression %s, please add to TypeOf class", typeid(*o).name());
        ASSERTFAIL("");
    }
}

// Just discover the type of operators, where the types of the operands have already been determined
// Note we alwayts get a Sequence, even when the operator is commutative
shared_ptr<Type> TypeOf::Get( shared_ptr<Operator> op, Sequence<Type> &optypes )
{
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
        ASSERT(0)("Unknown operator %s, please add to TypeOf class", typeid(*op).name());
        ASSERTFAIL("");
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
