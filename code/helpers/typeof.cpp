
#include "tree/tree.hpp"
#include "tree/type_db.hpp"
#include "helpers/walk.hpp"
#include "helpers/misc.hpp"
#include "typeof.hpp"


shared_ptr<Type> TypeOf::Get( shared_ptr<Expression> o )
{
    ASSERT(o);
    
    if( shared_ptr<SpecificInstanceIdentifier> ii = dynamic_pointer_cast<SpecificInstanceIdentifier>(o) ) // object or function instance
    {        
        shared_ptr<Instance> i = GetDeclaration(program, shared_ptr<InstanceIdentifier>(ii));
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
        shared_ptr<Type> t = Get(c->callee); // get type of the function itself
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
               
    else if( shared_ptr<Subscript> su = dynamic_pointer_cast<Subscript>(o) ) // a[i] just return element type of a
    {
        shared_ptr<Type> t = Get( su->base ); // type of the thing to the left of []
        if( shared_ptr<Array> a = dynamic_pointer_cast<Array>(t) )
            return a->element; 
        else if( shared_ptr<Pointer> p = dynamic_pointer_cast<Pointer>(t) )
            return p->destination;
        else
        {
            ASSERT(0)("Incorrect type %s coming before []", typeid(*t).name());
            ASSERTFAIL("");
        }            
    }
               
    else if( shared_ptr<SpecificInteger> si = dynamic_pointer_cast<SpecificInteger>(o) )
    {
    	// Get the info from Clang, and make an Inferno type for it
    	shared_ptr<Integral> it;
        if( si->value.isSigned() )
        	it = shared_new<Signed>();
        else
        	it = shared_new<Unsigned>();
        it->width = shared_ptr<SpecificInteger>( new SpecificInteger( si->value.getBitWidth() ) );
        return it;
    }

    else if( shared_ptr<Cast> c = dynamic_pointer_cast<Cast>(o) )
    {
        return c->type;
    }

    else 
    {
        ASSERT(0)("Unknown expression %s, please add to TypeOf class", typeid(*o).name());
        ASSERTFAIL("");
    }
}

// Just discover the type of operators, where the types of the operands have already been determined
// Note we alwayts get a Sequence, even when the operator is commutative
shared_ptr<Type> TypeOf::Get( shared_ptr<Operator> op, Sequence<Type> optypes )
{
	// Replace References with the referee type since we are going to analyse it
	for( int i=0; i<optypes.size(); i++ )
		while( shared_ptr<Reference> r = dynamic_pointer_cast<Reference>(optypes[i]) )
			optypes[i] = r->destination;

#define ARITHMETIC GetStandard( optypes )
#define BITWISE GetStandard( optypes )
#define LOGICAL shared_new<Boolean>()
#define COMPARISON shared_new<Boolean>()
#define SHIFT optypes[0]
#define SPECIAL GetSpecial( op, optypes )

    if(0) {}
#define INFIX(TOK, TEXT, NODE, BASE, CAT) \
	else if( dynamic_pointer_cast<NODE>(op) ) \
		return CAT;
#define PREFIX(TOK, TEXT, NODE, BASE, CAT) \
	else if( dynamic_pointer_cast<NODE>(op) ) \
		return CAT;
#define POSTFIX(TOK, TEXT, NODE, BASE, CAT) \
	else if( dynamic_pointer_cast<NODE>(op) ) \
		return CAT;
#include "tree/operator_db.inc"
    else
    {
        ASSERT(0)("Unknown operator %s (not in operator_db.inc), please add to TypeOf class", typeid(*op).name());
        ASSERTFAIL("");
    }
}


shared_ptr<Type> TypeOf::GetStandard( Sequence<Type> &optypes )
{
	Sequence<Numeric> nums;
	for( int i=0; i<optypes.size(); i++ )
		if( shared_ptr<Numeric> n = dynamic_pointer_cast<Numeric>(optypes[i]) )
			nums.push_back(n);
	if( nums.size() == optypes.size() )
		return GetStandard( nums );

	// TODO deal with pointer arithmetic

	if( optypes.size() == 2 )
		ASSERT(0)("Standard operator with %s and %s is unknown usage, please add to TypeOf class", typeid(*optypes[0]).name(), typeid(*optypes[1]).name());
	else
		ASSERT(0)("Standard operator with %s is unknown usage, please add to TypeOf class", typeid(*optypes[0]).name());
    ASSERTFAIL();
}


shared_ptr<Type> TypeOf::GetStandard( Sequence<Numeric> &optypes )
{
	shared_ptr<SpecificInteger> maxwidth( new SpecificInteger(TypeDb::integral_bits[0]) );
	bool seen_unsigned=false;

	for( int i=0; i<optypes.size(); i++ )
	{
		int score;

		// Floats take priority
		if( dynamic_pointer_cast<Floating>(optypes[i]) )
			return optypes[i]; // TODO hack LLVM::FloatSemantics to get a bigness measure
		                       // note that this always prefers the left one

		shared_ptr<Integral> intop = dynamic_pointer_cast<Integral>(optypes[i]);
        ASSERT( intop )
                ("%s is not Floating or Integral, please add to TypeOf class", typeid(*intop).name() );

		shared_ptr<SpecificInteger> width = dynamic_pointer_cast<SpecificInteger>(intop->width);
		ASSERT( width )( "Integral size %s is not specific, cannot decide result type", typeid(*(intop->width)).name());
		if( width->value > maxwidth->value )
			maxwidth = width;

		if( dynamic_pointer_cast<Unsigned>(optypes[i]) )
			seen_unsigned = true;
	}

	// TODO round up ints to 32-bit
	// TODO other bizarre shit from the language spec
	shared_ptr<Integral> res;

	if( seen_unsigned )
		res = shared_ptr<Integral>( new Unsigned );
	else
		res = shared_ptr<Integral>( new Signed );

	res->width = maxwidth;

	return res;
}


shared_ptr<Type> TypeOf::GetSpecial( shared_ptr<Operator> op, Sequence<Type> &optypes )
{
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
        ASSERT(0)("Unknown \"SPECIAL\" operator %s, please add to TypeOf class", typeid(*op).name());
        ASSERTFAIL("");
    }
}

// Is this call really a constructor call? If so return the object being
// constructed. Otherwise, return NULL
shared_ptr<Expression> TypeOf::IsConstructorCall( shared_ptr<Call> call )
{
    shared_ptr<Lookup> lf = dynamic_pointer_cast<Lookup>(call->callee);
    if(!lf)
        return shared_ptr<Expression>();
        
    ASSERT(lf->member);
    if( dynamic_pointer_cast<Constructor>( Get( lf->member ) ) )
        return lf->base;
    else
        return shared_ptr<Expression>();
}
