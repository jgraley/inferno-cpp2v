
#include "cpptree.hpp"
#include "common/read_args.hpp"
#include "vn/lang/sort_decls.hpp"

#define EXPLICIT_BASE 0

using namespace CPPTree;

//////////////////////////// Uninitialised ///////////////////////////////

Syntax::Production Uninitialised::GetMyProductionTerminal() const
{
	// Most types don't use declarators, so provide a safe default
	return Production::ANONYMOUS; // Renders as an empty string
}

//////////////////////////// Type ///////////////////////////////

Syntax::Production Type::GetMyProductionTerminal() const
{
	// GetMyProductionTerminal on types is for non-booted anonymous types eg with auto a = new <here>;
	// Default shall be to boot (i.e. force parentheses).
	return Production::BOTTOM_EXPR;
}


Syntax::Production Type::GetOperandInDeclaratorProduction() const
{
	// Most types don't use declarators, so provide a safe default
	return Production::BOTTOM_EXPR;
}


string Type::GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy )
{	
	// Declarator may be needed so enter declarator vcall but ask for anonymous by
	// setting declarator string to "". This corresponds to a type-id in https://alx71hub.github.io/hcb/ 
	return GetRenderTypeAndDeclarator( renderer, "", Syntax::Production::ANONYMOUS, surround_prod, policy, false ); 
}


string Type::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
                                         Syntax::Production, Syntax::Production, Syntax::Policy policy,
                                         bool constant)
{
	return (constant?"const ":"") + 
	       GetRenderTypeSpecSeq( renderer, policy ) + 
	       (declarator != "" ? " "+declarator : "");
}                                           


string Type::GetRenderTypeSpecSeq( VN::RendererInterface *renderer, Policy policy )
{
	// This would be a type-specifier-seq in https://alx71hub.github.io/hcb/ 
	// TODO just throw?
	return renderer->RenderNodeExplicit(shared_from_this(), Syntax::Production::EXPLICIT_NODE, policy);
}

//////////////////////////// Declaration ///////////////////////////////

Syntax::Production Declaration::GetMyProductionTerminal() const
{
	return Production::BARE_STMT_DECL;
}

//////////////////////////// DeclScope ///////////////////////////////



//////////////////////////// CodeUnit ///////////////////////////////

Syntax::Production CodeUnit::GetMyProductionTerminal() const
{
	return Production::PROGRAM;
}

string CodeUnit::GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy )
{
    string s;
	(void)surround_prod;
	if( !policy.full_render_code_unit )
	{
		s += "∞{\n";
		for( TreePtr<Node> m : members )
		{
			s += renderer->DoRender( m, Production::STMT_DECL, policy ) + "\n";
		}
		s += "}";
		return s;
	}

    Sequence<Declaration> sorted = SortDecls( members, true );

	queue<TreePtr<Declaration>> require_complete;
	queue<TreePtr<Declaration>> general;
	queue<shared_ptr<Syntax>> my_definitions;
	Policy my_policy = policy;
	my_policy.definitions = &my_definitions;
	
    if( !sorted.empty()  )
		s += "\n// Pre-proc and forward classes\n";   
		
    // Emit preprocs and an incomplete for each record 
    for( TreePtr<Declaration> pd : sorted )
    {       
        if( auto ppd = DynamicTreePtrCast<PreProcDecl>(pd) )
        {
            s += renderer->DoRender( ppd, Production::STMT_DECL, my_policy ) + "\n";
            continue;
        }
        
        if( DynamicTreePtrCast<Record>(pd) && !DynamicTreePtrCast<Enum>(pd) ) 
        {    
			// A record within our scope
			Syntax::Policy record_policy = my_policy;
			record_policy.force_incomplete_records = true; 

			s += renderer->DoRender( pd, Production::STMT_DECL, record_policy ); 
			require_complete.push( pd );
		}
		else
		{
			general.push( pd );
		}
    }
    
    // Emit the complete declarations of user types from in here, sorted for dependencies
    if( !require_complete.empty()  )
		s += "\n// Complete classes\n";    
    
    while( !require_complete.empty()  )
    {
        TreePtr<Declaration> d = require_complete.front();
        require_complete.pop();       		
        s += renderer->DoRender( d, Production::STMT_DECL, my_policy );
    }
    
    if( !general.empty()  )
		s += "\n// General\n";    

    while( !general.empty()  )
    {
        TreePtr<Declaration> d = general.front();
        general.pop();       		
        s += renderer->DoRender( d, Production::STMT_DECL, my_policy );
    }

    if( !my_definitions.empty() )
		s += "\n// Instance definitions\n";    
    
    // Emit the actual definitions of instances from anywhere under here, sorted for dependencies
    // These are rendered here, inside program scope but outside any additional scopes
    // that were on the scope stack when the instance was seen. These could go in a .cpp file.
	Syntax::Policy definition_policy = policy;
	definition_policy.force_initialisation = true;
	while( !my_definitions.empty() )
    {
		auto def = (TreePtr<Node>)(dynamic_pointer_cast<Node>(my_definitions.front()));
		ASSERT(def);
        s += renderer->DoRender( def, Syntax::Production::STMT_DECL, definition_policy ); 
        s += "\n";
        my_definitions.pop();
    }
    return s; 
}

//////////////////////////// SpecificIdentifier ///////////////////////////////

SpecificIdentifier::SpecificIdentifier() :
    addr_bounding_role( BoundingRole::NONE )
{
}


SpecificIdentifier::SpecificIdentifier( string s, BoundingRole addr_bounding_role_ ) : 
    addr_bounding_role(addr_bounding_role_),
    name(s) 
{
}


shared_ptr<Cloner> SpecificIdentifier::Duplicate( shared_ptr<Cloner> p )
{
    return p; // duplicating specific identifiers just gets the same id, since they are unique.
    // This means x.Duplicate() matches x, wheras x.Clone() does not
}


bool SpecificIdentifier::IsLocalMatchCovariant( const Matcher &candidate ) const 
{
    return &candidate == this;
}


Orderable::Diff SpecificIdentifier::OrderCompare3WayCovariant( const Orderable &right, 
                                                               OrderProperty order_property ) const
{
	(void)order_property;
    auto &r = GET_THAT_REFERENCE(right);
        
    //FTRACEC("Compare ")(*this)(" with ")(*r)(": ");

    if( &r == this )
    {
        //FTRACEC("0 (fast out)\n");
        return 0; // fast-out
    }
        
    // Primary ordering on name due rule #528
    if( name != r.name )
    {
        //FTRACEC("%d (name)\n", name.compare(r->name));
        return name.compare(r.name);      
    }
          
    // Optional over-ride of address compare for making ranges, see rule #528
    if( addr_bounding_role != BoundingRole::NONE || r.addr_bounding_role != BoundingRole::NONE )
    {
        return (int)addr_bounding_role - (int)(r.addr_bounding_role);
    }    
    
    // Secondary ordering on identity due rule #528
    return Compare3WayIdentity(*this, r);
}


