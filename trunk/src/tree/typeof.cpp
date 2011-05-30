
#include "tree.hpp"
#include "type_db.hpp"
#include "node/walk.hpp"
#include "misc.hpp"
#include "typeof.hpp"




#define INT 0

TreePtr<Type> TypeOf::Get( TreePtr<Expression> o )
{
    ASSERT(o);
    
    if( TreePtr<SpecificInstanceIdentifier> ii = dynamic_pointer_cast<SpecificInstanceIdentifier>(o) ) // object or function instance
    {        
        TreePtr<Node> n = GetDeclaration()(context, ii);
        TreePtr<Instance> i = dynamic_pointer_cast<Instance>(n);
        ASSERT(i);
        return i->type; 
    }
    else if( TreePtr<NonCommutativeOperator> op = dynamic_pointer_cast<NonCommutativeOperator>(o) ) // operator
    {
        // Get the types of all the operands to the operator first
        Sequence<Type> optypes;
        FOREACH( TreePtr<Expression> o, op->operands )
            optypes.push_back( Get(o) );
        return Get( op, optypes );
    }
    else if( TreePtr<CommutativeOperator> op = dynamic_pointer_cast<CommutativeOperator>(o) ) // operator
    {
        // Get the types of all the operands to the operator first
        Sequence<Type> optypes;
        FOREACH( TreePtr<Expression> o, op->operands )
                 optypes.push_back( Get(o) );
        return Get( op, optypes );
    }
    else if( TreePtr<Literal> l = dynamic_pointer_cast<Literal>(o) )
    {
        return GetLiteral( l );
    }
    else if( TreePtr<Call> c = dynamic_pointer_cast<Call>(o) )
    {
        TreePtr<Type> t = Get(c->callee); // get type of the function itself
        ASSERT( dynamic_pointer_cast<Subroutine>(t) )( "Trying to call something that is not a kind of Subroutine");
        if( TreePtr<Function> f = dynamic_pointer_cast<Function>(t) )
        	return f->return_type;
        else
        	return MakeTreePtr<Void>();
    }
    else if( TreePtr<Lookup> l = dynamic_pointer_cast<Lookup>(o) ) // a.b; just return type of b
    {
        return Get( l->member );
    }
    else if( TreePtr<Cast> c = dynamic_pointer_cast<Cast>(o) )
    {
        return c->type;
    }
    else if( TreePtr<MakeRecord> rl = dynamic_pointer_cast<MakeRecord>(o) )
    {
        return rl->type;
    }
    else if( dynamic_pointer_cast<LabelIdentifier>(o) )
    {
        return MakeTreePtr<Type>(); // TODO labels need a type
    }
    else if( dynamic_pointer_cast<SizeOf>(o) || dynamic_pointer_cast<AlignOf>(o))
    {
    	TreePtr<Integral> n;
    	if( TypeDb::int_default_signed )
    		n = MakeTreePtr<Signed>();
    	else
    		n = MakeTreePtr<Unsigned>();
    	TreePtr<SpecificInteger> sz( new SpecificInteger(TypeDb::integral_bits[INT]) );
    	n->width = sz;
       return n;
    }
    else if( TreePtr<New> n = dynamic_pointer_cast<New>(o) )
    {
        TreePtr<Pointer> p( new Pointer );
        p->destination = n->type;
        return p;
    }
    else if( dynamic_pointer_cast<Delete>(o) )
    {
        return MakeTreePtr<Type>();
    }
    else 
    {
        ASSERT(0)("Unknown expression ")(*o)(", please add to TypeOf class");
        ASSERTFAIL("");
    }
}

