
#include "cpptree.hpp"
#include "type_db.hpp"
#include "helpers/walk.hpp"
#include "misc.hpp"
#include "hastype.hpp"

using namespace CPPTree;


#define INT 0

TreePtr<Node> HasType::operator()( TreePtr<Node> c, TreePtr<Node> root )
{
	context = c;
	auto e = TreePtr<CPPTree::Expression>::DynamicCast(root);
	TreePtr<Node> n;
	if( e ) // if the tree at root is not an expression, return nullptr
		n = Get( e );
	context = TreePtr<Node>();
	return n;
}


TreePtr<Type> HasType::Get( TreePtr<Expression> o )
{
    ASSERT(o);
    
    if( auto ii = DynamicTreePtrCast<SpecificInstanceIdentifier>(o) ) // object or function instance
    {        
        TreePtr<Node> n = HasDeclaration()(context, ii);
        TreePtr<Instance> i = DynamicTreePtrCast<Instance>(n);
        ASSERT(i);
        return i->type; 
    }
    else if( auto op = DynamicTreePtrCast<NonCommutativeOperator>(o) ) // operator
    {
        // Get the types of all the operands to the operator first
        Sequence<Type> optypes;
        for( TreePtr<Expression> o : op->operands )
            optypes.push_back( Get(o) );
        return Get( op, optypes );
    }
    else if( auto op = DynamicTreePtrCast<CommutativeOperator>(o) ) // operator
    {
        // Get the types of all the operands to the operator first
        Sequence<Type> optypes;
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
        TreePtr<Type> t = Get(c->callee); // get type of the function itself
        ASSERT( dynamic_pointer_cast<Callable>(t) )( "Trying to call something that is not Callable");
        if( auto f = DynamicTreePtrCast<Function>(t) )
        	return f->return_type;
        else
        	return MakeTreeNode<Void>();
    }
    else if( auto l = DynamicTreePtrCast<Lookup>(o) ) // a.b; just return type of b
    {
        return Get( l->member );
    }
    else if( auto c = DynamicTreePtrCast<Cast>(o) )
    {
        return c->type;
    }
    else if( auto rl = DynamicTreePtrCast<MakeRecord>(o) )
    {
        return rl->type;
    }
    else if( DynamicTreePtrCast<LabelIdentifier>(o) )
    {
        return MakeTreeNode<Labeley>(); 
    }
    else if( DynamicTreePtrCast<SizeOf>(o) || DynamicTreePtrCast<AlignOf>(o) )
    {
    	TreePtr<Integral> n;
        n = MakeTreeNode<Unsigned>();
       	auto sz = MakeTreeNode<SpecificInteger>( TypeDb::size_t_bits );
    	n->width = sz;
        return n;
    }
    else if( auto n = DynamicTreePtrCast<New>(o) )
    {
        auto p = MakeTreeNode<Pointer>();
        p->destination = n->type;
        return p;
    }
    else if( DynamicTreePtrCast<Delete>(o) )
    {
        return MakeTreeNode<Void>(); 
    }
    else if( auto ce = DynamicTreePtrCast<StatementExpression>(o) )
    {
        if( ce->statements.empty() )
            return MakeTreeNode<Void>(); 
        TreePtr<Statement> last = ce->statements.back();
        if( TreePtr<Expression> e = DynamicTreePtrCast<Expression>(last) )
            return Get(e);
        else
            return MakeTreeNode<Void>(); 
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
TreePtr<Type> HasType::Get( TreePtr<Operator> op, Sequence<Type> optypes )
{
	// Lower types that masquerade as other types in preparation for operand analysis
	// - References go to the referenced type
	// - Arrays go to pointers
	for( TreePtr<Type> &t : optypes )
	{
		while( auto r = DynamicTreePtrCast<Reference>(t) )
			t = r->destination;
		if( auto a = DynamicTreePtrCast<Array>(t) )
		{
			auto p = MakeTreeNode<Pointer>();
			p->destination = a->element;
			t = p;
		}
		// Check we finished the job
		ASSERT( !DynamicTreePtrCast<Reference>(t) );
		ASSERT( !DynamicTreePtrCast<Array>(t) );
	}

	// Turn an array literal into an array
    if( auto al = DynamicTreePtrCast<MakeArray>(op) )
    {
    	auto a = MakeTreeNode<Array>();
    	a->element = optypes.front();
    	auto sz = MakeTreeNode<SpecificInteger>( optypes.size() );
    	a->size = sz;
        return a;
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
		if( DynamicTreePtrCast<Pointer>(optypes.front()) && DynamicTreePtrCast<Pointer>(optypes.back()) )
		{
			auto i = MakeTreeNode<Signed>();
			auto nc = MakeTreeNode<SpecificInteger>( TypeDb::integral_bits[INT] );
			i->width = nc;
			return i;
		}
	}

	// Pointer arithmetic: an add or subtract involving a pointer returns that pointer type
	if( DynamicTreePtrCast<Add>(op) || DynamicTreePtrCast<Subtract>(op) )
	{
		for( TreePtr<Type> t : optypes )
			if( TreePtr<Pointer> p = DynamicTreePtrCast<Pointer>(t) )
		        return p;
	}

#define ARITHMETIC GetStandard( optypes )
#define BITWISE GetStandard( optypes )
#define LOGICAL MakeTreeNode<Boolean>()
#define COMPARISON MakeTreeNode<Boolean>()
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
#include "tree/operator_db.inc"
    else
    {
        throw UnsupportedOperatorMismatch();
        //ASSERT(0)("Unknown operator ")(*op)(" (not in operator_db.inc), please add to HasType class");
        //ASSERTFAIL("");
    }
}


TreePtr<Type> HasType::GetStandard( Sequence<Type> &optypes )
{
	Sequence<Numeric> nums;
	for( TreePtr<Type> optype : optypes )
		if( auto n = DynamicTreePtrCast<Numeric>(optype) )
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


TreePtr<Type> HasType::GetStandard( Sequence<Numeric> &optypes )
{
	// Start the width and signedness as per regular "int" since this is the
	// minimum result type for standard operators
    int64_t maxwidth_signed = TypeDb::integral_bits[INT];
	int64_t maxwidth_unsigned = 0;
	TreePtr<SpecificFloatSemantics> maxwidth_float;

	// Look at the operands in turn
	for( TreePtr<Type> optype : optypes )
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
		return result;
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
	return result;
}


TreePtr<Type> HasType::GetSpecial( TreePtr<Operator> op, Sequence<Type> &optypes )
{
    if( dynamic_pointer_cast<Dereference>(op) || dynamic_pointer_cast<Subscript>(op) )
    {
        if( TreePtr<Pointer> o2 = DynamicTreePtrCast<Pointer>( optypes.front() ) )
            return o2->destination;
        else if( TreePtr<Array> o2 = DynamicTreePtrCast<Array>( optypes.front() ) )
            return o2->element;
        else
            throw DereferenceUsageMismatch();
            //ASSERTFAIL( "dereferencing non-pointer" );
    }
    else if( DynamicTreePtrCast<AddressOf>(op) )
    {
        auto p = MakeTreeNode<Pointer>();
        p->destination = optypes.front();
        return p;
    }
    else if( DynamicTreePtrCast<Comma>(op) )
    {
        return optypes.back();
    }
    else if( DynamicTreePtrCast<ConditionalOperator>(op) )
	{
        Sequence<Type>::iterator optypes_it = optypes.begin();
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

TreePtr<Type> HasType::GetLiteral( TreePtr<Literal> l )
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
        return it;
    }
    else if( auto sf = DynamicTreePtrCast<SpecificFloat>(l) )
    {
    	// Get the info from Clang, and make an Inferno type for it
    	auto ft = MakeTreeNode<Floating>();
    	ft->semantics = MakeTreeNode<SpecificFloatSemantics>( &sf->getSemantics() );
        return ft;
    }
    else if( DynamicTreePtrCast<Bool>(l) )
    {
        return MakeTreeNode<Boolean>();
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
TreePtr<Expression> HasType::IsConstructorCall( TreePtr<Node> c, TreePtr<Call> call )
{
	context = c;
	TreePtr<Expression> e;

    if( auto lf = DynamicTreePtrCast<Lookup>(call->callee) )
    {
		ASSERT(lf->member);
		if( DynamicTreePtrCast<Constructor>( Get( lf->member ) ) )
			e = lf->base;
    }

    context = TreePtr<Node>();
    return e;
}

HasType HasType::instance; 