string SpecificIdentifier::GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy) 
{		
	// Vcall on what kind of id this is
	string s = GetRenderWithoutScope(renderer, policy);          

    // Slight cheat for expediency: if a PURE_IDENTIFIER is expected, suppress scope resolution.
    // This could lead to the rendering of identifiers in the wrong scope. But, most PURE_IDENTIFIER
    // uses are declaring the id, or otherwise can't cope with the :: anyway. 
    if( surround_prod < Syntax::Production::PURE_IDENTIFIER ) 
    {
		auto me = TreePtr<SpecificIdentifier>::DynamicCast( TreePtr<Node>(shared_from_this()) );
		ASSERT(me);			
        s = renderer->RenderScopeResolvingPrefix( me, policy ) + s;   
	}
                                     
    return s;
}


string SpecificIdentifier::GetIdentifierName() const
{
	return name;
}


string SpecificIdentifier::GetDesignationNameHint() const
{
	return GetIdentifierName();
}


bool SpecificIdentifier::IsDesignationNamedIdentifier() const
{
	return true;
}


string SpecificIdentifier::GetGraphName() const
{
    // Since this is text from the program, use single quotes
    string s = "'" + name + "'";
    switch( addr_bounding_role )
    {
        case BoundingRole::NONE:
            break;
        case BoundingRole::MINIMUS:
            s += " MINIMUS";
            break;
        case BoundingRole::MAXIMUS:
            s += " MAXIMUS";
            break;
    }    
    return s;
}


string SpecificIdentifier::GetTrace() const
{
    return GetName() + "(" + GetGraphName() + ")" + GetSerialString();
}


string SpecificIdentifier::GetRenderWithoutScope( VN::RendererInterface *renderer, Policy )
{
	auto me = TreePtr<SpecificIdentifier>::DynamicCast( TreePtr<Node>(shared_from_this()) );
	ASSERT(me);	

	string s = renderer->GetUniqueIdentifierName(me);          
    ASSERT(s.size()>0)(*me)(" rendered to an empty string\n");
    return s;
}

//////////////////////////// InstanceIdentifier //////////////////////////////

Syntax::Production InstanceIdentifier::GetMyProductionTerminal() const
{ 
	return Production::PURE_IDENTIFIER; 
}


//////////////////////////// SpecificConstructorIdentifier //////////////////////////////

// Constructors and destructors have identifiers because the tree is "resolved" which means
// we already know exactly which decl a function call should reach. Mediating the relationship via
// identifiers allows patterns to use couplings to get from usage to decl. These 
// identifiers render differently: where the name would go you have [Scope::][~]Class
// TODO we don't want the name we inherit from SpecificIdentifier so split a NamedIdentifier 
// out and use for named cases, but not these.

string SpecificConstructorIdentifier::GetRenderWithoutScope( VN::RendererInterface *renderer, Policy policy )
{
	auto me = TreePtr<SpecificConstructorIdentifier>::DynamicCast( TreePtr<Node>(shared_from_this()) );
	ASSERT(me);		
	
    // TODO use TryGetRecordDeclaration( Typeof( o->identifier ) ) and leave scopes out of it
    TreePtr<Record> rec = DynamicTreePtrCast<Record>( renderer->TryGetScope( me ) );
    if( !rec )
		throw Unimplemented(); // Constructor must be declared in a record       
		
    return renderer->DoRender( rec->identifier, Syntax::Production::PURE_IDENTIFIER, policy );	// PURE_IDENTIFIER prevents scope resolution
}

//////////////////////////// SpecificDestructorIdentifier //////////////////////////////

string SpecificDestructorIdentifier::GetRenderWithoutScope( VN::RendererInterface *renderer, Policy policy )
{
	auto me = TreePtr<SpecificDestructorIdentifier>::DynamicCast( TreePtr<Node>(shared_from_this()) );
	ASSERT(me);		
	
    // TODO use TryGetRecordDeclaration( Typeof( o->identifier ) ) and leave scopes out of it
    TreePtr<Record> rec = DynamicTreePtrCast<Record>( renderer->TryGetScope( me ) );
    if( !rec )
		throw Unimplemented(); // Constructor must be declared in a record    
		   
    return "~" + 
           renderer->DoRender( rec->identifier, Syntax::Production::PURE_IDENTIFIER, policy );	// PURE_IDENTIFIER prevents scope resolution
}

//////////////////////////// TypeIdentifier //////////////////////////////

Syntax::Production TypeIdentifier::GetMyProductionTerminal() const
{ 
	return Production::PURE_IDENTIFIER; 
}

//////////////////////////// SpecificTypeIdentifier //////////////////////////////

string SpecificTypeIdentifier::GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy )
{
	// Render as identifier for cases that are not a type usage eg declarations
	return SpecificIdentifier::GetRender(renderer, surround_prod, policy);
}


string SpecificTypeIdentifier::GetRenderTypeSpecSeq( VN::RendererInterface *renderer, Policy policy )
{
	// Yes to scope resolution, otherwise we drop scope reolution on type usages
	return SpecificIdentifier::GetRender(renderer, Production::RESOLVER, policy);
}

//////////////////////////// IdValuePair ///////////////////////////////

Syntax::Production IdValuePair::GetMyProductionTerminal() const
{ 
	return Production::COLON_SEP; 
}


string IdValuePair::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
    return renderer->DoRender( key, BoostPrecedence(Production::COLON_SEP), policy ) +
		   "⦂ " +
           renderer->DoRender( value, Production::COLON_SEP, policy );	
}

Sequence<Expression> IdValuePair::SortMapById( Collection<IdValuePair> &id_value_map,
                                               Sequence<Declaration> key_sequence )
{   
    Sequence<Expression> out_sequence;
    for( TreePtr<Declaration> d : key_sequence )
    {
        // We only care about instances...
        if( TreePtr<Instance> i = DynamicTreePtrCast<Instance>( d ) )
        {
            // ...and not function instances
            if( !DynamicTreePtrCast<Callable>( i->type ) )
            {
                // search init for matching member (not a map: this is a Collection of IdValuePair nodes)
                for( TreePtr<IdValuePair> mi : id_value_map )
                {
                    if( i->identifier == mi->key )
                    {
                        out_sequence.push_back( mi->value );
                        break;
                    }
                }
            }
        }
    }
    return out_sequence;
}


//////////////////////////// MapArgumentation ///////////////////////////////

Syntax::Production MapArgumentation::GetMyProductionTerminal() const
{ 
	return Production::BRACKETED; 
}


