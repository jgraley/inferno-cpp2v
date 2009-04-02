#ifndef TYPEOF_HPP
#define TYPEOF_HPP

#include "tree/tree.hpp"
#include "helpers/walk.hpp"
#include "helpers/misc.hpp"

shared_ptr<UserType> GetDeclaration( shared_ptr<Program> program, shared_ptr<TypeIdentifier> id )
{
	Flattener<UserType> walkr(program);
	FOREACH( shared_ptr<UserType> d, walkr )
	{
        if( id == GetIdentifier( d ) ) 
	        return d;
	}
	ASSERT(0);
}

// bypass typedefs
shared_ptr<Record> GetRecordDeclaration( shared_ptr<Program> program, shared_ptr<TypeIdentifier> id )
{
	shared_ptr<UserType> ut = GetDeclaration( program, id );
	while( shared_ptr<Typedef> td = dynamic_pointer_cast<Typedef>(ut) )
	{
	    shared_ptr<TypeIdentifier> ti = dynamic_pointer_cast<TypeIdentifier>(td->type);
	    if(ti)
	        ut = GetDeclaration(program, ti);
	    else
	        return shared_ptr<Record>(); // not a record
	}
	shared_ptr<Record> r = dynamic_pointer_cast<Record>(ut);
	ASSERT(r && "user type is not record or typedef lol what is it!!??");
	return r;
}

shared_ptr<Instance> GetDeclaration( shared_ptr<Program> program, shared_ptr<InstanceIdentifier> id )
{
	Flattener<Instance> walkr(program);
	FOREACH( shared_ptr<Instance> d, walkr )
	{
        if( id == GetIdentifier( d ) ) 
	        return d;
	}
	ASSERT(0);
}

/*            shared_ptr<Typedef> td = dynamic_pointer_cast<Typedef>(ut);
            if(td)
                return
*/

class TypeOf
{
public:
    shared_ptr<Type> Get( shared_ptr<Program> program, shared_ptr<Operand> o )
    {
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
            ASSERT(!"Unknown expression, please add");             
    }
};

// Is this call really a constructor call? If so return the object being
// constructoed. Otherwise, return NULL
shared_ptr<Operand> IsConstructorCall( shared_ptr<Program> program, shared_ptr<Call> call )
{
    shared_ptr<Lookup> lf = dynamic_pointer_cast<Lookup>(call->function);            
    if( lf && dynamic_pointer_cast<Constructor>( TypeOf().Get( program, lf->member ) ) )
        return lf->base;
    else
        return shared_ptr<Operand>();
}

#endif
