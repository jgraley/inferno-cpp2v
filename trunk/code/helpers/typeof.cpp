
#include "tree/tree.hpp"
#include "tree/type_db.hpp"
#include "helpers/walk.hpp"
#include "helpers/misc.hpp"
#include "typeof.hpp"
#include "tree/tree.hpp"
#include "helpers/walk.hpp"
#include "helpers/misc.hpp"

#define INT 0

SharedPtr<Type> TypeOf::Get( SharedPtr<Expression> o )
{
    ASSERT(o);
    
    if( SharedPtr<SpecificInstanceIdentifier> ii = dynamic_pointer_cast<SpecificInstanceIdentifier>(o) ) // object or function instance
    {        
        SharedPtr<Node> n = GetDeclaration()(context, ii);
        SharedPtr<Instance> i = dynamic_pointer_cast<Instance>(n);
        ASSERT(i);
        return i->type; 
    }
    else if( SharedPtr<NonCommutativeOperator> op = dynamic_pointer_cast<NonCommutativeOperator>(o) ) // operator
    {
        // Get the types of all the operands to the operator first
        Sequence<Type> optypes;
        FOREACH( SharedPtr<Expression> o, op->operands )
            optypes.push_back( Get(o) );
        return Get( op, optypes );
    }
    else if( SharedPtr<CommutativeOperator> op = dynamic_pointer_cast<CommutativeOperator>(o) ) // operator
    {
        // Get the types of all the operands to the operator first
        Sequence<Type> optypes;
        FOREACH( SharedPtr<Expression> o, op->operands )
                 optypes.push_back( Get(o) );
        return Get( op, optypes );
    }
    else if( SharedPtr<Literal> l = dynamic_pointer_cast<Literal>(o) )
    {
        return GetLiteral( l );
    }
    else if( SharedPtr<Call> c = dynamic_pointer_cast<Call>(o) )
    {
        SharedPtr<Type> t = Get(c->callee); // get type of the function itself
        ASSERT( dynamic_pointer_cast<Subroutine>(t) )( "Trying to call something that is not a kind of Subroutine");
        if( SharedPtr<Function> f = dynamic_pointer_cast<Function>(t) )
        	return f->return_type;
        else
        	return shared_new<Void>();
    }
    else if( SharedPtr<Lookup> l = dynamic_pointer_cast<Lookup>(o) ) // a.b; just return type of b
    {
        return Get( l->member );
    }
    else if( SharedPtr<Cast> c = dynamic_pointer_cast<Cast>(o) )
    {
        return c->type;
    }
    else if( SharedPtr<MakeRecord> rl = dynamic_pointer_cast<MakeRecord>(o) )
    {
        return rl->type;
    }
    else if( SharedPtr<TypeOf> to = dynamic_pointer_cast<TypeOf>(o) )
    {
    	SharedPtr<Type> t = dynamic_pointer_cast<Type>(to->pattern); // get the pattern from the TransformTo base class
    	ASSERT( t );
        return t;
    }
    else if( dynamic_pointer_cast<LabelIdentifier>(o) )
    {
        return MakeShared<Type>(); // TODO labels need a type
    }
    else if( dynamic_pointer_cast<SizeOf>(o) || dynamic_pointer_cast<AlignOf>(o))
    {
    	SharedPtr<Integral> n;
    	if( TypeDb::int_default_signed )
    		n = MakeShared<Signed>();
    	else
    		n = MakeShared<Unsigned>();
    	SharedPtr<SpecificInteger> sz( new SpecificInteger(TypeDb::integral_bits[INT]) );
    	n->width = sz;
       return n;
    }
    else if( SharedPtr<New> n = dynamic_pointer_cast<New>(o) )
    {
        SharedPtr<Pointer> p( new Pointer );
        p->destination = n->type;
        return p;
    }
    else if( dynamic_pointer_cast<Delete>(o) )
    {
        return MakeShared<Type>();
    }
    else 
    {
        ASSERT(0)("Unknown expression ")(*o)(", please add to TypeOf class");
        ASSERTFAIL("");
    }
}