string MapArgumentation::GetRender( VN::RendererInterface *, Production, Policy )
{
	throw Unimplemented(); // Not syntactically valid in isolation
}


string MapArgumentation::DirectRenderArgumentation(VN::RendererInterface *renderer, Policy policy)
{
	if( policy.refuse_map_argumentation )
		throw RefusedByPolicy(); // Would output 〔, 〕 and ⦂, so C++ renderer needs to resolve into seq args
			
	list<string> ls;
	for( TreePtr<Node> arg : arguments )
		ls.push_back( renderer->DoRender( arg, Production::COMMA_SEP, policy ) );
	
    return Join( ls, ", ", "〔", "〕" );	
}

//////////////////////////// SeqArgumentation ///////////////////////////////

Syntax::Production SeqArgumentation::GetMyProductionTerminal() const
{ 
	return Production::BRACKETED; 
}


string SeqArgumentation::GetRender( VN::RendererInterface *, Production, Policy )
{	
	throw Unimplemented(); // Not syntactically valid in isolation	
}


string SeqArgumentation::DirectRenderArgumentation(VN::RendererInterface *renderer, Policy policy)
{
	list<string> ls;
	for( TreePtr<Node> arg : arguments )
		ls.push_back( renderer->DoRender( arg, Production::COMMA_SEP, policy ) );	
	
	return Join( ls, ", ", "(", ")" );	
}

//////////////////////////// Literal ///////////////////////////////

string Literal::GetName() const
{
	string value_string;
	try
	{
		value_string = "(" + GetRenderTerminal(Syntax::Production::BOTTOM_EXPR) + ")";
	}
	catch(Refusal &) {}
	return Traceable::GetName() + value_string;
}

//////////////////////////// SpecificString ///////////////////////////////

SpecificString::SpecificString() 
{
} 


SpecificString::SpecificString( string s ) :
      value(s) 
{
}


bool SpecificString::IsLocalMatchCovariant( const Matcher &candidate ) const 
{
    auto &c = GET_THAT_REFERENCE(candidate);
    return c.value == value;
}

 
Orderable::Diff SpecificString::OrderCompare3WayCovariant( const Orderable &right, 
                                                           OrderProperty ) const
{
    auto &r = GET_THAT_REFERENCE(right);
    return value.compare(r.value);
}
 
 
string SpecificString::GetRenderTerminal( Production ) const
{
    // Since this is a string literal, output it double quoted
    return "\"" + value + "\"";
}


Syntax::Production SpecificString::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_EXPR; 
}

//////////////////////////// SpecificInteger ///////////////////////////////

SpecificInteger::SpecificInteger()  : 
    value(TypeDb::int_bits, false)
{
}


SpecificInteger::SpecificInteger( llvm::APSInt i ) : 
    value(i)
{
}


SpecificInteger::SpecificInteger( int i ) : 
    value(TypeDb::int_bits, false)  // signed
{ 
    value = i; 
} 


SpecificInteger::SpecificInteger( int64_t i ) : 
    value(64, false)  // 64-bit
{ 
    value = i; 
} 


SpecificInteger::SpecificInteger( unsigned i ) : 
    value(TypeDb::int_bits, true)  // unsigned
{ 
    value = i; 
} 


SpecificInteger::SpecificInteger( uint64_t i ) : 
    value(64, false)  // unsigned
{ 
    value = i; 
} 


int64_t SpecificInteger::GetInt64() const
{
    return value.getSExtValue();
}


bool SpecificInteger::IsSigned() const
{
    return value.isSigned();
}


int64_t SpecificInteger::GetWidth() const
{
    return value.getBitWidth();
}


bool SpecificInteger::IsLocalMatchCovariant( const Matcher &candidate ) const
{
    auto &c = GET_THAT_REFERENCE(candidate);
    // A local match will require all fields to match, not just the numerical value.
    return c.value.isUnsigned() == value.isUnsigned() &&
           c.value.getBitWidth() == value.getBitWidth() &&
           c.value == value;
}


Orderable::Diff SpecificInteger::OrderCompare3WayCovariant( const Orderable &right, 
                                                            OrderProperty ) const
{
    auto &r = GET_THAT_REFERENCE(right);

    if( int d = ((int)value.isUnsigned() - (int)r.value.isUnsigned()) )
        return d;
    if( int d = (int)(value.getBitWidth()) - (int)(r.value.getBitWidth()) )
        return d;
    return (value > r.value) - (value < r.value);
    // Note: just subtracting could overflow
}
 
 
string SpecificInteger::GetRenderTerminal( Production ) const 
{
    return string(value.toString(10)) + // decimal
           (value.isUnsigned() ? "U" : "") +
#if EXPLICIT_BASE
           ("b"+to_string(value.getBitWidth()));
#else            
           (value.getBitWidth()>TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified] ? "L" : "") +
           (value.getBitWidth()>TypeDb::integral_bits[clang::DeclSpec::TSW_long] ? "L" : "");
#endif
           // note, assuming longlong bigger than long, so second L appends first to get LL
}


Syntax::Production SpecificInteger::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_EXPR; 
}

//////////////////////////// SpecificFloat ///////////////////////////////

SpecificFloat::SpecificFloat() :
    llvm::APFloat(0.0) 
{
} 


SpecificFloat::SpecificFloat( llvm::APFloat v ) : 
    llvm::APFloat(v) 
{
}


bool SpecificFloat::IsLocalMatchCovariant( const Matcher &candidate ) const 
{
    auto &c = GET_THAT_REFERENCE(candidate);
    // A local match will require all fields to match, not just the numerical value.
    return bitwiseIsEqual( c ); 
    //     && c.getSemantics() == getSemantics() && //TODO
    
}


Orderable::Diff SpecificFloat::OrderCompare3WayCovariant( const Orderable &right, 
                                                          OrderProperty order_property ) const
{
	(void)order_property;
    auto &r = GET_THAT_REFERENCE(right);
        
    // Primary ordering: the value
    cmpResult cr = compare(r);
    if( cr==APFloat::cmpLessThan )
        return -1;
    if( cr==APFloat::cmpGreaterThan )
        return 1;    
    
    // Secondary ordering: the hash
    uint32_t h = getHashValue();
    uint32_t ch = r.getHashValue();
    return (int)(h > ch) - (int)(h < ch);
    // Note: just subtracting could overflow
}
 

string SpecificFloat::GetRenderTerminal( Production ) const
{
    char hs[256];
    // generate hex float since it can be exact
    convertToHexString( hs, 0, false, llvm::APFloat::rmTowardNegative); // note rounding mode ignored when hex_digits==0
    return string(hs) +
           (&getSemantics()==TypeDb::float_semantics ? "F" : "") +
           (&getSemantics()==TypeDb::long_double_semantics ? "L" : "");
}


Syntax::Production SpecificFloat::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_EXPR; 
}

