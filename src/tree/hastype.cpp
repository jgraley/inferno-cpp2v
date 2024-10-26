
#include "cpptree.hpp"
#include "type_data.hpp"
#include "helpers/walk.hpp"
#include "misc.hpp"
#include "hastype.hpp"

using namespace CPPTree;


#define INT 0

AugTreePtr<Node> HasType::ApplyTransformation( const TreeKit &kit, AugTreePtr<Node> node ) const
{
	auto e = AugTreePtr<CPPTree::Expression>::DynamicCast(node);
	AugTreePtr<Node> n;
	if( e ) // if the tree at root is not an expression, return nullptr
		n = Get( kit, e );
	return n;
}


AugTreePtr<CPPTree::Type> HasType::Get( const TreeKit &kit, AugTreePtr<Expression> o ) const
{
    ASSERT(o);
    
    if( auto ii = AugTreePtr<SpecificInstanceIdentifier>::DynamicCast(o) ) // object or function instance
    {        
        AugTreePtr<Node> n = HasDeclaration().ApplyTransformation(kit, ii);
        auto i = AugTreePtr<Instance>::DynamicCast(n);
        ASSERT(i);
        return GET_CHILD(i, type); 
    }
    else if( auto op = AugTreePtr<NonCommutativeOperator>::DynamicCast(o) ) // operator
    {
        // Get the types of all the operands to the operator first
        list<AugTreePtr<Type>> optypes;
        FOR_AUG_CONTAINER( op, operands, [&](AugTreePtr<Expression> o_atp)		
		{
			AugTreePtr<Type> type = Get(kit, o_atp);
            optypes.push_back( type );
		} );
        return GetOperator( kit, op, optypes );
    }
    else if( auto op = AugTreePtr<CommutativeOperator>::DynamicCast(o) ) // operator
    {
        // Get the types of all the operands to the operator first
        list<AugTreePtr<Type>> optypes;
        FOR_AUG_CONTAINER( op, operands, [&](AugTreePtr<Expression> o_atp)		
		{
			AugTreePtr<Type> type = Get(kit, o_atp);
            optypes.push_back( type );
		} );
        return GetOperator( kit, op, optypes );
    }
    else if( auto l = AugTreePtr<Literal>::DynamicCast(o) )
    {
        return GetLiteral( kit, l );
    }
    else if( auto c = AugTreePtr<Call>::DynamicCast(o) )
    {
        AugTreePtr<Type> t = Get(kit, GET_CHILD(c, callee)); // get type of the function itself
        ASSERT( AugTreePtr<Callable>::DynamicCast(t) )( "Trying to call something that is not Callable: ")(*t);
        if( auto f = AugTreePtr<Function>::DynamicCast(t) )
        	return GET_CHILD(f, return_type);
        else if( auto e = TryGetConstructedExpression( kit, c ) )
			return Get( kit, e ); // recurse with the object (presumably) that's being constructed
        else
        	return kit.utils->MakeAugTreePtr<Void>(); 
    }
    else if( auto l = AugTreePtr<Lookup>::DynamicCast(o) ) // a.b; just return type of b
    {
        return Get( kit, GET_CHILD(l, member) );
    }
    else if( auto c = AugTreePtr<Cast>::DynamicCast(o) )
    {
        return GET_CHILD(c, type);
    }
    else if( auto rl = AugTreePtr<MakeRecord>::DynamicCast(o) )
    {
        return GET_CHILD(rl, type);
    }
    else if( AugTreePtr<LabelIdentifier>::DynamicCast(o) )
    {
        return kit.utils->MakeAugTreePtr<Labeley>(); 
    }
    else if( AugTreePtr<SizeOf>::DynamicCast(o) || AugTreePtr<AlignOf>::DynamicCast(o) )
    {
    	AugTreePtr<Integral> n;
        n = kit.utils->MakeAugTreePtr<Unsigned>();
       	auto sz = kit.utils->MakeAugTreePtr<SpecificInteger>( TypeDb::size_t_bits );
    	SET_CHILD(n, width, sz);
        return n;
    }
    else if( auto n = AugTreePtr<New>::DynamicCast(o) )
    {
        auto p = kit.utils->MakeAugTreePtr<Pointer>();
        SET_CHILD(p, destination, GET_CHILD(n, type));
        return p;
    }
    else if( AugTreePtr<Delete>::DynamicCast(o) )
    {
        return kit.utils->MakeAugTreePtr<Void>(); 
    }
    else if( auto ce = AugTreePtr<StatementExpression>::DynamicCast(o) )
    {
        if( ce->statements.empty() ) // TODO trap operator -> as a dep leak
            return kit.utils->MakeAugTreePtr<Void>(); 
        AugTreePtr<Statement> last = GET_CHILD_BACK(ce, statements);
        if( auto e = AugTreePtr<Expression>::DynamicCast(last) )
            return Get(kit, e);
        else
            return kit.utils->MakeAugTreePtr<Void>(); 
    }
    else 
    {
        throw UnsupportedExpressionMismatch();
        //ASSERT(0)("Unknown expression ")(*o)(", please add to HasType class");
        //ASSERTFAIL("");
    }
}

