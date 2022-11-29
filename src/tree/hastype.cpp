
#include "cpptree.hpp"
#include "type_data.hpp"
#include "helpers/walk.hpp"
#include "misc.hpp"
#include "hastype.hpp"

using namespace CPPTree;


#define INT 0

AugTreePtr<Node> HasType::operator()( const TreeKit &kit_, TreePtr<Node> node )
{
	kit = &kit_;
	auto e = TreePtr<CPPTree::Expression>::DynamicCast(node);
	AugTreePtr<Node> n;
	if( e ) // if the tree at root is not an expression, return nullptr
		n = Get( e );
	kit = nullptr;
	return n;
}


AugTreePtr<CPPTree::Type> HasType::Get( TreePtr<Expression> o )
{
    ASSERT(o);
    
    if( auto ii = DynamicTreePtrCast<SpecificInstanceIdentifier>(o) ) // object or function instance
    {        
        AugTreePtr<Node> n = HasDeclaration()(*kit, ii);
        TreePtr<Instance> i = DynamicTreePtrCast<Instance>(n);
        ASSERT(i);
        return n.Descend<Type>(&i->type); 
    }
    else if( auto op = DynamicTreePtrCast<NonCommutativeOperator>(o) ) // operator
    {
        // Get the types of all the operands to the operator first
        list<AugTreePtr<Type>> optypes;
        for( TreePtr<Expression> o : op->operands )
            optypes.push_back( Get(o) );
        return Get( op, optypes );
    }
    else if( auto op = DynamicTreePtrCast<CommutativeOperator>(o) ) // operator
    {
        // Get the types of all the operands to the operator first
        list<AugTreePtr<Type>> optypes;
        for( TreePtr<Expression> o : op->operands )
                 optypes.push_back( Get(o) );
        return Get( op, optypes );
    }
    else if( auto l = DynamicTreePtrCast<Literal>(o) )
    {
        return GetLiteral( l );
    }
    else if( auto c = DynamicTreePtrCast<Call>(o) )
    {
        AugTreePtr<Type> t = Get(c->callee); // get type of the function itself
        ASSERT( dynamic_pointer_cast<Callable>(t) )( "Trying to call something that is not Callable");
        if( auto f = DynamicTreePtrCast<Function>(t) )
        	return t.Descend<Type>(&f->return_type);
        else
        	return AugTreePtr<Type>(MakeTreeNode<Void>());
    }
    else if( auto l = DynamicTreePtrCast<Lookup>(o) ) // a.b; just return type of b
    {
        return Get( l->member );
    }
    else if( auto c = DynamicTreePtrCast<Cast>(o) )
    {
        return AugTreePtr<Type>(&c->type);
    }
    else if( auto rl = DynamicTreePtrCast<MakeRecord>(o) )
    {
        return AugTreePtr<Type>(&rl->type);
    }
    else if( DynamicTreePtrCast<LabelIdentifier>(o) )
    {
        return AugTreePtr<Type>(MakeTreeNode<Labeley>()); 
    }
    else if( DynamicTreePtrCast<SizeOf>(o) || DynamicTreePtrCast<AlignOf>(o) )
    {
    	TreePtr<Integral> n;
        n = MakeTreeNode<Unsigned>();
       	auto sz = MakeTreeNode<SpecificInteger>( TypeDb::size_t_bits );
    	n->width = sz;
        return AugTreePtr<Type>(n);
    }
    else if( auto n = DynamicTreePtrCast<New>(o) )
    {
        auto p = MakeTreeNode<Pointer>();
        p->destination = n->type;
        return AugTreePtr<Type>(p);
    }
    else if( DynamicTreePtrCast<Delete>(o) )
    {
        return AugTreePtr<Type>(MakeTreeNode<Void>()); 
    }
    else if( auto ce = DynamicTreePtrCast<StatementExpression>(o) )
    {
        if( ce->statements.empty() )
            return AugTreePtr<Type>(MakeTreeNode<Void>()); 
        TreePtr<Statement> last = ce->statements.back();
        if( TreePtr<Expression> e = DynamicTreePtrCast<Expression>(last) )
            return Get(e);
        else
            return AugTreePtr<Type>(MakeTreeNode<Void>()); 
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
AugTreePtr<CPPTree::Type> HasType::Get( TreePtr<Operator> op, list<AugTreePtr<Type>> optypes )
{
	// Lower types that masquerade as other types in preparation for operand analysis
	// - References go to the referenced type
	// - Arrays go to pointers
	for( AugTreePtr<Type> &t : optypes )
	{
		while( auto r = DynamicTreePtrCast<Reference>(t) )
			t = t.Descend<Type>(&r->destination);
		if( auto a = DynamicTreePtrCast<Array>(t) )
		{
			auto p = MakeTreeNode<Pointer>();
			p->destination = a->element;
			t = AugTreePtr<Type>(p);
		}
		// Check we finished the job
		ASSERT( !DynamicTreePtrCast<Reference>(t) );
		ASSERT( !DynamicTreePtrCast<Array>(t) );
	}

	// Turn an array literal into an array
    if( auto al = DynamicTreePtrCast<MakeArray>(op) )
    {
    	auto a = MakeTreeNode<Array>();
    	auto sz = MakeTreeNode<SpecificInteger>( (int)(optypes.size()) ); // TODO make it work with size_t and remove the cast
    	a->size = sz;
    	if( optypes.empty() )
			a->element = AugTreePtr<Type>(MakeTreeNode<Void>()); // array has no elements so cannot determine type
		else
			a->element = optypes.front();
        return AugTreePtr<Type>(a);
    }

	// Assignment operators return their left-hand operand type in all cases
	if( DynamicTreePtrCast<AssignmentOperator>(op) )
	{
		return optypes.front();
	}

	// Pointer arithmetic: a subtract involving two pointers returns int
	// we are not bothering to check that the pointer types are compatible.
	if( dynamic_pointer_cast<Subtract>(op) )
	{
		if( DynamicTreePtrCast<Pointer>(optypes.front()) && 
            DynamicTreePtrCast<Pointer>(optypes.back()) )
		{
			auto i = MakeTreeNode<Signed>();
			auto nc = MakeTreeNode<SpecificInteger>( TypeDb::integral_bits[INT] );
			i->width = nc;
			return AugTreePtr<Type>(i);
		}
	}

	// Pointer arithmetic: an add or subtract involving a pointer returns that pointer type
	if( DynamicTreePtrCast<Add>(op) || DynamicTreePtrCast<Subtract>(op) )
	{
		for( AugTreePtr<Type> t : optypes )
			if( auto p = AugTreePtr<Pointer>::DynamicCast(t) )
		        return p;
	}

#define ARITHMETIC GetStandard( optypes )
#define BITWISE GetStandard( optypes )
#define LOGICAL AugTreePtr<Boolean>(MakeTreeNode<Boolean>())
#define COMPARISON AugTreePtr<Boolean>(MakeTreeNode<Boolean>())
#define SHIFT optypes.front()
#define SPECIAL GetSpecial( op, optypes )

    if(0) {}
#define INFIX(TOK, TEXT, NODE, BASE, CAT) \
	else if( DynamicTreePtrCast<NODE>(op) ) \
		return CAT;
#define PREFIX(TOK, TEXT, NODE, BASE, CAT) \
	else if( DynamicTreePtrCast<NODE>(op) ) \
		return CAT;
#define POSTFIX(TOK, TEXT, NODE, BASE, CAT) \
	else if( DynamicTreePtrCast<NODE>(op) ) \
		return CAT;
#define OTHER(TOK, TEXT, NODE, BASE, CAT) \
	else if( DynamicTreePtrCast<NODE>(op) ) \
		return CAT;
#include "tree/operator_data.inc"
    else
    {
        throw UnsupportedOperatorMismatch();
        //ASSERT(0)("Unknown operator ")(*op)(" (not in operator_data.inc), please add to HasType class");
        //ASSERTFAIL("");
    }
}


AugTreePtr<CPPTree::Type> HasType::GetStandard( list<AugTreePtr<Type>> &optypes )
{
	list<AugTreePtr<Numeric>> nums;
	for( AugTreePtr<Type> optype : optypes )
		if( auto n = AugTreePtr<Numeric>::DynamicCast(optype) )
			nums.push_back(n);
	if( nums.size() == optypes.size() )
		return GetStandard( nums );

    throw NumericalOperatorUsageMismatch1();
//	if( optypes.size() == 2 )
//		ASSERT(0)("Standard operator unknown usage, please add to HasType class");
//	else
//		ASSERT(0)("Standard operator unknown usage, please add to HasType class");
//    ASSERTFAIL();
}


AugTreePtr<CPPTree::Type> HasType::GetStandard( list<AugTreePtr<Numeric>> &optypes )
{
	// Start the width and signedness as per regular "int" since this is the
	// minimum result type for standard operators
    int64_t maxwidth_signed = TypeDb::integral_bits[INT];
	int64_t maxwidth_unsigned = 0;
	TreePtr<SpecificFloatSemantics> maxwidth_float;

	// Look at the operands in turn
	for( AugTreePtr<Type> optype : optypes ) // TODO why drop Numeric?
	{
		// Floats take priority
		if( auto f = DynamicTreePtrCast<Floating>(optype) )
		{
			TreePtr<SpecificFloatSemantics> sfs = DynamicTreePtrCast<SpecificFloatSemantics>(f->semantics);
			ASSERT(sfs)("Floating point type seen with semantics not specific");
			unsigned int sl = llvm::APFloat::semanticsPrecision( *sfs );
			unsigned int sr = llvm::APFloat::semanticsPrecision( *maxwidth_float );
			if( !maxwidth_float || sl > sr )
				maxwidth_float = sfs;
		}

		// Should only have Integrals from here on
		TreePtr<Integral> intop = DynamicTreePtrCast<Integral>(optype);
        if( !intop )
            throw NumericalOperatorUsageMismatch2();
        //ASSERT( intop )(*optype)(" is not Floating or Integral, please add to HasType class" );

        // Do a max algorithm on the width
		auto siwidth = DynamicTreePtrCast<SpecificInteger>(intop->width);
        if( !siwidth )
            throw NumericalOperatorUsageMismatch3();
		//ASSERT( siwidth )( "Integral size ")(*(intop->width))(" is not specific, cannot decide result type");
		int64_t width = siwidth->GetInt64(); // here we assume int64_t can hold the widths of integer variablkes

		if( DynamicTreePtrCast<Signed>(optype) )
		{
            maxwidth_signed = max( width, maxwidth_signed );
		}
		else if( DynamicTreePtrCast<Unsigned>(optype) )
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
		TreePtr<Floating> result;
		result->semantics = MakeTreeNode<SpecificFloatSemantics>( *maxwidth_float );
		return AugTreePtr<Type>(result);
	}

	// Build the required integral result type
	TreePtr<Integral> result;
	if( maxwidth_unsigned && maxwidth_unsigned >= maxwidth_signed )
	{
        // Use the unsigned size if unsigned operand exists and at least as big as int size
		result = MakeTreeNode<Unsigned>();
		result->width = MakeTreeNode<SpecificInteger>(maxwidth_unsigned);
	}
	else
	{
		result = MakeTreeNode<Signed>();
		result->width = MakeTreeNode<SpecificInteger>(maxwidth_signed);
	}
	return AugTreePtr<Type>(result);
}


AugTreePtr<CPPTree::Type> HasType::GetSpecial( TreePtr<Operator> op, list<AugTreePtr<Type>> &optypes )
{
    if( dynamic_pointer_cast<Dereference>(op) || dynamic_pointer_cast<Subscript>(op) )
    {
        if( TreePtr<Pointer> o2 = DynamicTreePtrCast<Pointer>( optypes.front() ) )
            return optypes.front().Descend<Type>(&o2->destination);
        else if( TreePtr<Array> o2 = DynamicTreePtrCast<Array>( optypes.front() ) )
            return optypes.front().Descend<Type>(&o2->element);
        else
            throw DereferenceUsageMismatch();
            //ASSERTFAIL( "dereferencing non-pointer" );
    }
    else if( DynamicTreePtrCast<AddressOf>(op) )
    {
        auto p = MakeTreeNode<Pointer>();
        p->destination = optypes.front();
        return AugTreePtr<Type>(p);
    }
    else if( DynamicTreePtrCast<Comma>(op) )
    {
        return optypes.back();
    }
    else if( DynamicTreePtrCast<ConditionalOperator>(op) )
	{
        list<AugTreePtr<Type>>::iterator optypes_it = optypes.begin();
        ++optypes_it;
		return *optypes_it; // middle element TODO do this properly, consider cond ? nullptr : &x
	}
    else if( DynamicTreePtrCast<This>(op) )
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

AugTreePtr<CPPTree::Type> HasType::GetLiteral( TreePtr<Literal> l )
{
    if( auto si = DynamicTreePtrCast<SpecificInteger>(l) )
    {
    	// Get the info from Clang, and make an Inferno type for it
    	TreePtr<Integral> it;
        if( si->IsSigned() )
        	it = MakeTreeNode<Signed>();
        else
        	it = MakeTreeNode<Unsigned>();
        it->width = MakeTreeNode<SpecificInteger>( si->GetWidth() );
        return AugTreePtr<Type>(it);
    }
    else if( auto sf = DynamicTreePtrCast<SpecificFloat>(l) )
    {
    	// Get the info from Clang, and make an Inferno type for it
    	auto ft = MakeTreeNode<Floating>();
    	ft->semantics = MakeTreeNode<SpecificFloatSemantics>( &sf->getSemantics() );
        return AugTreePtr<Type>(ft);
    }
    else if( DynamicTreePtrCast<Bool>(l) )
    {
        return AugTreePtr<Type>(MakeTreeNode<Boolean>());
    }
    else if( DynamicTreePtrCast<String>(l) )
    {
    	TreePtr<Integral> n;
    	if( TypeDb::char_default_signed )
    		n = MakeTreeNode<Signed>();
    	else
    		n = MakeTreeNode<Unsigned>();
    	auto sz = MakeTreeNode<SpecificInteger>( TypeDb::char_bits );
    	n->width = sz;
    	auto p = MakeTreeNode<Pointer>();
    	p->destination = n;
        return AugTreePtr<Type>(p);
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
AugTreePtr<CPPTree::Expression> HasType::IsConstructorCall( const TreeKit &kit_, TreePtr<Call> call )
{
	kit = &kit_;
	AugTreePtr<CPPTree::Expression> e;

    if( auto lf = DynamicTreePtrCast<Lookup>(call->callee) )
    {
		ASSERT(lf->member);
		if( DynamicTreePtrCast<Constructor>( Get( lf->member ) ) )
			e = AugTreePtr<Expression>(&lf->base);
    }

    kit = nullptr;
    return e;
}

HasType HasType::instance; 