//////////////////////////// AccessSpec //////////////////////////////

Syntax::Production AccessSpec::GetMyProductionTerminal() const
{ 
	return Production::TERMINAL; 
}


string AccessSpec::GetRender( VN::RendererInterface *, Production surround_prod, Policy policy )
{
	bool in_class_body = (surround_prod == Syntax::Production::BARE_STMT_DECL);
	
	if( type_index(typeid(*this)) == policy.current_access && 
	    in_class_body )
		return ""; // elide because same as previous
		
	string s = GetKeyword();    
    if( in_class_body )
		s += ":\n";	
	
	return s;
}


string AccessSpec::GetKeyword() const
{
	throw UnimplementedToken();
}

//////////////////////////// Public //////////////////////////////

string Public::GetKeyword() const
{
	return "public";
}

//////////////////////////// Private //////////////////////////////

string Private::GetKeyword() const
{
	return "private";
}

//////////////////////////// Protected //////////////////////////////

string Protected::GetKeyword() const
{
	return "protected";
}

//////////////////////////// Instance //////////////////////////////

Syntax::Production Instance::GetMyProduction(const VN::RendererInterface *, Policy policy) const
{ 
	if( !DynamicTreePtrCast<Expression>(initialiser) && policy.force_initialisation )
		return Production::STMT_DECL;
	else
		return Production::BARE_STMT_DECL;
}

//////////////////////////// Base //////////////////////////////

Syntax::Production Base::GetMyProductionTerminal() const
{
	return Production::BASE_CLASS_SPEC;
}


string Base::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	string s;
	s += renderer->DoRender( access, Production::PRIMARY_EXPR, policy );
	s += " ";
	s += renderer->DoRender( record, Production::SPACE_SEP_TYPE, policy );
	return s;
}   

//////////////////////////// LabelIdentifier //////////////////////////////

Syntax::Production LabelIdentifier::GetMyProductionTerminal() const
{ 
	return Production::PURE_IDENTIFIER; 
}

//////////////////////////// SpecificLabelIdentifier //////////////////////////////

Syntax::Production SpecificLabelIdentifier::GetMyProductionTerminal() const
{ 
	return Production::PURE_IDENTIFIER; 
}


string SpecificLabelIdentifier::GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy)
{
	if( policy.goto_uses_ref_and_deref && 
	    surround_prod < Syntax::Production::PURE_IDENTIFIER ) // Don't add && in a label declaration
	{
		// label-as-variable (GCC extension)  
		return "&&" + SpecificIdentifier::GetRender( renderer, Syntax::Production::RESOLVER, policy );
	}		   
    
    return SpecificIdentifier::GetRender( renderer, surround_prod, policy );
}

//////////////////////////// LabelDeclaration //////////////////////////////

Syntax::Production LabelDeclaration::GetMyProductionTerminal() const
{ 
	return Production::LABEL; 
}


string LabelDeclaration::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->DoRender( identifier, Syntax::Production::PURE_IDENTIFIER, policy) + ":";	// TODO try PRIMARY_EXPR like with access specs which also end in :
}

//////////////////////////// Callable //////////////////////////////


Syntax::Production Callable::GetMyProductionTerminal() const
{
	// Rendering as a type.
	// We will require an abstract declarator (we'll hit this node first)
	return Production::DECLARATOR_IN_USE; 
}

Syntax::Production Callable::GetOperandInDeclaratorProduction() const
{
	// Rendering a non-abstract declarator.
	return Production::POSTFIX; // eg int a();
}


//////////////////////////// CallableParams //////////////////////////////

string CallableParams::GetRenderParameterisation(VN::RendererInterface *renderer, Policy policy)
{
    list<string> strings;
    for( auto d : params )
	{
        if( auto o = TreePtr<Instance>::DynamicCast(d) )
        {
			Syntax::Production starting_declarator_prod = Syntax::Production::PURE_IDENTIFIER;
			string name = renderer->DoRender( o->identifier, starting_declarator_prod, policy);
			strings.push_back( renderer->DoRenderTypeAndDeclarator( o->type, name, starting_declarator_prod, Syntax::Production::BARE_STMT_DECL, policy, false ) );
		}
		else
		{
            strings.push_back( renderer->DoRender( d, Syntax::Production::BARE_STMT_DECL, policy ) );
        }		
	}					

    return Join( strings, ", ", "(", ")" );
}

//////////////////////////// Function //////////////////////////////

string Function::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
											 Syntax::Production , Syntax::Production surround_prod, Syntax::Policy policy,
											 bool constant )
{
	string d2 = declarator + 
	            GetRenderParameterisation(renderer, policy) +	             
	            (constant?" const":"");
    return renderer->DoRenderTypeAndDeclarator( return_type, 
                                                d2,
                                                Syntax::Production::POSTFIX, 
                                                surround_prod, 
                                                policy );
}

//////////////////////////// Constructor //////////////////////////////

string Constructor::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
												Syntax::Production , Syntax::Production, Syntax::Policy policy,
												bool )
{
	if( declarator.empty() )
		return "⨤" + GetRenderParameterisation(renderer, policy); // anonymoius
	else
		return declarator + GetRenderParameterisation(renderer, policy);
}

//////////////////////////// Destructor //////////////////////////////

string Destructor::GetRenderTypeAndDeclarator( VN::RendererInterface *, string declarator, 
											   Syntax::Production , Syntax::Production, Syntax::Policy,
											   bool )
{
	if( declarator.empty() )
		return "~()"; // anonymoius
	else
		return declarator + "()";
}

//////////////////////////// Array //////////////////////////////

Syntax::Production Array::GetMyProductionTerminal() const
{ 
	// Rendering as a type.
	// We will require an abstract declarator (we'll hit this node first)
	return Production::DECLARATOR_IN_USE; 
}


Syntax::Production Array::GetOperandInDeclaratorProduction() const
{
	// Rendering a non-abstract declarator.
	return Production::POSTFIX; // eg int a[9];
}


string Array::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
                                   Syntax::Production, Syntax::Production surround_prod, Syntax::Policy policy,
                                   bool constant )
{
	string d2 = declarator + 
	            "[" + 
	            renderer->DoRender( size, Syntax::Production::BOTTOM_EXPR, policy) + 
	            "]";
    return renderer->DoRenderTypeAndDeclarator( element, 
												d2, 
												Syntax::Production::POSTFIX,
												surround_prod,
												policy,
												constant );
}                                   

//////////////////////////// Indirection //////////////////////////////

Syntax::Production Indirection::GetMyProductionTerminal() const 
{ 
	// Rendering as a type.
	// We will require an abstract declarator (we'll hit this node first)
	return Production::DECLARATOR_IN_USE;
}