// Just discover the type of operators, where the types of the operands have already been determined
// Note we always get a Sequence, even when the operator is commutative
AugTreePtr<CPPTree::Type> HasType::GetOperator( const TreeKit &kit, AugTreePtr<Operator> op, list<AugTreePtr<Type>> optypes ) const
{
	// Lower types that masquerade as other types in preparation for operand analysis
	// - References go to the referenced type
	// - Arrays go to pointers
	for( AugTreePtr<Type> &t : optypes )
	{
		while( auto r = AugTreePtr<Reference>::DynamicCast(t) )
			t = GET_CHILD(r, destination);
		if( auto a = AugTreePtr<Array>::DynamicCast(t) )
		{
			auto p = kit.utils->MakeAugTreePtr<Pointer>();
			SET_CHILD(p, destination, GET_CHILD(a, element));
			t = AugTreePtr<Type>(p);
		}
		// Check we finished the job
		ASSERT( !AugTreePtr<Reference>::DynamicCast(t) );
		ASSERT( !AugTreePtr<Array>::DynamicCast(t) );
	}

	// Turn an array literal into an array
    if( auto al = AugTreePtr<MakeArray>::DynamicCast(op) )
    {
    	auto a = kit.utils->MakeAugTreePtr<Array>();
    	auto sz = kit.utils->MakeAugTreePtr<SpecificInteger>( (int)(optypes.size()) ); // TODO make it work with size_t and remove the cast
    	SET_CHILD(a, size, sz);
    	if( optypes.empty() )
			SET_CHILD(a, element, kit.utils->MakeAugTreePtr<Void>()); // array has no elements so cannot determine type
		else
			SET_CHILD(a, element, optypes.front());
        return AugTreePtr<Type>(a);
    }

	// Assignment operators return their left-hand operand type in all cases
	if( AugTreePtr<AssignmentOperator>::DynamicCast(op) )
	{
		return optypes.front();
	}

	// Pointer arithmetic: a subtract involving two pointers returns int
	// we are not bothering to check that the pointer types are compatible.
	if( AugTreePtr<Subtract>::DynamicCast(op) )
	{
		if( AugTreePtr<Pointer>::DynamicCast(optypes.front()) && 
            AugTreePtr<Pointer>::DynamicCast(optypes.back()) )
		{
			auto i = kit.utils->MakeAugTreePtr<Signed>();
			auto nc = kit.utils->MakeAugTreePtr<SpecificInteger>( TypeDb::integral_bits[INT] );
			SET_CHILD(i, width, nc); // TODO need eg SetChild() to pull nc into deps
			return i;
		}
	}

	// Pointer arithmetic: an add or subtract involving a pointer returns that pointer type
	if( AugTreePtr<Add>::DynamicCast(op) || AugTreePtr<Subtract>::DynamicCast(op) )
	{
		for( AugTreePtr<Type> t : optypes )
			if( auto p = AugTreePtr<Pointer>::DynamicCast(t) )
		        return p;
	}

#define ARITHMETIC GetStandard( kit, optypes )
#define BITWISE GetStandard( kit, optypes )
#define LOGICAL kit.utils->MakeAugTreePtr<Boolean>()
#define COMPARISON kit.utils->MakeAugTreePtr<Boolean>()
#define SHIFT optypes.front()
#define SPECIAL GetSpecial( kit, op, optypes )

    if(false) {}
#define INFIX(TOK, TEXT, NODE, BASE, CAT) \
	else if( AugTreePtr<NODE>::DynamicCast(op) ) \
		return CAT;
#define PREFIX(TOK, TEXT, NODE, BASE, CAT) \
	else if( AugTreePtr<NODE>::DynamicCast(op) ) \
		return CAT;
#define POSTFIX(TOK, TEXT, NODE, BASE, CAT) \
	else if( AugTreePtr<NODE>::DynamicCast(op) ) \
		return CAT;
#define OTHER(TOK, TEXT, NODE, BASE, CAT) \
	else if( AugTreePtr<NODE>::DynamicCast(op) ) \
		return CAT;
#include "tree/operator_data.inc"
    else
    {
        throw UnsupportedOperatorMismatch();
        //ASSERT(0)("Unknown operator ")(*op)(" (not in operator_data.inc), please add to HasType class");
        //ASSERTFAIL("");
    }
}