// Just discover the type of operators, where the types of the operands have already been determined
// Note we always get a Sequence, even when the operator is commutative
TreePtr<Type> TypeOf::Get( TreePtr<Operator> op, Sequence<Type> optypes )
{
	// Lower types that masquerade as other types in preparation for operand analysis
	// - References go to the referenced type
	// - Arrays go to pointers
	FOREACH( TreePtr<Type> &t, optypes )
	{
		while( TreePtr<Reference> r = dynamic_pointer_cast<Reference>(t) )
			t = r->destination;
		if( TreePtr<Array> a = dynamic_pointer_cast<Array>(t) )
		{
			TreePtr<Pointer> p( new Pointer );
			p->destination = a->element;
			t = p;
		}
		// Check we finished the job
		ASSERT( !dynamic_pointer_cast<Reference>(t) );
		ASSERT( !dynamic_pointer_cast<Array>(t) );
	}

	// Turn an array literal into an array
    if( TreePtr<MakeArray> al = dynamic_pointer_cast<MakeArray>(op) )
    {
    	TreePtr<Array> a( new Array );
    	a->element = optypes[0];
    	TreePtr<SpecificInteger> sz( new SpecificInteger(optypes.size()) );
    	a->size = sz;
        return a;
    }

	// Assignment operators return their left-hand operand type in all cases
	if( dynamic_pointer_cast<AssignmentOperator>(op) )
	{
		return optypes[0];
	}

	// Pointer arithmetic: a subtract involving two pointers returns int
	// we are not bothering to check that the pointer types are compatible.
	if( dynamic_pointer_cast<Subtract>(op) )
	{
		if( dynamic_pointer_cast<Pointer>(optypes[0]) && dynamic_pointer_cast<Pointer>(optypes[1]) )
		{
			TreePtr<Signed> i = TreePtr<Signed>( new Signed );
			TreePtr<SpecificInteger> nc( new SpecificInteger(TypeDb::integral_bits[INT]) );
			i->width = nc;
			return i;
		}
	}

	// Pointer arithmetic: an add or subtract involving a pointer returns that pointer type
	if( dynamic_pointer_cast<Add>(op) || dynamic_pointer_cast<Subtract>(op) )
	{
		FOREACH( TreePtr<Type> t, optypes )
			if( TreePtr<Pointer> p = dynamic_pointer_cast<Pointer>(t) )
		        return p;
	}

#define ARITHMETIC GetStandard( optypes )
#define BITWISE GetStandard( optypes )
#define LOGICAL MakeTreePtr<Boolean>()
#define COMPARISON MakeTreePtr<Boolean>()
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


TreePtr<Type> TypeOf::GetStandard( Sequence<Type> &optypes )
{
	Sequence<Numeric> nums;
	for( int i=0; i<optypes.size(); i++ )
		if( TreePtr<Numeric> n = dynamic_pointer_cast<Numeric>(optypes[i]) )
			nums.push_back(n);
	if( nums.size() == optypes.size() )
		return GetStandard( nums );

	if( optypes.size() == 2 )
		ASSERT(0)("Standard operator with operands ")(*optypes[0])(*optypes[1])(" is unknown usage, please add to TypeOf class");
	else
		ASSERT(0)("Standard operator with ")(*optypes[0])(" is unknown usage, please add to TypeOf class");
    ASSERTFAIL();
}


TreePtr<Type> TypeOf::GetStandard( Sequence<Numeric> &optypes )
{
	// Start the width and signedness as per regular "int" since this is the
	// minimum result type for standard operators
	TreePtr<SpecificInteger> maxwidth_signed( new SpecificInteger(TypeDb::integral_bits[INT]) );
	TreePtr<SpecificInteger> maxwidth_unsigned;
	TreePtr<SpecificFloatSemantics> maxwidth_float;

	// Look at the operands in turn
	for( int i=0; i<optypes.size(); i++ )
	{
		// Floats take priority
		if( TreePtr<Floating> f = dynamic_pointer_cast<Floating>(optypes[i]) )
		{
			TreePtr<SpecificFloatSemantics> sfs = dynamic_pointer_cast<SpecificFloatSemantics>(f->semantics);
			ASSERT(sfs)("Floating point type seen with semantics not specific");
			unsigned int sl = llvm::APFloat::semanticsPrecision( *sfs );
			unsigned int sr = llvm::APFloat::semanticsPrecision( *maxwidth_float );
			if( !maxwidth_float || sl > sr )
				maxwidth_float = sfs;
		}

		// Should only have Integrals from here on
		TreePtr<Integral> intop = dynamic_pointer_cast<Integral>(optypes[i]);
        ASSERT( intop )(*optypes[i])(" is not Floating or Integral, please add to TypeOf class" );

        // Do a max algorithm on the width
		TreePtr<SpecificInteger> width = dynamic_pointer_cast<SpecificInteger>(intop->width);
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
		TreePtr<Floating> result;
		result->semantics = TreePtr<SpecificFloatSemantics>( new SpecificFloatSemantics(*maxwidth_float) );
		return result;
	}

	// Build the required integral result type
	TreePtr<Integral> result;
	if( maxwidth_unsigned && *maxwidth_unsigned >= *maxwidth_signed )
	{
		result = TreePtr<Integral>( new Unsigned );
		result->width = maxwidth_unsigned;
	}
	else
	{
		result = TreePtr<Integral>( new Signed );
		result->width = maxwidth_signed;
	}
	return result;
}


TreePtr<Type> TypeOf::GetSpecial( TreePtr<Operator> op, Sequence<Type> &optypes )
{
    if( dynamic_pointer_cast<Dereference>(op) || dynamic_pointer_cast<Subscript>(op) )
    {
        if( TreePtr<Pointer> o2 = dynamic_pointer_cast<Pointer>( optypes[0] ) )
            return o2->destination;
        else if( TreePtr<Array> o2 = dynamic_pointer_cast<Array>( optypes[0] ) )
            return o2->element;
        else
            ASSERTFAIL( "dereferencing non-pointer" );
    }
    else if( dynamic_pointer_cast<AddressOf>(op) )
    {
        TreePtr<Pointer> p( new Pointer );
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

TreePtr<Type> TypeOf::GetLiteral( TreePtr<Literal> l )
{
    if( TreePtr<SpecificInteger> si = dynamic_pointer_cast<SpecificInteger>(l) )
    {
    	// Get the info from Clang, and make an Inferno type for it
    	TreePtr<Integral> it;
        if( si->isSigned() )
        	it = MakeTreePtr<Signed>();
        else
        	it = MakeTreePtr<Unsigned>();
        it->width = TreePtr<SpecificInteger>( new SpecificInteger( si->getBitWidth() ) );
        return it;
    }
    else if( TreePtr<SpecificFloat> sf = dynamic_pointer_cast<SpecificFloat>(l) )
    {
    	// Get the info from Clang, and make an Inferno type for it
    	MakeTreePtr<Floating> ft;
    	ft->semantics = TreePtr<SpecificFloatSemantics>( new SpecificFloatSemantics(&sf->getSemantics()) );
        return ft;
    }
    else if( dynamic_pointer_cast<Bool>(l) )
    {
        return MakeTreePtr<Boolean>();
    }
    else if( dynamic_pointer_cast<String>(l) )
    {
    	TreePtr<Integral> n;
    	if( TypeDb::char_default_signed )
    		n = MakeTreePtr<Signed>();
    	else
    		n = MakeTreePtr<Unsigned>();
    	TreePtr<SpecificInteger> sz( new SpecificInteger(TypeDb::char_bits) );
    	n->width = sz;
    	MakeTreePtr<Pointer> p;
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
TreePtr<Expression> TypeOf::IsConstructorCall( TreePtr<Node> c, TreePtr<Call> call )
{
	context = c;
	TreePtr<Expression> e;

    if( TreePtr<Lookup> lf = dynamic_pointer_cast<Lookup>(call->callee) )
    {
		ASSERT(lf->member);
		if( dynamic_pointer_cast<Constructor>( Get( lf->member ) ) )
			e = lf->base;
    }

    context = TreePtr<Node>();
    return e;
}

TypeOf TypeOf::instance; // TODO Use this instead of constructing a temp (could contain lookup tables etc in the future)