Syntax::Production Indirection::GetOperandInDeclaratorProduction() const
{
	// Rendering a non-abstract declarator.
	return Production::PREFIX; // eg int *a;
}

//////////////////////////// Pointer //////////////////////////////

string Pointer::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
											Syntax::Production, Syntax::Production surround_prod, Syntax::Policy policy,
											bool constant )
{
	// TODO Pointer node to indicate constancy of pointed-to object - would go into this call to DoRenderTypeAndDeclarator
	string d2 = string(DynamicTreePtrCast<Const>(constancy)?"const ":"") + "*" + (constant?" const ":"") + declarator;
    return renderer->DoRenderTypeAndDeclarator( destination, 
											    d2,
											    Syntax::Production::PREFIX, 
											    surround_prod, 
											    policy, 
											    false ); 
}                                       

//////////////////////////// Reference //////////////////////////////

string Reference::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
											Syntax::Production, Syntax::Production surround_prod, Syntax::Policy policy,
											bool constant )
{
	string d2 = string(DynamicTreePtrCast<Const>(constancy)?"const ":"") + "&" + (constant?" const ":"") + declarator;
    return renderer->DoRenderTypeAndDeclarator( destination, 
											    d2,
											    Syntax::Production::PREFIX, 
											    surround_prod, 
											    policy, 
											    false ); 
}                                       

//////////////////////////// Void ///////////////////////////////

Syntax::Production Void::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_TYPE; // eg auto a = new void;
}


string Void::GetRenderTypeSpecSeq( VN::RendererInterface *, Policy )
{
	return "void";
}

//////////////////////////// Boolean ///////////////////////////////

Syntax::Production Boolean::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_TYPE; // eg auto a = new bool;
}

string Boolean::GetRenderTypeSpecSeq( VN::RendererInterface *, Policy )
{
	return "bool";
}

//////////////////////////// Numeric ///////////////////////////////

Syntax::Production Numeric::GetMyProductionTerminal() const
{ 
	return Production::SPACE_SEP_TYPE; // eg auto a = new unsigned long;
}

//////////////////////////// Integral ///////////////////////////////

string Integral::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
                                             Syntax::Production, Syntax::Production, Syntax::Policy policy,
                                             bool constant)
{
    int64_t width_val;
    auto ic = DynamicTreePtrCast<SpecificInteger>( width );
    if( !ic )
		throw Unimplemented(); // Cannot render because width was wildcarded (using NULL or Integer node)
		
    width_val = ic->GetInt64();

    TRACE("width %" PRId64 "\n", width_val);

	string s = GetRenderTypeSpecSeq( renderer, policy ); // TODO could we not recurse via DoRenderTypeAndDeclarator() - once we've made the declarator string?

    s += " " + declarator;

    // Fix the width
    bool bitfield = !( width_val == TypeDb::char_bits ||
                       width_val == TypeDb::integral_bits[clang::DeclSpec::TSW_short] ||
                       width_val == TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified] ||
                       width_val == TypeDb::integral_bits[clang::DeclSpec::TSW_long] ||
                       width_val == TypeDb::integral_bits[clang::DeclSpec::TSW_longlong] );

    if( bitfield )
    {
		// This function only exists to provide bitfields in member declarations that use declarators.
		// RenderSimpleTypeIntegral() can provide the pure types directly, without bitfields.
		s += SSPrintf(":%" PRId64, width_val);
    }

	return (constant?"const ":"") + s;
}


string Integral::GetRenderTypeSpecSeq( VN::RendererInterface *, Policy )
{
    bool ds;
    int64_t width_bits;
    auto ic = DynamicTreePtrCast<SpecificInteger>( width );
    if(!ic)
		throw UnimplementedIntegralType();
    width_bits = ic->GetInt64();

    TRACE("width %" PRId64 "\n", width);

    if( width_bits == TypeDb::char_bits )
        ds = TypeDb::char_default_signed;
    else
        ds = true;

    // Produce signed or unsigned if required
    // Note: literal strings can be converted to char * but not unsigned char * or signed char *
    bool is_signed = IsSigned();
    string s;
    if( is_signed && !ds )
        s = "signed ";
    else if( !is_signed && ds )
        s = "unsigned ";

    // Fix the width
    if( width_bits <= TypeDb::char_bits )
        s += "char";
    else if( width_bits <= TypeDb::integral_bits[clang::DeclSpec::TSW_short] )
        s += "short";
    else if( width_bits <= TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified] )
        s += "int";
    else if( width_bits <= TypeDb::integral_bits[clang::DeclSpec::TSW_long] )
        s += "long";
    else if( width_bits <= TypeDb::integral_bits[clang::DeclSpec::TSW_longlong] )
        s += "long long";
    else
		throw UnimplementedIntegralType();
		
	if( ReadArgs::use.count("c") )
		s += SSPrintf("/* %d bits */", width_bits );

    return s;	
}

//////////////////////////// SpecificFloatSemantics ///////////////////////////////

SpecificFloatSemantics::SpecificFloatSemantics() 
{
}


SpecificFloatSemantics::SpecificFloatSemantics( const llvm::fltSemantics *s ) : 
    value(s)
{
}


bool SpecificFloatSemantics::IsLocalMatchCovariant( const Matcher &candidate ) const
{
    auto &c = GET_THAT_REFERENCE(candidate);
    return c.value == value;
}


Orderable::Diff SpecificFloatSemantics::OrderCompare3WayCovariant( const Orderable &right, 
                                                                   OrderProperty order_property ) const
{
    auto &r = GET_THAT_REFERENCE(right); 

    Orderable::Diff d;
    switch( order_property )
    {
    case TOTAL:
        // Don't use any particular ordering apart from where the 
        // llvm::fltSemantics are being stored.
        d = (int)(value > r.value) - (int)(value < r.value);
        // Note: just subtracting could overflow
        break;
    case REPEATABLE:
    default:
        // Repeatable ordering stops at type
        d = 0;
        break;
    }
    return d;
}


SpecificFloatSemantics::operator const llvm::fltSemantics &() const 
{
    return *value;
}

//////////////////////////// Floating ///////////////////////////////

string Floating::GetRenderTypeSpecSeq( VN::RendererInterface *, Policy )
{
    string s;
    TreePtr<SpecificFloatSemantics> sem = DynamicTreePtrCast<SpecificFloatSemantics>(semantics);
    ASSERT(sem);

    if( &(const llvm::fltSemantics &)*sem == TypeDb::float_semantics )
        s += "float";
    else if( &(const llvm::fltSemantics &)*sem == TypeDb::double_semantics )
        s += "double";
    else if( &(const llvm::fltSemantics &)*sem == TypeDb::long_double_semantics )
        s += "long double";
    else
        throw UnimplementedFloatingType();

    return s;
}

//////////////////////////// Labeley ///////////////////////////////