AugTreePtr<CPPTree::Type> HasType::GetStandard( const TreeKit &kit, list<AugTreePtr<Type>> &optypes ) const
{
	list<AugTreePtr<Numeric>> nums;
	for( AugTreePtr<Type> optype : optypes )
		if( auto n = AugTreePtr<Numeric>::DynamicCast(optype) )
			nums.push_back(n);
	if( nums.size() == optypes.size() )
		return GetStandardOnNumerics( kit, nums );

    throw NumericalOperatorUsageMismatch1();
//	if( optypes.size() == 2 )
//		ASSERT(0)("Standard operator unknown usage, please add to HasType class");
//	else
//		ASSERT(0)("Standard operator unknown usage, please add to HasType class");
//    ASSERTFAIL();
}


AugTreePtr<CPPTree::Type> HasType::GetStandardOnNumerics( const TreeKit &kit, list<AugTreePtr<Numeric>> &optypes ) const
{
	// Start the width and signedness as per regular "int" since this is the
	// minimum result type for standard operators
    int64_t maxwidth_signed = TypeDb::integral_bits[INT];
	int64_t maxwidth_unsigned = 0;
	AugTreePtr<SpecificFloatSemantics> maxwidth_float;

	// Look at the operands in turn
	for( AugTreePtr<Type> optype : optypes ) // TODO why drop Numeric?
	{
		// Floats take priority
		if( auto f = AugTreePtr<Floating>::DynamicCast(optype) )
		{
			auto sfs = AugTreePtr<SpecificFloatSemantics>::DynamicCast(GET_CHILD(f, semantics));
			ASSERT(sfs)("Floating point type seen with semantics not specific");
			unsigned int sl = llvm::APFloat::semanticsPrecision( *sfs );
			unsigned int sr = llvm::APFloat::semanticsPrecision( *maxwidth_float );
			if( !maxwidth_float || sl > sr )
				maxwidth_float = sfs;
		}

		// Should only have Integrals from here on
		auto intop = AugTreePtr<Integral>::DynamicCast(optype);
        if( !intop )
            throw NumericalOperatorUsageMismatch2();
        //ASSERT( intop )(*optype)(" is not Floating or Integral, please add to HasType class" );

        // Do a max algorithm on the width
		auto siwidth = AugTreePtr<SpecificInteger>::DynamicCast(GET_CHILD(intop, width));
        if( !siwidth )
            throw NumericalOperatorUsageMismatch3();
		//ASSERT( siwidth )( "Integral size ")(*(intop->width))(" is not specific, cannot decide result type");
		int64_t width = siwidth->GetInt64(); // here we assume int64_t can hold the widths of integer variablkes

		if( AugTreePtr<Signed>::DynamicCast(optype) )
		{
            maxwidth_signed = max( width, maxwidth_signed );
		}
		else if( AugTreePtr<Unsigned>::DynamicCast(optype) )
		{
            maxwidth_unsigned = max( width, maxwidth_unsigned );
		}
		else
        {
            throw NumericalOperatorUsageMismatch4();
			//ASSERT( 0 )(*intop)(" is not Signed or Unsigned, please add to HasType class");
        }
	}

	if( maxwidth_float )
	{
		auto result = kit.utils->MakeAugTreePtr<Floating>();
		SET_CHILD(result, semantics, kit.utils->MakeAugTreePtr<SpecificFloatSemantics>( *maxwidth_float ));
		return AugTreePtr<Type>(result);
	}

	// Build the required integral result type
	AugTreePtr<Integral> result;
	if( maxwidth_unsigned && maxwidth_unsigned >= maxwidth_signed )
	{
        // Use the unsigned size if unsigned operand exists and at least as big as int size
		result = kit.utils->MakeAugTreePtr<Unsigned>();
		SET_CHILD(result, width, kit.utils->MakeAugTreePtr<SpecificInteger>(maxwidth_unsigned));
	}
	else
	{
		result = kit.utils->MakeAugTreePtr<Signed>();
		SET_CHILD( result, width, kit.utils->MakeAugTreePtr<SpecificInteger>(maxwidth_signed));
	}
	return result;
}