// Just discover the type of operators, where the types of the operands have already been determined
// Note we always get a Sequence, even when the operator is commutative
SharedPtr<Type> TypeOf::Get( SharedPtr<Operator> op, Sequence<Type> optypes )
{
	// Lower types that masquerade as other types in preparation for operand analysis
	// - References go to the referenced type
	// - Arrays go to pointers
	for( int i=0; i<optypes.size(); i++ )
	{
		while( SharedPtr<Reference> r = dynamic_pointer_cast<Reference>(optypes[i]) )
			optypes[i] = r->destination;
		if( SharedPtr<Array> a = dynamic_pointer_cast<Array>(optypes[i]) )
		{
			SharedPtr<Pointer> p( new Pointer );
			p->destination = a->element;
			optypes[i] = p;
		}
		// Check we finished the job
		ASSERT( !dynamic_pointer_cast<Reference>(optypes[i]) );
		ASSERT( !dynamic_pointer_cast<Array>(optypes[i]) );
	}

    if( SharedPtr<MakeArray> al = dynamic_pointer_cast<MakeArray>(op) )
    {
    	SharedPtr<Array> a( new Array );
    	a->element = optypes[0];
    	SharedPtr<SpecificInteger> sz( new SpecificInteger(optypes.size()) );
    	a->size = sz;
        return a;
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
			if( SharedPtr<Pointer> p = dynamic_pointer_cast<Pointer>(optypes[i]) )
		        return p;
	}

	// Pointer arithmetic: a subtract involving a pointer returns int
	if( dynamic_pointer_cast<Subtract>(op) )
	{
		for( int i=0; i<optypes.size(); i++ )
			if( SharedPtr<Pointer> p = dynamic_pointer_cast<Pointer>(optypes[i]) )
			{
                SharedPtr<Signed> i = SharedPtr<Signed>( new Signed );
                SharedPtr<SpecificInteger> nc( new SpecificInteger(TypeDb::integral_bits[INT]) );
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
#define OTHER(TOK, TEXT, NODE, BASE, CAT) \
	else if( dynamic_pointer_cast<NODE>(op) ) \
		return CAT;
#include "tree/operator_db.inc"
    else
    {
        ASSERT(0)("Unknown operator ")(*op)(" (not in operator_db.inc), please add to TypeOf class");
        ASSERTFAIL("");
    }
}


SharedPtr<Type> TypeOf::GetStandard( Sequence<Type> &optypes )
{
	Sequence<Numeric> nums;
	for( int i=0; i<optypes.size(); i++ )
		if( SharedPtr<Numeric> n = dynamic_pointer_cast<Numeric>(optypes[i]) )
			nums.push_back(n);
	if( nums.size() == optypes.size() )
		return GetStandard( nums );

	if( optypes.size() == 2 )
		ASSERT(0)("Standard operator with operands ")(*optypes[0])(*optypes[1])(" is unknown usage, please add to TypeOf class");
	else
		ASSERT(0)("Standard operator with ")(*optypes[0])(" is unknown usage, please add to TypeOf class");
    ASSERTFAIL();
}


SharedPtr<Type> TypeOf::GetStandard( Sequence<Numeric> &optypes )
{
	// Start the width and signedness as per regular "int" since this is the
	// minimum result type for standard operators
	SharedPtr<SpecificInteger> maxwidth_signed( new SpecificInteger(TypeDb::integral_bits[INT]) );
	SharedPtr<SpecificInteger> maxwidth_unsigned;
	SharedPtr<SpecificFloatSemantics> maxwidth_float;

	// Look at the operands in turn
	for( int i=0; i<optypes.size(); i++ )
	{
		// Floats take priority
		if( SharedPtr<Floating> f = dynamic_pointer_cast<Floating>(optypes[i]) )
		{
			SharedPtr<SpecificFloatSemantics> sfs = dynamic_pointer_cast<SpecificFloatSemantics>(f->semantics);
			ASSERT(sfs)("Floating point type seen with semantics not specific");
			unsigned int sl = llvm::APFloat::semanticsPrecision( *sfs );
			unsigned int sr = llvm::APFloat::semanticsPrecision( *maxwidth_float );
			if( !maxwidth_float || sl > sr )
				maxwidth_float = sfs;
		}

		// Should only have Integrals from here on
		SharedPtr<Integral> intop = dynamic_pointer_cast<Integral>(optypes[i]);
        ASSERT( intop )(*optypes[i])(" is not Floating or Integral, please add to TypeOf class" );

        // Do a max algorithm on the width
		SharedPtr<SpecificInteger> width = dynamic_pointer_cast<SpecificInteger>(intop->width);
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
		SharedPtr<Floating> result;
		result->semantics = SharedPtr<SpecificFloatSemantics>( new SpecificFloatSemantics(*maxwidth_float) );
		return result;
	}

	// Build the required integral result type
	SharedPtr<Integral> result;
	if( maxwidth_unsigned && *maxwidth_unsigned >= *maxwidth_signed )
	{
		result = SharedPtr<Integral>( new Unsigned );
		result->width = maxwidth_unsigned;
	}
	else
	{
		result = SharedPtr<Integral>( new Signed );
		result->width = maxwidth_signed;
	}
	return result;
}


SharedPtr<Type> TypeOf::GetSpecial( SharedPtr<Operator> op, Sequence<Type> &optypes )
{
    if( dynamic_pointer_cast<Dereference>(op) || dynamic_pointer_cast<Subscript>(op) )
    {
        if( SharedPtr<Pointer> o2 = dynamic_pointer_cast<Pointer>( optypes[0] ) )
            return o2->destination;
        else if( SharedPtr<Array> o2 = dynamic_pointer_cast<Array>( optypes[0] ) )
            return o2->element;
        else
            ASSERTFAIL( "dereferencing non-pointer" );
    }
    else if( dynamic_pointer_cast<AddressOf>(op) )
    {
        SharedPtr<Pointer> p( new Pointer );
        p->destination = optypes[0];
        return p;
    }
    else if( dynamic_pointer_cast<Comma>(op) )
    {
        ASSERT( optypes.size() == 2 );
        return optypes[1];
    }
    else if( dynamic_pointer_cast<Multiplexor>(op) )
	{
		return optypes[1]; // TODO do this properly, consider cond ? NULL : &x
	}
    else if( dynamic_pointer_cast<This>(op) )
    {
    	ASSERTFAIL(""); // TODO
    }
    else
    {
        ASSERT(0)("Unknown SPECIAL operator ")(*op)(", please add to TypeOf class");
        ASSERTFAIL("");
    }
}

SharedPtr<Type> TypeOf::GetLiteral( SharedPtr<Literal> l )
{
    if( SharedPtr<SpecificInteger> si = dynamic_pointer_cast<SpecificInteger>(l) )
    {
    	// Get the info from Clang, and make an Inferno type for it
    	SharedPtr<Integral> it;
        if( si->isSigned() )
        	it = shared_new<Signed>();
        else
        	it = shared_new<Unsigned>();
        it->width = SharedPtr<SpecificInteger>( new SpecificInteger( si->getBitWidth() ) );
        return it;
    }
    else if( SharedPtr<SpecificFloat> sf = dynamic_pointer_cast<SpecificFloat>(l) )
    {
    	// Get the info from Clang, and make an Inferno type for it
    	MakeShared<Floating> ft;
    	ft->semantics = SharedPtr<SpecificFloatSemantics>( new SpecificFloatSemantics(&sf->getSemantics()) );
        return ft;
    }
    else if( dynamic_pointer_cast<Bool>(l) )
    {
        return MakeShared<Boolean>();
    }
    else if( dynamic_pointer_cast<String>(l) )
    {
    	SharedPtr<Integral> n;
    	if( TypeDb::char_default_signed )
    		n = MakeShared<Signed>();
    	else
    		n = MakeShared<Unsigned>();
    	SharedPtr<SpecificInteger> sz( new SpecificInteger(TypeDb::char_bits) );
    	n->width = sz;
    	MakeShared<Pointer> p;
    	p->destination = n;
        return p;
    }
    else
    {
        ASSERT(0)("Unknown literal ")(*l)(", please add to TypeOf class");
        ASSERTFAIL("");
    }
}


// Is this call really a constructor call? If so return the object being
// constructed. Otherwise, return NULL
SharedPtr<Expression> TypeOf::IsConstructorCall( SharedPtr<Node> c, SharedPtr<Call> call )
{
	context = c;
	SharedPtr<Expression> e;

    if( SharedPtr<Lookup> lf = dynamic_pointer_cast<Lookup>(call->callee) )
    {
		ASSERT(lf->member);
		if( dynamic_pointer_cast<Constructor>( Get( lf->member ) ) )
			e = lf->base;
    }

    context = SharedPtr<Node>();
    return e;
}