Syntax::Production Labeley::GetMyProductionTerminal() const
{
	return Production::POSTFIX; // renders as void *
}

string Labeley::GetRenderTypeSpecSeq( VN::RendererInterface *, Policy policy )
{
	if( policy.refuse_local_node_types ) 
		throw RefuseDueLocal(); 
	return "const void *"; // TODO lower Labely to const void * in a lowering step
}

//////////////////////////// Typedef ///////////////////////////////

Syntax::Production Typedef::GetMyProductionTerminal() const
{
	return Production::BARE_STMT_DECL; 
}

//////////////////////////// Record ///////////////////////////////

TreePtr<AccessSpec> Record::GetInitialAccess() const
{
	return nullptr;
}


Syntax::Production Record::GetMyProductionTerminal() const
{
	return Production::BARE_STMT_DECL;
}


string Record::GetRender( VN::RendererInterface *renderer, Syntax::Production, Policy policy ) 
{
	//throw TemporarilyDisabled(); // TODO fix DeclScope render
	
	string s;
	s += GetKeyword(); // class, struct etc
	s += " ";
	s += renderer->DoRender(identifier, Production::PURE_IDENTIFIER, policy); // Don't want scope resolution when declaring

	if( policy.force_incomplete_records )
		return s;

	s += RenderExtras(renderer, Production::SPACE_SEP_STMT_DECL, policy);
    s += "\n";
	s += RenderBody(renderer, policy);
	return s;
}   


string Record::GetKeyword() const
{
	throw UnimplementedToken();
}


string Record::RenderExtras(VN::RendererInterface *, Syntax::Production, Policy)
{
	return ""; // Nothing extra by default
}


string Record::RenderBody( VN::RendererInterface *renderer, Syntax::Policy policy )
{
	string s;

	// Members
	s += "{\n";

	TreePtr<AccessSpec> a = GetInitialAccess();
	ASSERT( a );
	type_index current_access = type_index(typeid(*a));

    policy.split_bulky_statics = true; // Our scope is a record body
	policy.permit_static_keyword = true; // Our scope is a record body
	
    Sequence<Declaration> sorted = SortDecls( members, true );
	
    // Emit preprocs and an incomplete for each record 
    for( TreePtr<Declaration> d : sorted )
    {       
		// Decide access spec for this declaration (explicit if instance, 
		// otherwise force to Public because decls don't have an access spec). TODO fix this, #877
		TreePtr<AccessSpec> this_access = MakeTreeNode<Public>();
		if( TreePtr<Field> f = DynamicTreePtrCast<Field>(d) )
			this_access = f->access;

		Syntax::Policy access_policy = policy;
		access_policy.current_access = current_access;
		s += renderer->DoRender( this_access, Syntax::Production::BARE_STMT_DECL, access_policy );
		current_access = type_index(typeid(*this_access));

        s += renderer->DoRender( d, Syntax::Production::STMT_DECL, policy );
    }
   
   	s += "}";
		
	return s;
}

//////////////////////////// Union ///////////////////////////////

TreePtr<AccessSpec> Union::GetInitialAccess() const
{
	return MakeTreeNode<Public>();
}


string Union::GetKeyword() const
{
	return "union";
}

//////////////////////////// Enum ///////////////////////////////

string Enum::GetKeyword() const
{
	return "enum";
}


string Enum::RenderBody( VN::RendererInterface *, Syntax::Policy  )
{
	throw TemporarilyDisabled(); // TODO implement enum render
}

//////////////////////////// InheritanceRecord ///////////////////////////////

string InheritanceRecord::RenderExtras(VN::RendererInterface *renderer, Syntax::Production, Policy policy)
{
	list<string> ls;
	for( TreePtr<Base> b : bases )
		ls.push_back( renderer->DoRender(b, Production::BASE_CLASS_SPEC, policy ) );    	
	
	if( ls.empty() )
		return "";
	else
		return " : " + Join( ls, ", ", "", "" );
}

//////////////////////////// Struct ///////////////////////////////

TreePtr<AccessSpec> Struct::GetInitialAccess() const
{
	return MakeTreeNode<Public>();
}


string Struct::GetKeyword() const
{
	return "struct";
}

//////////////////////////// Class ///////////////////////////////

TreePtr<AccessSpec> Class::GetInitialAccess() const
{
	return MakeTreeNode<Private>();
}


string Class::GetKeyword() const
{
	return "class";
}

//////////////////////////// True ///////////////////////////////

Syntax::Production True::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_EXPR; 
}

//////////////////////////// False ///////////////////////////////

Syntax::Production False::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_EXPR; 
}

//////////////////////////// Operators from operator_data.inc ///////////////////////////////

#define PREFIX(TOK, TEXT, NODE, BASE, CAT, PROD, ASSOC) \
Syntax::Production NODE::GetMyProductionTerminal() const \
{ \
	return Production::PROD; \
} \
string NODE::GetRender( VN::RendererInterface *renderer, Production, Policy policy ) \
{ \
	Sequence<Expression>::iterator operands_it = operands.begin(); \
	string s = TEXT; \
	bool paren = false; \
	/* Prevent interpretation as a member function pointer literal */ \
	if( dynamic_cast<AddressOf *>(shared_from_this().get()) ) \
		if( auto id = TreePtr<Identifier>::DynamicCast(*operands_it) ) \
			paren = !renderer->RenderScopeResolvingPrefix( id, policy ).empty(); \
	return s + (paren?"(":"") + renderer->DoRender( *operands_it, Production::PROD, policy) + (paren?")":""); \
} \

#define POSTFIX(TOK, TEXT, NODE, BASE, CAT, PROD, ASSOC) \
Syntax::Production NODE::GetMyProductionTerminal() const \
{ \
	return Production::PROD; \
} \
string NODE::GetRender( VN::RendererInterface *renderer, Production, Policy policy ) \
{ \
	Sequence<Expression>::iterator operands_it = operands.begin(); \
	return renderer->DoRender( *operands_it, Production::PROD, policy) + TEXT; \
} \

#define INFIX(TOK, TEXT, NODE, BASE, CAT, PROD, ASSOC) \
Syntax::Production NODE::GetMyProductionTerminal() const \
{ \
	return Production::PROD; \
} \
string NODE::GetRender( VN::RendererInterface *renderer, Production, Policy policy ) \
{ \
	Production prod_left = Production::PROD; \
	Production prod_right = Production::PROD; \
	switch( Association::ASSOC ) \
	{ \
		case Association::RIGHT: prod_left = BoostPrecedence(prod_left); break; \
		case Association::LEFT:  prod_right = BoostPrecedence(prod_right); break; \
	} \
	Sequence<Expression>::iterator operands_it = operands.begin(); \
	string s = renderer->DoRender( *operands_it, prod_left, policy ); \
	s += TEXT; \
	++operands_it; \
	s += renderer->DoRender( *operands_it, prod_right, policy ); \
	return s; \
}

