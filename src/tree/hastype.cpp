
#include "cpptree.hpp"
#include "type_data.hpp"
#include "helpers/walk.hpp"
#include "misc.hpp"
#include "hastype.hpp"

using namespace CPPTree;


#define INT 0

Transformation::AugTreePtr<Node> HasType::operator()( const TreeKit &kit_, TreePtr<Node> node )
{
	kit = &kit_;
	auto e = TreePtr<CPPTree::Expression>::DynamicCast(node);
	Transformation::AugTreePtr<Node> n;
	if( e ) // if the tree at root is not an expression, return nullptr
		n = Get( e );
	kit = nullptr;
	return n;
}


Transformation::AugTreePtr<CPPTree::Type> HasType::Get( TreePtr<Expression> o )
{
    ASSERT(o);
    
    if( auto ii = DynamicTreePtrCast<SpecificInstanceIdentifier>(o) ) // object or function instance
    {        
        AugTreePtr<Node> n = HasDeclaration()(*kit, ii);
        TreePtr<Instance> i = DynamicTreePtrCast<Instance>(GET_NODE(n));
        ASSERT(i);
        return PARENT_AND_CHILD(i, type); 
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
        TreePtr<Type> t = GET_NODE(Get(c->callee)); // get type of the function itself
        ASSERT( dynamic_pointer_cast<Callable>(t) )( "Trying to call something that is not Callable");
        if( auto f = DynamicTreePtrCast<Function>(t) )
        	return PARENT_AND_CHILD(f, return_type);
        else
        	return NODE_ONLY(MakeTreeNode<Void>());
    }
    else if( auto l = DynamicTreePtrCast<Lookup>(o) ) // a.b; just return type of b
    {
        return Get( l->member );
    }
    else if( auto c = DynamicTreePtrCast<Cast>(o) )
    {
        return PARENT_AND_CHILD(c, type);
    }
    else if( auto rl = DynamicTreePtrCast<MakeRecord>(o) )
    {
        return PARENT_AND_CHILD(rl, type);
    }
    else if( DynamicTreePtrCast<LabelIdentifier>(o) )
    {
        return NODE_ONLY(MakeTreeNode<Labeley>()); 
    }
    else if( DynamicTreePtrCast<SizeOf>(o) || DynamicTreePtrCast<AlignOf>(o) )
    {
    	TreePtr<Integral> n;
        n = MakeTreeNode<Unsigned>();
       	auto sz = MakeTreeNode<SpecificInteger>( TypeDb::size_t_bits );
    	n->width = sz;
        return NODE_ONLY(n);
    }
    else if( auto n = DynamicTreePtrCast<New>(o) )
    {
        auto p = MakeTreeNode<Pointer>();
        p->destination = n->type;
        return NODE_ONLY(p);
    }
    else if( DynamicTreePtrCast<Delete>(o) )
    {
        return NODE_ONLY(MakeTreeNode<Void>()); 
    }
    else if( auto ce = DynamicTreePtrCast<StatementExpression>(o) )
    {
        if( ce->statements.empty() )
            return NODE_ONLY(MakeTreeNode<Void>()); 
        TreePtr<Statement> last = ce->statements.back();
        if( TreePtr<Expression> e = DynamicTreePtrCast<Expression>(last) )
            return Get(e);
        else
            return NODE_ONLY(MakeTreeNode<Void>()); 
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
Transformation::AugTreePtr<CPPTree::Type> HasType::Get( TreePtr<Operator> op, list<AugTreePtr<Type>> optypes )
{
	// Lower types that masquerade as other types in preparation for operand analysis
	// - References go to the referenced type
	// - Arrays go to pointers
	for( AugTreePtr<Type> &t : optypes )
	{
		while( auto r = DynamicTreePtrCast<Reference>(GET_NODE(t)) )
			t = PARENT_AND_CHILD(r, destination);
		if( auto a = DynamicTreePtrCast<Array>(GET_NODE(t)) )
		{
			auto p = MakeTreeNode<Pointer>();
			p->destination = a->element;
			t = NODE_ONLY(p);
		}
		// Check we finished the job
		ASSERT( !DynamicTreePtrCast<Reference>(GET_NODE(t)) );
		ASSERT( !DynamicTreePtrCast<Array>(GET_NODE(t)) );
	}

	// Turn an array literal into an array
    if( auto al = DynamicTreePtrCast<MakeArray>(op) )
    {
    	auto a = MakeTreeNode<Array>();
    	a->element = GET_NODE(optypes.front());
    	auto sz = MakeTreeNode<SpecificInteger>( (int)(optypes.size()) ); // TODO make it work with size_t and remove the cast
    	a->size = sz;
        return NODE_ONLY(a);
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
		if( DynamicTreePtrCast<Pointer>(GET_NODE(optypes.front())) && 
            DynamicTreePtrCast<Pointer>(GET_NODE(optypes.back())) )
		{
			auto i = MakeTreeNode<Signed>();
			auto nc = MakeTreeNode<SpecificInteger>( TypeDb::integral_bits[INT] );
			i->width = nc;
			return NODE_ONLY(i);
		}
	}

	// Pointer arithmetic: an add or subtract involving a pointer returns that pointer type
	if( DynamicTreePtrCast<Add>(op) || DynamicTreePtrCast<Subtract>(op) )
	{
		for( AugTreePtr<Type> t : optypes )
			if( TreePtr<Pointer> p = DynamicTreePtrCast<Pointer>(GET_NODE(t)) )
		        return CHANGE_NODE(p, t);
	}

#define ARITHMETIC GetStandard( optypes )
#define BITWISE GetStandard( optypes )
#define LOGICAL NODE_ONLY(MakeTreeNode<Boolean>())
#define COMPARISON NODE_ONLY(MakeTreeNode<Boolean>())
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


Transformation::AugTreePtr<CPPTree::Type> HasType::GetStandard( list<AugTreePtr<Type>> &optypes )
{
	list<AugTreePtr<Numeric>> nums;
	for( AugTreePtr<Type> optype : optypes )
		if( auto n = DynamicTreePtrCast<Numeric>(GET_NODE(optype)) )
			nums.push_back(CHANGE_NODE(n, optype));
	if( nums.size() == optypes.size() )
		return GetStandard( nums );

    throw NumericalOperatorUsageMismatch1();
//	if( optypes.size() == 2 )
//		ASSERT(0)("Standard operator unknown usage, please add to HasType class");
//	else
//		ASSERT(0)("Standard operator unknown usage, please add to HasType class");
//    ASSERTFAIL();
}


Transformation::AugTreePtr<CPPTree::Type> HasType::GetStandard( list<AugTreePtr<Numeric>> &optypes )
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
		if( auto f = DynamicTreePtrCast<Floating>(GET_NODE(optype)) )
		{
			TreePtr<SpecificFloatSemantics> sfs = DynamicTreePtrCast<SpecificFloatSemantics>(f->semantics);
			ASSERT(sfs)("Floating point type seen with semantics not specific");
			unsigned int sl = llvm::APFloat::semanticsPrecision( *sfs );
			unsigned int sr = llvm::APFloat::semanticsPrecision( *maxwidth_float );
			if( !maxwidth_float || sl > sr )
				maxwidth_float = sfs;
		}

		// Should only have Integrals from here on
		TreePtr<Integral> intop = DynamicTreePtrCast<Integral>(GET_NODE(optype));
        if( !intop )
            throw NumericalOperatorUsageMismatch2();
        //ASSERT( intop )(*optype)(" is not Floating or Integral, please add to HasType class" );

        // Do a max algorithm on the width
		auto siwidth = DynamicTreePtrCast<SpecificInteger>(intop->width);
        if( !siwidth )
            throw NumericalOperatorUsageMismatch3();
		//ASSERT( siwidth )( "Integral size ")(*(intop->width))(" is not specific, cannot decide result type");
		int64_t width = siwidth->GetInt64(); // here we assume int64_t can hold the widths of integer variablkes

		if( DynamicTreePtrCast<Signed>(GET_NODE(optype)) )
		{
            maxwidth_signed = max( width, maxwidth_signed );
		}
		else if( DynamicTreePtrCast<Unsigned>(GET_NODE(optype)) )
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
		return NODE_ONLY(result);
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
	return NODE_ONLY(result);
}


Transformation::AugTreePtr<CPPTree::Type> HasType::GetSpecial( TreePtr<Operator> op, list<AugTreePtr<Type>> &optypes )
{
    if( dynamic_pointer_cast<Dereference>(op) || dynamic_pointer_cast<Subscript>(op) )
    {
        if( TreePtr<Pointer> o2 = DynamicTreePtrCast<Pointer>( GET_NODE(optypes.front()) ) )
            return PARENT_AND_CHILD(o2, destination);
        else if( TreePtr<Array> o2 = DynamicTreePtrCast<Array>( GET_NODE(optypes.front()) ) )
            return PARENT_AND_CHILD(o2, element);
        else
            throw DereferenceUsageMismatch();
            //ASSERTFAIL( "dereferencing non-pointer" );
    }
    else if( DynamicTreePtrCast<AddressOf>(op) )
    {
        auto p = MakeTreeNode<Pointer>();
        p->destination = GET_NODE(optypes.front());
        return NODE_ONLY(p);
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

Transformation::AugTreePtr<CPPTree::Type> HasType::GetLiteral( TreePtr<Literal> l )
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
        return NODE_ONLY(it);
    }
    else if( auto sf = DynamicTreePtrCast<SpecificFloat>(l) )
    {
    	// Get the info from Clang, and make an Inferno type for it
    	auto ft = MakeTreeNode<Floating>();
    	ft->semantics = MakeTreeNode<SpecificFloatSemantics>( &sf->getSemantics() );
        return NODE_ONLY(ft);
    }
    else if( DynamicTreePtrCast<Bool>(l) )
    {
        return NODE_ONLY(MakeTreeNode<Boolean>());
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
        return NODE_ONLY(p);
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
Transformation::AugTreePtr<CPPTree::Expression> HasType::IsConstructorCall( const TreeKit &kit_, TreePtr<Call> call )
{
	kit = &kit_;
	AugTreePtr<CPPTree::Expression> e;

    if( auto lf = DynamicTreePtrCast<Lookup>(call->callee) )
    {
		ASSERT(lf->member);
		if( DynamicTreePtrCast<Constructor>( GET_NODE(Get( lf->member )) ) )
			e = PARENT_AND_CHILD(lf, base);
    }

    kit = nullptr;
    return e;
}

HasType HasType::instance; 