AugTreePtr<CPPTree::Type> HasType::GetSpecial( const TreeKit &kit, AugTreePtr<Operator> op, list<AugTreePtr<Type>> &optypes ) const
{
    if( AugTreePtr<Dereference>::DynamicCast(op) || AugTreePtr<Subscript>::DynamicCast(op) )
    {
        if( auto o2 = AugTreePtr<Pointer>::DynamicCast( optypes.front() ) )
            return GET_CHILD(o2, destination);
        else if( auto o2 = AugTreePtr<Array>::DynamicCast( optypes.front() ) )
            return GET_CHILD(o2, element);
        else
            throw DereferenceUsageMismatch();
            //ASSERTFAIL( "dereferencing non-pointer" );
    }
    else if( AugTreePtr<AddressOf>::DynamicCast(op) )
    {
        auto p = kit.utils->MakeAugTreePtr<Pointer>();
        SET_CHILD(p, destination, optypes.front());
        return AugTreePtr<Type>(p);
    }
    else if( AugTreePtr<Comma>::DynamicCast(op) )
    {
        return optypes.back();
    }
    else if( AugTreePtr<ConditionalOperator>::DynamicCast(op) )
	{
        list<AugTreePtr<Type>>::iterator optypes_it = optypes.begin();
        ++optypes_it;
		return *optypes_it; // middle element TODO do this properly, consider cond ? nullptr : &x
	}
    else if( AugTreePtr<This>::DynamicCast(op) )
    {
    	throw UnsupportedThisMismatch(); // TODO add support
        //ASSERTFAIL(""); 
    }
    else
    {
        throw UnsupportedSpecialMismatch();
        //ASSERT(0)("Unknown SPECIAL operator ")(*op)(", please add to HasType class");
        //ASSERTFAIL("");
    }
}


AugTreePtr<CPPTree::Type> HasType::GetLiteral( const TreeKit &kit, AugTreePtr<Literal> l ) const
{
    if( auto si = AugTreePtr<SpecificInteger>::DynamicCast(l) )
    {
    	// Get the info from Clang, and make an Inferno type for it
    	AugTreePtr<Integral> it;
        if( si->IsSigned() )
        	it = kit.utils->MakeAugTreePtr<Signed>();
        else
        	it = kit.utils->MakeAugTreePtr<Unsigned>();
        SET_CHILD(it, width, kit.utils->MakeAugTreePtr<SpecificInteger>( si->GetWidth() ));
        return it;
    }
    else if( auto sf = AugTreePtr<SpecificFloat>::DynamicCast(l) )
    {
    	// Get the info from Clang, and make an Inferno type for it
    	auto ft = kit.utils->MakeAugTreePtr<Floating>();
    	SET_CHILD(ft, semantics, kit.utils->MakeAugTreePtr<SpecificFloatSemantics>( &sf->getSemantics() ));
        return ft;
    }
    else if( AugTreePtr<Bool>::DynamicCast(l) )
    {
        return kit.utils->MakeAugTreePtr<Boolean>();
    }
    else if( AugTreePtr<String>::DynamicCast(l) )
    {
    	AugTreePtr<Integral> n;
    	if( TypeDb::char_default_signed )
    		n = kit.utils->MakeAugTreePtr<Signed>();
    	else
    		n = kit.utils->MakeAugTreePtr<Unsigned>();
    	auto sz = kit.utils->MakeAugTreePtr<SpecificInteger>( TypeDb::char_bits );
    	SET_CHILD(n, width, sz);
    	auto p = kit.utils->MakeAugTreePtr<Pointer>();
    	SET_CHILD(p, destination, n);
        return p;
    }
    else
    {
        throw UnsupportedLiteralMismatch();
        //ASSERT(0)("Unknown literal ")(*l)(", please add to HasType class");
        //ASSERTFAIL("");
    }
}


// Is this call really a constructor call? If so return the object being
// constructed. Otherwise, return nullptr
AugTreePtr<CPPTree::Expression> HasType::TryGetConstructedExpression( const TreeKit &kit, AugTreePtr<Call> call ) const
{
	AugTreePtr<CPPTree::Expression> e;

    if( auto lf = AugTreePtr<Lookup>::DynamicCast(GET_CHILD(call, callee)) )
    {
		ASSERT(lf->member);
		if( AugTreePtr<Constructor>::DynamicCast( Get( kit, GET_CHILD(lf, member) ) ) )
			e = GET_CHILD(lf, base);
    }

    return e;
}

HasType HasType::instance; 