#include "operator_data.inc"

//////////////////////////// ConditionalOperator ///////////////////////////////

Syntax::Production ConditionalOperator::GetMyProductionTerminal() const
{ 
	// This operator borros ASSIGN's precidence and both are right-associative, 
	// so the two can be mingled freely and will always evaluate right-to-left.
	return Production::ASSIGN; 
}


string ConditionalOperator::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->DoRender( condition, BoostPrecedence(Syntax::Production::ASSIGN), policy ) + 
		   " ? " +
		   // Middle expression boots parser - so you can't split it up using (), [] etc
		   renderer->DoRender( expr_then, Production::BOTTOM_EXPR, policy ) + 
		   " : " +
		   renderer->DoRender( expr_else, Production::ASSIGN, policy );          
}


//////////////////////////// Subscript ///////////////////////////////

Syntax::Production Subscript::GetMyProductionTerminal() const
{ 
	return Production::POSTFIX; 
}

string Subscript::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->DoRender( destination, Production::POSTFIX, policy ) + 
		   "[" +
		   renderer->DoRender( index, Production::BOTTOM_EXPR, policy ) + 
		   "]";       
}

//////////////////////////// ArrayInitialiser ///////////////////////////////

Syntax::Production ArrayInitialiser::GetMyProductionTerminal() const
{ 
	return Production::INITIALISER; 
}


string ArrayInitialiser::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	list<string> renders;    
    for( TreePtr<Expression> e : elements )
		renders.push_back( renderer->DoRender( e, Production::COMMA_SEP, policy ) );
    // Use of ={} in expressions is irregular so handle locally. = is used to disambiguate
    // from a compound statement.
    return "=" + Join(renders, ", ", "{", "}"); 
}

//////////////////////////// This ///////////////////////////////

Syntax::Production This::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_EXPR; 
}


string This::GetRenderTerminal( Production ) const
{
	return "this";
}

//////////////////////////// New ///////////////////////////////

Syntax::Production New::GetMyProductionTerminal() const
{ 
	return Production::PREFIX; 
}


string New::GetRender( VN::RendererInterface *renderer, Production, Policy policy)
{
	return string (DynamicTreePtrCast<Global>(global) ? "::" : "") +
		   "new" + placement_argumentation->DirectRenderArgumentation(renderer, policy) +
		   " " +
		   renderer->DoRender( type, Syntax::Production::TYPE_IN_NEW, policy ) +
		   constructor_argumentation->DirectRenderArgumentation(renderer, policy);
}

//////////////////////////// Delete ///////////////////////////////

Syntax::Production Delete::GetMyProductionTerminal() const
{ 
	return Production::PREFIX; 
}


string Delete::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return string(DynamicTreePtrCast<Global>(global) ? "::" : "") +
		   "delete" +
		   (DynamicTreePtrCast<DeleteArray>(array) ? "[]" : "") +
		   " " + renderer->DoRender( pointer, Syntax::Production::PREFIX, policy );
}


//////////////////////////// Lookup ///////////////////////////////

Syntax::Production Lookup::GetMyProductionTerminal() const
{ 
	return Production::POSTFIX; 
}


string Lookup::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->DoRender(object, Production::POSTFIX, policy) +
		   "." +
		   renderer->DoRender(member, Production::PRIMARY_EXPR, policy);
}

//////////////////////////// Cast ///////////////////////////////

Syntax::Production Cast::GetMyProductionTerminal() const
{ 
	return Production::PREFIX; 
}

string Cast::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	if( policy.refuse_c_style_cast )
		throw RefusedByPolicy();
		
    return "(" + renderer->DoRender( type, Syntax::Production::BOOT_TYPE, policy ) + ")" +
                 renderer->DoRender( operand, Syntax::Production::PREFIX, policy );
}

//////////////////////////// Call ///////////////////////////////

Syntax::Production Call::GetMyProductionTerminal() const
{
	return Production::POSTFIX; 	
}


string Call::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{				
	return renderer->DoRender( callee, Syntax::Production::POSTFIX, policy ) + 
	       argumentation->DirectRenderArgumentation(renderer, policy);
}

//////////////////////////// ConstructInit ///////////////////////////////

Syntax::Production ConstructInit::GetMyProductionTerminal() const
{
	return Production::INITIALISER; 	
}


string ConstructInit::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{		
	if( !policy.detect_and_render_constructor )
		throw RefusedByPolicy(); // TODO find a way of disambiguating from a Call in VN lang
			
	// We never render the identifier for constructors - they are "invisible" and represent
	// the choice of which overload we are bound to.		
	return argumentation->DirectRenderArgumentation(renderer, policy);
}

//////////////////////////// RecordInitialiser ///////////////////////////////

Syntax::Production RecordInitialiser::GetMyProductionTerminal() const
{ 
	return Production::BRACKETED; 
}

//////////////////////////// SizeOf ///////////////////////////////

Syntax::Production SizeOf::GetMyProductionTerminal() const
{ 
	return Production::PREFIX; 	
}


string SizeOf::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "sizeof(" + renderer->DoRender(argument, Production::BOOT_TYPE, policy ) + ")";
}

//////////////////////////// AlignOf ///////////////////////////////

Syntax::Production AlignOf::GetMyProductionTerminal() const
{ 
	return Production::PREFIX; 
}


string AlignOf::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "alignof(" + renderer->DoRender(argument, Production::BOOT_TYPE, policy ) + ")";
}

//////////////////////////// Compound ///////////////////////////////

Syntax::Production Compound::GetMyProductionTerminal() const
{ 
	return Production::COMPOUND;
}


string Compound::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
    string s = " { ";
    for( TreePtr<Declaration> m : members )    
        s += renderer->DoRender( m, Syntax::Production::STMT_DECL, policy );    
    if( policy.compound_uses_vn_separator )
		s += "⚬";
    for( TreePtr<Statement> st : statements )    
        s += renderer->DoRender( st, Syntax::Production::STMT_DECL_LOW, policy );    
    s += " } ";
    return s;
}

//////////////////////////// StatementExpression ///////////////////////////////

Syntax::Production StatementExpression::GetMyProductionTerminal() const
{ 
	return Production::BRACKETED; 
}

/*
string StatementExpression::GetRender( VN::RendererInterface *renderer, Production production, Policy policy )
{
}*/

//////////////////////////// Return ///////////////////////////////

Syntax::Production Return::GetMyProductionTerminal() const
{ 
	return Production::BARE_STMT_DECL; 	
}


string Return::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "return " + renderer->DoRender( return_value, Syntax::Production::SPACE_SEP_STMT_DECL, policy );
}

//////////////////////////// Goto ///////////////////////////////

