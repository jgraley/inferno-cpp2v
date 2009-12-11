
#include "tree/tree.hpp"
#include "tree/type_db.hpp"
#include "helpers/walk.hpp"
#include "helpers/misc.hpp"
#include "typeof.hpp"

#define INT 0

shared_ptr<Type> TypeOf::Get( shared_ptr<Expression> o )
{
    ASSERT(o);
    
    if( shared_ptr<SpecificInstanceIdentifier> ii = dynamic_pointer_cast<SpecificInstanceIdentifier>(o) ) // object or function instance
    {        
        shared_ptr<Instance> i = GetDeclaration(context, shared_ptr<InstanceIdentifier>(ii));
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
            ASSERT(0)("Incorrect type ")(*t)(" coming before []");
            ASSERTFAIL("");
        }            
    }
               
    else if( shared_ptr<SpecificInteger> si = dynamic_pointer_cast<SpecificInteger>(o) )
    {
    	// Get the info from Clang, and make an Inferno type for it
    	shared_ptr<Integral> it;
        if( si->isSigned() )
        	it = shared_new<Signed>();
        else
        	it = shared_new<Unsigned>();
        it->width = shared_ptr<SpecificInteger>( new SpecificInteger( si->getBitWidth() ) );
        return it;
    }

    else if( shared_ptr<Cast> c = dynamic_pointer_cast<Cast>(o) )
    {
        return c->type;
    }

    else 
    {
        ASSERT(0)("Unknown expression ")(*o)(", please add to TypeOf class");
        ASSERTFAIL("");
    }
}

// Just discover the type of operators, where the types of the operands have already been determined
// Note we always get a Sequence, even when the operator is commutative
shared_ptr<Type> TypeOf::Get( shared_ptr<Operator> op, Sequence<Type> optypes )
{
	// Lower types that masquerade as other types in preparation for operand analysis
	// - References go to the referenced type
	// - Arrays go to pointers
	for( int i=0; i<optypes.size(); i++ )
	{
		while( shared_ptr<Reference> r = dynamic_pointer_cast<Reference>(optypes[i]) )
			optypes[i] = r->destination;
		if( shared_ptr<Array> a = dynamic_pointer_cast<Array>(optypes[i]) )
		{
			shared_ptr<Pointer> p( new Pointer );
			p->destination = a->element;
			optypes[i] = p;
		}
		// Check we finished the job
		ASSERT( !dynamic_pointer_cast<Reference>(optypes[i]) );
		ASSERT( !dynamic_pointer_cast<Array>(optypes[i]) );
	}

	// Assignment operators return their left-hand operand type in all cases
	if( dynamic_pointer_cast<AssignmentOperator>(op) )
	{
		return optypes[0];
	}

	// Pointer arithmetic: an add involving a pointer returns that pointer type
	if( dynamic_pointer_cast<Add>(op) )
	{
		for( int i=0; i<optypes.size(); i++ )
			if( shared_ptr<Pointer> p = dynamic_pointer_cast<Pointer>(optypes[i]) )
		        return p;
	}

	// Pointer arithmetic: a subtract involving a pointer returns int
	if( dynamic_pointer_cast<Subtract>(op) )
	{
		for( int i=0; i<optypes.size(); i++ )
			if( shared_ptr<Pointer> p = dynamic_pointer_cast<Pointer>(optypes[i]) )
			{
                shared_ptr<Signed> i = shared_ptr<Signed>( new Signed );
                shared_ptr<SpecificInteger> nc( new SpecificInteger(TypeDb::integral_bits[INT]) );
                i->width = nc;
                return i;
			}
	}

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
        ASSERT(0)("Unknown operator ")(*op)(" (not in operator_db.inc), please add to TypeOf class");
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

	if( optypes.size() == 2 )
		ASSERT(0)("Standard operator with operands ")(*optypes[0])(*optypes[1])(" is unknown usage, please add to TypeOf class");
	else
		ASSERT(0)("Standard operator with ")(*optypes[0])(" is unknown usage, please add to TypeOf class");
    ASSERTFAIL();
}


shared_ptr<Type> TypeOf::GetStandard( Sequence<Numeric> &optypes )
{
	// Start the width and signedness as per regular "int" since this is the
	// minimum result type for standard operators
	shared_ptr<SpecificInteger> maxwidth_signed( new SpecificInteger(TypeDb::integral_bits[INT]) );
	shared_ptr<SpecificInteger> maxwidth_unsigned;
	shared_ptr<SpecificFloatSemantics> maxwidth_float;

	// Look at the operands in turn
	for( int i=0; i<optypes.size(); i++ )
	{
		// Floats take priority
		if( shared_ptr<Floating> f = dynamic_pointer_cast<Floating>(optypes[i]) )
		{
			shared_ptr<SpecificFloatSemantics> sfs = dynamic_pointer_cast<SpecificFloatSemantics>(f->semantics);
			ASSERT(sfs)("Floating point type seen with semantics not specific");
			unsigned int sl = llvm::APFloat::semanticsPrecision( *sfs );
			unsigned int sr = llvm::APFloat::semanticsPrecision( *maxwidth_float );
			if( !maxwidth_float || sl > sr )
				maxwidth_float = sfs;
		}

		// Should only have Integrals from here on
		shared_ptr<Integral> intop = dynamic_pointer_cast<Integral>(optypes[i]);
        ASSERT( intop )(*optypes[i])(" is not Floating or Integral, please add to TypeOf class" );

        // Do a max algorithm on the width
		shared_ptr<SpecificInteger> width = dynamic_pointer_cast<SpecificInteger>(intop->width);
		ASSERT( width )( "Integral size ")(*(intop->width))(" is not specific, cannot decide result type");

		if( dynamic_pointer_cast<Signed>(optypes[i]) )
		{
			if( *width >= *maxwidth_signed )
		    	maxwidth_signed = width;
		}
		else if( dynamic_pointer_cast<Unsigned>(optypes[i]) )
		{
			if( !maxwidth_unsigned || *width >= *maxwidth_unsigned )
				maxwidth_unsigned = width;
		}
		else
			ASSERT( 0 )(*intop)(" is not Signed or Unsigned, please add to TypeOf class");
	}

	if( maxwidth_float )
	{
		shared_ptr<Floating> result;
		result->semantics = shared_ptr<SpecificFloatSemantics>( new SpecificFloatSemantics(*maxwidth_float) );
		return result;
	}

	// Build the required integral result type
	shared_ptr<Integral> result;
	if( maxwidth_unsigned && *maxwidth_unsigned >= *maxwidth_signed )
	{
		result = shared_ptr<Integral>( new Unsigned );
		result->width = maxwidth_unsigned;
	}
	else
	{
		result = shared_ptr<Integral>( new Signed );
		result->width = maxwidth_signed;
	}
	return result;
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
            ASSERTFAIL( "dereferencing non-pointer" );
    }
    else if( dynamic_pointer_cast<AddressOf>(op) )
    {
        shared_ptr<Pointer> p( new Pointer );
        p->destination = optypes[0];
        return p;
    }
    else if( dynamic_pointer_cast<Comma>(op) )
    {
        ASSERT( optypes.size() == 2 );
        return optypes[1];
    }
    else
    {
        ASSERT(0)("Unknown \"SPECIAL\" operator ")(*op)(", please add to TypeOf class");
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