Syntax::Production Goto::GetMyProductionTerminal() const
{ 
	return Production::BARE_STMT_DECL; 
}


string Goto::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	string s = "goto ";
	bool star = false;
	bool remove_double_deref = false;
	Production prod = Production::SPACE_SEP_STMT_DECL;
    if( policy.goto_uses_ref_and_deref )
    {
		if( !DynamicTreePtrCast< SpecificLabelIdentifier >(destination) )
			star = true;
		else
			remove_double_deref = true;
	}
	
	if( star )
	{
		s += "*";
		prod = Production::PREFIX;
	}
	
	string label = renderer->DoRender( destination, prod, policy );
	
	if( remove_double_deref )
		label = label.substr(2); // REMOVE THE &&
	
	return s + label;
}

//////////////////////////// If ///////////////////////////////

Syntax::Production If::GetMyProductionTerminal() const
{ 
	// If we don't have an else clause, we might steal the else from a 
	// surrounding If node, so drop our precedence a little bit.
	bool has_else_clause = !DynamicTreePtrCast<Nop>(body_else);
	return has_else_clause ? Production::STMT_DECL_HIGH : Production::STMT_DECL_LOW; 
}


string If::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	bool has_else_clause = !DynamicTreePtrCast<Nop>(body_else); // Nop means no else clause
	string s = "if( " +
		       renderer->DoRender( condition, Production::BOTTOM_EXPR, policy ) +
		       " )\n" +
		       renderer->DoRender( body, has_else_clause ? Production::STMT_DECL_HIGH : Production::STMT_DECL_LOW, policy );
		       
    if( has_else_clause )  
        s += "else\n" + renderer->DoRender( body_else, Production::STMT_DECL_LOW, policy);
        
    return s;		       
}

//////////////////////////// Breakable ///////////////////////////////

Syntax::Production Breakable::GetMyProductionTerminal() const
{ 
	return Production::STMT_DECL_HIGH; 
}

//////////////////////////// While ///////////////////////////////

Syntax::Production While::GetMyProductionTerminal() const
{ 
	return Production::STMT_DECL; 
}


string While::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "while( " +
		   renderer->DoRender( condition, Production::BOTTOM_EXPR, policy ) +
		   " )\n" +
		   renderer->DoRender( body, Production::STMT_DECL, policy );
}

//////////////////////////// Do ///////////////////////////////

Syntax::Production Do::GetMyProductionTerminal() const
{ 
	return Production::BARE_STMT_DECL; 
}


string Do::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "do\n" +
		   renderer->DoRender( body, Production::STMT_DECL_LOW, policy ) +
		   "while( " +
		   renderer->DoRender( condition, Production::BOTTOM_EXPR, policy ) +
		   " )" ;
}

//////////////////////////// For ///////////////////////////////

Syntax::Production For::GetMyProductionTerminal() const
{ 
	return Production::STMT_DECL; 
}


string For::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "for( " +
		   renderer->DoRender( initialisation, Production::BOTTOM_EXPR, policy ) +
		   "; " +
		   renderer->DoRender( condition, Production::BOTTOM_EXPR, policy ) +
		   "; " +
		   renderer->DoRender( increment, Production::BOTTOM_EXPR, policy ) +
		   " )\n" +
		   renderer->DoRender( body, Production::STMT_DECL, policy );
}

//////////////////////////// Switch ///////////////////////////////

Syntax::Production Switch::GetMyProductionTerminal() const
{ 
	return Production::STMT_DECL; 
}


string Switch::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "switch( " +
		   renderer->DoRender( condition, Production::BOTTOM_EXPR, policy ) +
		   " )\n" +
		   renderer->DoRender( body, Production::STMT_DECL, policy );
}

//////////////////////////// SwitchTarget ///////////////////////////////

Syntax::Production SwitchTarget::GetMyProductionTerminal() const
{ 
	return Production::LABEL; 
}

//////////////////////////// RangeCase //////////////////////////////

string RangeCase::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "case " + 
	       renderer->DoRender( value_lo, Syntax::Production::EXPR_CONST, policy) + 
	       ".." +
	       renderer->DoRender( value_hi, Syntax::Production::EXPR_CONST, policy) + 
	       ":";	
}

//////////////////////////// Case //////////////////////////////

string Case::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "case " + renderer->DoRender( value, Syntax::Production::EXPR_CONST, policy) + ":";	
}

//////////////////////////// Default //////////////////////////////

string Default::GetRender( VN::RendererInterface *, Production, Policy )
{
	return "default:";	
}

//////////////////////////// Continue ///////////////////////////////

Syntax::Production Continue::GetMyProductionTerminal() const
{ 
	return Production::BARE_STMT_DECL; 
}


string Continue::GetRenderTerminal( Production ) const
{
	return "continue";
}


//////////////////////////// Break ///////////////////////////////

Syntax::Production Break::GetMyProductionTerminal() const
{ 
	return Production::BARE_STMT_DECL; 
}


string Break::GetRenderTerminal( Production ) const
{
	return "break";
}

//////////////////////////// MembInitialisation ///////////////////////////////

Syntax::Production MembInitialisation::GetMyProductionTerminal() const
{ 
	return Production::COMMA_SEP; 
}


string MembInitialisation::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	if( !policy.detect_and_render_constructor )
		throw RefusedByPolicy(); // TODO find a way of disambiguating from a Call in VN lang

	return renderer->DoRender( member_id, Production::PURE_IDENTIFIER, policy ) +
		   renderer->DoRender( initialiser, Production::INITIALISER, policy );
}

//////////////////////////// Nop ///////////////////////////////

Syntax::Production Nop::GetMyProductionTerminal() const
{ 
	return Production::BARE_STMT_DECL; // Force a ;
}


string Nop::GetRender( VN::RendererInterface *, Production, Policy )
{
	return "";
}

//////////////////////////// PreprocessorIdentifier //////////////////////////////

Syntax::Production PreprocessorIdentifier::GetMyProductionTerminal() const
{ 
	return Production::PURE_IDENTIFIER; 
}

//////////////////////////// MacroDeclaration ///////////////////////////////

Syntax::Production MacroDeclaration::GetMyProduction(const VN::RendererInterface *, Policy) const
{ 
	if( !DynamicTreePtrCast<Expression>(initialiser) )
		return Production::STMT_DECL;
	else
		return Production::BARE_STMT_DECL;
}

//////////////////////////// MacroStatement ///////////////////////////////

Syntax::Production MacroStatement::GetMyProductionTerminal() const
{ 
	return Production::STMT_DECL; 
}

//////////////////////////// PreProcDecl ///////////////////////////////

Syntax::Production PreProcDecl::GetMyProductionTerminal() const
{ 
	return Production::PRE_PROC_DIRECTIVE; 
}

