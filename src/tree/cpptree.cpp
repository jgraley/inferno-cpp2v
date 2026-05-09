
#include "cpptree.hpp"
#include "common/read_args.hpp"
#include "vn/lang/sort_decls.hpp"
#include "vn/lang/render.hpp"
#include "typeof.hpp"

#define EXPLICIT_BASE 0

#define DEEP_ACCESS_SPECS

using namespace CPPTree;

//////////////////////////// Uninitialised ///////////////////////////////

Syntax::Production Uninitialised::GetMyProductionTerminal() const
{
	return Production::ANONYMOUS;
}

string Uninitialised::GetRender( VN::RendererInterface *, Production, Policy policy )
{	
	if( policy.refuse_invisibles )
		throw RefusedByPolicy();
		
	return "";
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


string Type::GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy ) try
{
	return GetRenderTypeSpecSeq(renderer, policy);
}
catch( Unimplemented & )
{
	// Declarator may be needed so enter declarator vcall but ask for anonymous by
	// setting declarator string to "". This corresponds to a type-id in https://alx71hub.github.io/hcb/ 
	// type-id is hard for the parser to differentiate from a declarator, potentially needing 
	// lots of look-ahead. So clarify using ⍑ symbol. See #888
	if( policy.disambiguate_type_id )
		surround_prod = Production::BOOT_TYPE;
	string s = GetRenderTypeAndDeclarator( renderer, "", Production::ANONYMOUS, surround_prod, policy, MakeTreeNode<NonConst>() );
	if( policy.disambiguate_type_id )
		s = "⍑⍑(" + s + ")"; 
	return s;
}


string Type::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
                                         Production, Production, Policy policy,
                                         TreePtr<Node> constant)
{
	auto dc = TreePtr<Constancy>::DynamicCast(constant);	
	return GetRenderTypeSpecSeq( renderer, policy ) + 
		   renderer->DoRender(&dc, Production::SPACE_SEP_STMT_DECL, policy) + 
	       (declarator != "" ? " "+declarator : "");
}                                           


string Type::GetRenderTypeSpecSeq( VN::RendererInterface *, Policy  )
{
	// This would be a type-specifier-seq in https://alx71hub.github.io/hcb/ 
	throw Unimplemented();
}

//////////////////////////// Declaration ///////////////////////////////

Syntax::Production Declaration::GetMyProductionTerminal() const
{
	return Production::BARE_STMT_DECL;
}


bool Declaration::ShouldSplitInstance( Policy ) const 
{ 
	// Splitting instances is generally unsafe, unless we know how to do it for a given kind of Declaration.
	return false; 
}	


list<string> Declaration::ApplyAndRenderAccessSpec( TreePtr<Node> new_access, bool force, VN::RendererInterface *renderer, Policy policy ) const
{
	// Note 1: we will render an access spec if the pointer changes, even if it's just switching
	// to another access of the same type. This is because the renderer does not duplicate 
	// accesses used more than once but instead couples the usage to the same access. 
	
	// Note 2: access specs are attached to declarations, not the surrounding record, so that
	// for example a delta pattern can be used to change the access spec of a field.
	
	list<string> ls;	
	if( policy.cur_access )
	{
		//ls.push_back( "/* "+Trace(*policy.cur_access)+" -> "+Trace(new_access)+" */" );
		if( new_access.get() != policy.cur_access->get() )			
			ls.push_back( renderer->DoRenderPreserve( new_access, Production::BARE_STMT_DECL, policy ) + ":" );	
		*policy.cur_access = new_access;
	}
	else if( force )
	{
		//ls.push_back( "/* forced */" );
		// Parser cannot determine the access any other way, so treat as if always given
		ls.push_back( renderer->DoRenderPreserve( new_access, Production::BARE_STMT_DECL, policy ) + ":" );
	}

	return ls;
}

//////////////////////////// DeclScope ///////////////////////////////

//////////////////////////// CodeUnit ///////////////////////////////

Syntax::Production CodeUnit::GetMyProductionTerminal() const
{
	return Production::PROGRAM;
}

string CodeUnit::GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy )
{
	INDENT("U");
    string s;
	(void)surround_prod;
 	policy.permit_static_keyword = false; // No support for code-unit statics
	policy.cur_access = nullptr; // No access specs here

	if( !policy.full_render_code_unit )
	{
		s += "∞{\n";
		for( auto &m : members )
			s += renderer->DoRender( &m, Production::STMT_DECL, policy ) + "\n";
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
    for( auto &pd : sorted )
    {       
        if( auto ppd = DynamicTreePtrCast<PreProcDecl>(pd) )
        {
            s += renderer->DoRender( &ppd, Production::STMT_DECL, my_policy ) + "\n";
            continue;
        }
        
        if( DynamicTreePtrCast<Record>(pd) && !DynamicTreePtrCast<Enum>(pd) ) 
        {    
			// A record within our scope
			Policy record_policy = my_policy;
			record_policy.force_incomplete_records = true; 

			s += renderer->DoRender( &pd, Production::STMT_DECL, record_policy ); 
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
        s += renderer->DoRender( &require_complete.front(), Production::STMT_DECL, my_policy );
        require_complete.pop();       		
    }
    
    if( !general.empty()  )
		s += "\n// General\n";    

    while( !general.empty()  )
    {
        s += renderer->DoRender( &general.front(), Production::STMT_DECL, my_policy );
        general.pop();       		
    }

    if( !my_definitions.empty() )
		s += "\n// Instance definitions\n";    
    
    // Emit the actual definitions of instances from anywhere under here, sorted for dependencies
    // These are rendered here, inside program scope but outside any additional scopes
    // that were on the scope stack when the instance was seen. These could go in a .cpp file.
	Policy definition_policy = policy;
	definition_policy.rendering_definitions = true;
	while( !my_definitions.empty() )
    {
		// def points to Declaration so that Instances won't refuse due policy.pointer_archetype
		TreePtr<Declaration> def = (TreePtr<Node>)(dynamic_pointer_cast<Node>(my_definitions.front()));
		ASSERT(def);
        s += renderer->DoRender( &def, Production::STMT_DECL, definition_policy ); 
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
        
    if( &r == this )
        return 0; // fast-out
        
    // Primary ordering on name due rule #528
    if( name != r.name )
        return name.compare(r.name);      
          
    // Optional over-ride of address compare for making ranges, see rule #528
    if( addr_bounding_role != BoundingRole::NONE || r.addr_bounding_role != BoundingRole::NONE )
        return (int)addr_bounding_role - (int)(r.addr_bounding_role);  
    
    // Secondary ordering on identity due rule #528
    return Compare3WayIdentity(*this, r);
}


string SpecificIdentifier::GetRender( VN::RendererInterface *renderer, Production, Policy policy) 
{		
	// Vcall on what kind of id this is
	string s = GetRenderWithoutScope(renderer, policy);          

    if( policy.resolve_identifier_scope) 
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
	return Production::PRIMARY_EXPR; 
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
	Policy id_policy = policy;
	id_policy.resolve_identifier_scope = false;// prevents scope resolution
	auto me = TreePtr<Node>(shared_from_this());		
	
    TreePtr<Record> rec = DynamicTreePtrCast<Record>( renderer->TryGetScope( me ) );
    if( !rec )
		throw Unimplemented(); // Constructor must be declared in a record       
		
    return renderer->DoRender( &rec->identifier, Production::PRIMARY_EXPR, id_policy );	
}

//////////////////////////// SpecificDestructorIdentifier //////////////////////////////

string SpecificDestructorIdentifier::GetRenderWithoutScope( VN::RendererInterface *renderer, Policy policy )
{
	Policy id_policy = policy;
	id_policy.resolve_identifier_scope = false; // prevents scope resolution
	auto me = TreePtr<Node>(shared_from_this());	
	
    TreePtr<Record> rec = DynamicTreePtrCast<Record>( renderer->TryGetScope( me ) );
    if( !rec )
		throw Unimplemented(); // Constructor must be declared in a record    
		   
    return "~" + 
           renderer->DoRender( &rec->identifier, Production::PRIMARY_EXPR, id_policy );	
}

//////////////////////////// TypeIdentifier //////////////////////////////

Syntax::Production TypeIdentifier::GetMyProductionTerminal() const
{ 
	return Production::PRIMARY_EXPR; 
}

//////////////////////////// SpecificTypeIdentifier //////////////////////////////

string SpecificTypeIdentifier::GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy )
{
	// Render as identifier for cases that are not a type usage eg declarations
	return SpecificIdentifier::GetRender(renderer, surround_prod, policy);
}


string SpecificTypeIdentifier::GetRenderTypeSpecSeq( VN::RendererInterface *renderer, Policy policy )
{
	// Yes to scope resolution, otherwise we drop scope resolution on type usages
	return SpecificIdentifier::GetRender(renderer, Production::PRIMARY_EXPR, policy); 
}

//////////////////////////// IdValuePair ///////////////////////////////

Syntax::Production IdValuePair::GetMyProductionTerminal() const
{ 
	return Production::COLON_SEP; 
}


string IdValuePair::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
    return renderer->DoRender( &key, BoostPrecedence(Production::COLON_SEP), policy ) +
		   "⦂ " +
           renderer->DoRender( &value, Production::COLON_SEP, policy );	
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
	return Production::PARENTHESISED; 
}


string MapArgumentation::GetRender( VN::RendererInterface *, Production, Policy )
{
	throw Unimplemented(); // Not syntactically valid in isolation
}


string MapArgumentation::DirectRenderArgumentation(VN::RendererInterface *renderer, Policy policy)
{
	if( policy.convert_argumentation_to_seq )
		throw RefusedByPolicy(); // Would output 〔, 〕 and ⦂, so C++ renderer needs to resolve into seq args
			
	list<string> ls;
	for( auto &arg : arguments )
		ls.push_back( renderer->DoRender( &arg, Production::COMMA_SEP, policy ) );
	
    return Join( ls, ", ", "〔", "〕" );	
}


TreePtr<Argumentation> MapArgumentation::ConvertToSeqIfPolicyAllows(TreePtr<Expression> callee, VN::RendererInterface *renderer, Policy policy)
{
	if( !policy.convert_argumentation_to_seq )
		return TreePtr<SeqArgumentation>( shared_from_this() ); // No conversion needed, so return self

	// Note: we need to operate on the call, so that we can use callee to find the function type 
	// and resolve the map into a sequence.

	// Determine the type of the callee function or constructor
	TreePtr<Type> callee_type;
	try
	{
		callee_type = TypeOf::instance.Get(*renderer->GetTransKit(), callee).GetTreePtr();
		ASSERT( callee_type );
	}
	catch( BaseDeclarationOf::DeclarationNotFound &dnf )
	{
		throw RefuseDueUndeclared(callee);
	}
	catch( ::Mismatch &ex )
	{
		ASSERT(false)("TypeOf failure: ")(ex.what())(" on ")(callee);
	}	

	// Convert f->params from Parameters to Declarations and settle on an arbitrary 
	// ordering. This needs to be the same on each visit with a given callee.
	Sequence<Declaration> decl_sequence;   
	if( auto f = TreePtr<CallableParams>::DynamicCast(callee_type) )  
		for( auto param : f->params )
			decl_sequence.push_back(param); 

	// Determine args sequence using param sequence
	auto sa = MakeTreeNode<SeqArgumentation>();
	sa->arguments = IdValuePair::SortMapById( arguments, decl_sequence );
	
	return sa;
}

//////////////////////////// SeqArgumentation ///////////////////////////////

Syntax::Production SeqArgumentation::GetMyProductionTerminal() const
{ 
	return Production::PARENTHESISED; 
}


string SeqArgumentation::GetRender( VN::RendererInterface *, Production, Policy )
{	
	throw Unimplemented(); // Not syntactically valid in isolation	
}


string SeqArgumentation::DirectRenderArgumentation(VN::RendererInterface *renderer, Policy policy)
{
	list<string> ls;
	for( auto &arg : arguments )
		ls.push_back( renderer->DoRender( &arg, Production::COMMA_SEP, policy ) );	
	
	return Join( ls, ", ", "(", ")" );	
}


TreePtr<Argumentation> SeqArgumentation::ConvertToSeqIfPolicyAllows(TreePtr<Expression>, VN::RendererInterface *, Policy)
{
	return TreePtr<SeqArgumentation>( shared_from_this() );
}

//////////////////////////// Literal ///////////////////////////////

string Literal::GetName() const
{
	string value_string;
	try
	{
		value_string = "(" + GetRenderTerminal(Production::BOTTOM_EXPR) + ")";
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
	// TODO use \n \r etc 
	string s;
    for( string::size_type i=0; i<value.size(); i++ )
    {
        char c[10];
        if( value[i] < ' ' )
            s += SSPrintf( c, "\\x%02x", value[i] );
        else
            s += value[i];
    }
    
    // Since this is a string literal, output it double quoted
    return "\"" + s + "\"";
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

//////////////////////////// MemberInitialiser ///////////////////////////////

Syntax::Production MemberInitialiser::GetMyProductionTerminal() const
{ 
	return Production::COMMA_SEP; 
}


string MemberInitialiser::GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy )
{
	string s;
	if( surround_prod == Syntax::Production::VN_SEP_ITEMS )
		s += "‽" + VN::Render::RenderNodeTypeName(shared_from_this()) + " ";  // As an item, this conflicts with a function call so disambiguate using pre-restriction

	Policy id_policy = policy;
	id_policy.resolve_identifier_scope = false;

	return s + renderer->DoRender( &member_id, Production::PRIMARY_EXPR, id_policy ) +
		   "(" + 
		   renderer->DoRender( &initialiser, Production::BOOT, policy ) + 
		   ")";
}

//////////////////////////// MembInitSeq ///////////////////////////////

list<string> MembInitSeq::RenderMemberInits( VN::RendererInterface *renderer, Policy policy )
{	
    if( memb_inits.empty() )        
		return {};

    // Render the constructor initialisers
	list<string> ls = {":"}; 
	bool first = true;
	for( auto &initialiser : memb_inits ) 
	{
		if( !first )
			ls.back() += ",";
		first = false;
		ls.push_back( renderer->DoRender( &initialiser, Production::COMMA_SEP, policy ) );		
	}
    return ls;		    
}

//////////////////////////// Virtuality //////////////////////////////

Syntax::Production Virtuality::GetMyProductionTerminal() const
{
	return Production::KEYWORD;
}

//////////////////////////// Virtual //////////////////////////////

string Virtual::GetRender( VN::RendererInterface *, Production , Policy  )
{
	return "virtual";
}

//////////////////////////// NonVirtual //////////////////////////////

string NonVirtual::GetRender( VN::RendererInterface *, Production, Policy )
{
	return "";
}

//////////////////////////// AccessSpec //////////////////////////////

Syntax::Production AccessSpec::GetMyProductionTerminal() const
{ 
	return Production::KEYWORD; 
}

//////////////////////////// Public //////////////////////////////

string Public::GetRender( VN::RendererInterface *, Production , Policy )
{
	return "public";
}

//////////////////////////// Private //////////////////////////////

string Private::GetRender( VN::RendererInterface *, Production , Policy )
{
	return "private";
}

//////////////////////////// Protected //////////////////////////////

string Protected::GetRender( VN::RendererInterface *, Production , Policy )
{
	return "protected";
}

//////////////////////////// Constancy //////////////////////////////

Syntax::Production Constancy::GetMyProductionTerminal() const
{ 
	return Production::KEYWORD; 
}

//////////////////////////// Const //////////////////////////////

string Const::GetRender( VN::RendererInterface *, Production, Policy )
{
	return "const";
}

//////////////////////////// NonConst //////////////////////////////

string NonConst::GetRender( VN::RendererInterface *, Production surround_prod, Policy )
{
	if( surround_prod == Syntax::Production::VN_SEP_ITEMS )
		throw RefuseInItemisation(); // Not defaulted in itemisations
	return "";
}

//////////////////////////// Instance //////////////////////////////

Syntax::Production Instance::GetMyProduction(const VN::RendererInterface *, Policy policy) const
{ 
	bool will_split = policy.can_split_instances && 
					  !policy.rendering_definitions && 
					  ShouldSplitInstance(policy);
	
	if( !dynamic_pointer_cast<Declaration>(policy.pointer_archetype) )
		return Production::PREFIX;
	else if( initialiser && DynamicTreePtrCast<Compound>(initialiser) && !will_split ) 
		return Production::STMT_DECL; // won't get ; added
	else
		return Production::BARE_STMT_DECL; // will get ; added
}


string Instance::GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy )
{        
	(void)surround_prod;
	string s;
	
	s += GetRenderImpl( renderer, policy );
		
	// Don't render if the incoming pointer is not some kind of Declaration. This catches cases
	// like Stuff terminus, where the parser won't have enough information to determine whether
	// we have Local, Global, Field etc. TODO better to use a scope-like thing?
	if( !dynamic_pointer_cast<Declaration>(policy.pointer_archetype) )
	{
		s = "‽" + 
		    VN::Render::RenderNodeTypeName(shared_from_this()) + 
		    "(" +
		    s + 
		    ")";
	}
		
	return s;
}

			
string Instance::GetRenderImpl( VN::RendererInterface *renderer, Policy policy )
{
	list<string> ls;
	
	Policy sub_policy = policy;
    sub_policy.rendering_definitions = false; // stop at one level
	Policy id_policy = sub_policy;
	if( !policy.rendering_definitions )
		id_policy.resolve_identifier_scope = false;

    if( !policy.rendering_definitions )
    {
		Append( ls, RenderAccessSpec(renderer, sub_policy) );
		Append( ls, RenderDeclSpecPre(renderer, sub_policy) );
	}
    
    string declarator = renderer->DoRender( &identifier, Production::PRIMARY_EXPR, id_policy );   
    ls.push_back( renderer->DoRenderTypeAndDeclarator(&type, declarator, Production::PRIMARY_EXPR, Production::BARE_STMT_DECL, sub_policy, constancy) );

    if( !policy.rendering_definitions )
		Append( ls, RenderDeclSpecPost(renderer, sub_policy) );
		    
	if( policy.can_split_instances && 
		!policy.rendering_definitions && 
		!TreePtr<Uninitialised>::DynamicCast(initialiser) &&
		ShouldSplitInstance(policy) )
	{
		// Emit just a prototype now and request definition later
		// Split out the definition of the instance for rendering later at CodeUnit scope
		ASSERT(policy.definitions); // Not under a node that can render definitions
		policy.definitions->push(TreePtr<Instance>(shared_from_this()));
		return Join( ls, " " );
	}		

	// Member inits
	Append( ls, RenderInitPre(renderer, sub_policy) );

 	if( ReadArgs::use.contains("c") )
		ls.push_back( "/* Instance initialiser */" );
		
	// Use DIRECT_INIT so accomodation maybe adds an = depending on the node
    if( !TreePtr<Uninitialised>::DynamicCast(initialiser) )
		ls.push_back( renderer->DoRender(&initialiser, Production::DIRECT_INIT, sub_policy) );

	return Join( ls, " " ) + (policy.rendering_definitions?"\n":"");
}


// Decide what gets split into a part that goes into the record (main line of rendering) and
// a part that goes separately (definitions get appended at end of code unit).
bool Instance::ShouldSplitInstance( Policy ) const
{
	// By default for Instance, split a function but not a data object. There are overrides, 
	// and a more general default in Declaration.
    return !!DynamicTreePtrCast<Callable>( type );
}


list<string> Instance::RenderAccessSpec( VN::RendererInterface *, Policy policy ) const
{
	if( policy.cur_access ) // are we in a record scope
		throw NonFieldInRecord(); 
	// Patterns managed to get an Instance that isn't a Field into a Record body.
	return {};
}


list<string> Instance::RenderDeclSpecPre( VN::RendererInterface *, Policy ) const 
{ 
	return {}; 
}


list<string> Instance::RenderDeclSpecPost( VN::RendererInterface *, Policy )
{
	return {};
}


list<string> Instance::RenderInitPre( VN::RendererInterface *, Policy ) 
{
	return {};
}

//////////////////////////// Global //////////////////////////////

list<string> Global::RenderDeclSpecPre( VN::RendererInterface *, Policy policy ) const 
{ 
    if( policy.permit_static_keyword )
        return {"static"};
    else
		return {};
}


bool Global::ShouldSplitInstance( Policy policy ) const
{
	if( policy.split_bulky_statics )
	{
		if( DynamicTreePtrCast<Const>(constancy) && DynamicTreePtrCast<Numeric>( type ) )
			return false;

		return true;                
	}
	return false;
}

//////////////////////////// Field //////////////////////////////

list<string> Field::RenderAccessSpec( VN::RendererInterface *renderer, Policy policy ) const
{		
	// If we don't have a current access, the fact that this is a field 
	// over-rides, so force an access spec to be rendered.
	return ApplyAndRenderAccessSpec( access, true, renderer, policy );
}


list<string> Field::RenderDeclSpecPre( VN::RendererInterface *renderer, Policy policy) const 
{ 
	// Demand consistency between type and identifier, as well as limitations on qualifiers
	if( TreePtr<Constructor>::DynamicCast(type) )
	{
		ASSERT( !identifier || TreePtr<ConstructorIdentifier>::DynamicCast(identifier) )(identifier);
		ASSERT( TreePtr<NonConst>::DynamicCast(constancy) )(constancy);
		ASSERT( TreePtr<NonVirtual>::DynamicCast(virt) )(virt);
	}
	if( TreePtr<Destructor>::DynamicCast(type) )
	{
		ASSERT( TreePtr<NonConst>::DynamicCast(constancy) )(constancy);
		ASSERT( !identifier || TreePtr<DestructorIdentifier>::DynamicCast(identifier) )(identifier);	
		// Virtual is allowed for destructors
	}

	return { renderer->DoRender(&virt, Production::SPACE_SEP_STMT_DECL, policy) };
}


list<string> Field::RenderInitPre( VN::RendererInterface *renderer, Policy policy ) 
{
	return RenderMemberInits(renderer, policy);
}

//////////////////////////// Temporary //////////////////////////////

list<string> Temporary::RenderDeclSpecPre( VN::RendererInterface *, Policy policy ) const 
{ 
	if( policy.refuse_local_node_types )
		throw RefuseDueLocal(); 
		
	return { "/*temp*/" };
}

//////////////////////////// Base //////////////////////////////

Syntax::Production Base::GetMyProductionTerminal() const
{
	return Production::BASE_CLASS_SPEC;
}


string Base::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	string s;
	s += renderer->DoRender( &access, Production::PRIMARY_EXPR, policy );
	s += " ";
	s += renderer->DoRender( &record, Production::SPACE_SEP_TYPE, policy );
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
	    surround_prod < Production::PURE_IDENTIFIER ) // Don't add && in a label declaration
	{
		// label-as-variable (GCC extension)  
		return "&&" + SpecificIdentifier::GetRender( renderer, Production::PRIMARY_EXPR, policy );
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
	Policy id_policy = policy;
	id_policy.resolve_identifier_scope = false;
	// Two reasons to add the semicolon at the end:
	// 1. C syntax: a label at the end of a compound would otherwise render as a label 
	//    without a statement which is not allowed, see https://alx71hub.github.io/hcb/#labeled-statement 
	// 2. The parser rules for labels must be symmetrical to those for access specs
	//    and the latter are tightly bound to declarations so that we can for example
	//    change the access in a delta pattern. The ; gets around this by making the 
	//    label a complete statement.
	return renderer->DoRender( &identifier, Production::PURE_IDENTIFIER, id_policy) + ":" + ";";
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


string Callable::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
											 Production , Production, Policy policy,
											 TreePtr<Node> constant )
{
	throw RefuseDifficultSyntax(); 
	// This will render into something ambiguous with expressions
	
	// We have no return type, so we can't recurse and might as well complete the render here
	return UpdateDeclarator( renderer, declarator, policy, constant );
}


string Callable::UpdateDeclarator( VN::RendererInterface *renderer, string declarator, Policy policy,
                                   TreePtr<Node> constant ) 
{
	auto dc = TreePtr<Constancy>::DynamicCast(constant);	
	return declarator + 
	       GetRenderParameterisation(renderer, policy) +	             
	       renderer->DoRender(&dc, Production::SPACE_SEP_STMT_DECL, policy);
}                  


string Callable::GetRenderParameterisation(VN::RendererInterface *, Policy )
{ 
	return "()";
}

//////////////////////////// CallableParams //////////////////////////////

string CallableParams::GetRenderParameterisation(VN::RendererInterface *renderer, Policy policy)
{
	INDENT("P");
	policy.cur_access = nullptr; // No access spec here
		
    list<string> strings;
    for( auto &d : params )	
		strings.push_back( renderer->DoRender( &d, Production::BARE_STMT_DECL, policy ) );       

    return Join( strings, ", ", "(", ")" );
}

//////////////////////////// CallableParamsReturn //////////////////////////////

string CallableParamsReturn::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
											 Production, Production surround_prod, Policy policy,
											 TreePtr<Node> constant )
{
	string d2 = UpdateDeclarator( renderer, declarator, policy, constant);
    return renderer->DoRenderTypeAndDeclarator( &return_type, 
                                                d2,
                                                Production::POSTFIX, 
                                                surround_prod, 
                                                policy,
                                                MakeTreeNode<NonConst>() );
}

//////////////////////////// Constructor //////////////////////////////

string Constructor::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
												Production , Production, Policy policy,
												TreePtr<Node> constant )
{
	string d2 = UpdateDeclarator(renderer, declarator, policy, constant);
	if( policy.use_vn_xstructor_symbol )
		return "⨤" + d2; 
	else
		return d2; 
}

//////////////////////////// Destructor //////////////////////////////

string Destructor::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
											   Production , Production, Policy policy,
											   TreePtr<Node> constant)
{
	if( declarator.empty() )
		declarator = "~";
		
	string d2 = UpdateDeclarator(renderer, declarator, policy, constant);
	if( policy.use_vn_xstructor_symbol )
		return "⨤" + d2;
	else
		return d2; 
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
										  Production, Production surround_prod, Policy policy,
										  TreePtr<Node> constant )
{
	string d2 = declarator + 
	            "[" + 
	            renderer->DoRender( &size, Production::BOTTOM_EXPR, policy) + 
	            "]";
    return renderer->DoRenderTypeAndDeclarator( &element, 
												d2, 
												Production::POSTFIX,
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
											Production, Production surround_prod, Policy policy,
											TreePtr<Node> constant )
{
	// TODO Pointer node to indicate constancy of pointed-to object - would go into this call to DoRenderTypeAndDeclarator
	auto dc = TreePtr<Constancy>::DynamicCast(constant);	
	string d2 = "*" + 
	            renderer->DoRender(&dc, Production::SPACE_SEP_STMT_DECL, policy) + 
	            " " + 
	            declarator;
    return renderer->DoRenderTypeAndDeclarator( &destination, 
											    d2,
											    Production::PREFIX, 
											    surround_prod, 
											    policy, 
											    constancy ); 
}                                       

//////////////////////////// Reference //////////////////////////////

string Reference::GetRenderTypeAndDeclarator( VN::RendererInterface *renderer, string declarator, 
											Production, Production surround_prod, Policy policy,
											TreePtr<Node> constant )
{
	auto dc = TreePtr<Constancy>::DynamicCast(constant);
	string d2 = "&" + 
	            renderer->DoRender(&dc, Production::SPACE_SEP_STMT_DECL, policy) + 
	            " " + 
	            declarator;
    return renderer->DoRenderTypeAndDeclarator( &destination, 
											    d2,
											    Production::PREFIX, 
											    surround_prod, 
											    policy, 
											    constancy ); 
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
                                             Production declarator_prod, Production surround_prod, Policy policy,
                                             TreePtr<Node> constant)
{
    int64_t width_val;
    auto ic = DynamicTreePtrCast<SpecificInteger>( width );
    if( !ic )
		throw Unimplemented(); // Cannot render because width was wildcarded (eg using NULL or Integer node)
		
    // Fix the width by possibly adding bitfield notation to the declarator
    width_val = ic->GetInt64();
    TRACE("width %" PRId64 "\n", width_val);
    bool bitfield = !( width_val == TypeDb::char_bits ||
                       width_val == TypeDb::integral_bits[clang::DeclSpec::TSW_short] ||
                       width_val == TypeDb::integral_bits[clang::DeclSpec::TSW_unspecified] ||
                       width_val == TypeDb::integral_bits[clang::DeclSpec::TSW_long] ||
                       width_val == TypeDb::integral_bits[clang::DeclSpec::TSW_longlong] );

    if( bitfield )
    {
		// This function only exists to provide bitfields in member declarations that use declarators.
		// RenderSimpleTypeIntegral() can provide the pure types directly, without bitfields.
		declarator += SSPrintf(":%" PRId64, width_val);
		declarator_prod = Production::POSTFIX;
    }

	return Type::GetRenderTypeAndDeclarator( renderer, declarator, declarator_prod, surround_prod, policy, constant );
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
		
	if( ReadArgs::use.contains("c") )
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
		
	// Note: all instances must be const
	return "void *"; // TODO lower Labely to const void * in a lowering step
}

//////////////////////////// Typedef ///////////////////////////////

Syntax::Production Typedef::GetMyProductionTerminal() const
{
	return Production::BARE_STMT_DECL; 
}


string Typedef::GetRender( VN::RendererInterface *renderer, Production, Policy policy ) 
{
    list<string> ls;
	if( policy.missing_access_to_public )
	    Append( ls, ApplyAndRenderAccessSpec( MakeTreeNode<Public>(), false, renderer, policy ) ); // see #877

	Syntax::Policy id_policy = policy;
	id_policy.resolve_identifier_scope = false;
    Syntax::Production starting_declarator_prod = Syntax::Production::PRIMARY_EXPR;
    auto id = renderer->DoRender( &identifier, starting_declarator_prod, id_policy );
    
	ls.push_back("typedef");
    ls.push_back( renderer->DoRenderTypeAndDeclarator( &type, id, starting_declarator_prod, Syntax::Production::TYPE_IN_DECLARATION, policy, MakeTreeNode<NonConst>() ) );
    return Join(ls);    
}

//////////////////////////// Record ///////////////////////////////

TreePtr<AccessSpec> Record::GetInitialAccess() const
{
	return MakeTreeNode<Public>();
}


Syntax::Production Record::GetMyProductionTerminal() const
{
	return Production::BARE_STMT_DECL;
}


string Record::GetRender( VN::RendererInterface *renderer, Production production, Policy policy ) 
{
	INDENT("R");
	list<string> ls;
	// For when we are a record-in-a-record i.e. member class
	if( policy.missing_access_to_public )
		Append( ls, ApplyAndRenderAccessSpec( MakeTreeNode<Public>(), false, renderer, policy ) );// see #877

	// For our members
	shared_ptr<Syntax> cur_access = GetInitialAccess();
	ASSERT( cur_access );		
	policy.cur_access = &cur_access;
	//ls.push_back("/* start record with " + Trace(*policy.cur_access) + "*/");

	Policy id_policy = policy;
	id_policy.resolve_identifier_scope = false; // Don't want scope resolution when declaring
		
	try
	{
		ls.push_back( GetKeyword() ); // class, struct etc
		ls.push_back( renderer->DoRender(&identifier, Production::PRIMARY_EXPR, id_policy) ); 
		string s = Join(ls);
		if( policy.force_incomplete_records )
			return s;

		s += RenderExtras(renderer, Production::SPACE_SEP_STMT_DECL, policy);	
		s += "\n";
		s += RenderBody(renderer, policy);
		return s;
	}
	catch( Refusal & )
	{
		// Catch the exception so that our modified policy propagates into the explicit render
		// TODO could we not put the modified policy into the exception object? We'd still have
		// to catch, but could throw again with updated policy.
		return renderer->RenderNodeExplicit(shared_from_this(), production, policy);
	}
	
}   


string Record::GetKeyword() const
{	
	// Short form of explicit has no VN parens and only serves to inject a 
	// node name into a syntax we already know how to render.
	return "⯁" + VN::Render::RenderNodeTypeName(shared_from_this());
}


string Record::RenderExtras(VN::RendererInterface *, Production, Policy)
{
	return ""; // Nothing extra by default
}


string Record::RenderBody( VN::RendererInterface *renderer, Policy policy )
{
	string s;
    Sequence<Declaration> sorted = SortDecls( members, true );

	// Members
	s += "{\n";

	Policy member_policy = policy;
    member_policy.split_bulky_statics = true; // Our scope is a record body
	member_policy.permit_static_keyword = true; // In a record body, static means global
	
    // Emit preprocs and an incomplete for each record 
    for( auto &d : sorted )
    {       
		//s += "/* in record with " + Trace(*member_policy.cur_access) + "*/";
		// Do this before checking access spec, so that the cur_access can be updated
		string rendered_member = renderer->DoRender( &d, Production::STMT_DECL, member_policy );			
        s += rendered_member;
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

string Enum::GetRender( VN::RendererInterface *, Production, Policy ) 
{
	throw TemporarilyDisabled(); // TODO implement enum render
}


string Enum::RenderBody( VN::RendererInterface *, Policy  )
{
	throw TemporarilyDisabled(); 
}

//////////////////////////// InheritanceRecord ///////////////////////////////

string InheritanceRecord::RenderExtras(VN::RendererInterface *renderer, Production, Policy policy)
{
	list<string> ls;
	for( auto &b : bases )
		ls.push_back( renderer->DoRender(&b, Production::BASE_CLASS_SPEC, policy ) );    	
	
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
	return s + (paren?"(":"") + renderer->DoRender( &*operands_it, Production::PROD, policy) + (paren?")":""); \
} \

#define POSTFIX(TOK, TEXT, NODE, BASE, CAT, PROD, ASSOC) \
Syntax::Production NODE::GetMyProductionTerminal() const \
{ \
	return Production::PROD; \
} \
string NODE::GetRender( VN::RendererInterface *renderer, Production, Policy policy ) \
{ \
	Sequence<Expression>::iterator operands_it = operands.begin(); \
	return renderer->DoRender( &*operands_it, Production::PROD, policy) + TEXT; \
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
	string s = renderer->DoRender( &*operands_it, prod_left, policy ); \
	s += TEXT; \
	++operands_it; \
	s += renderer->DoRender( &*operands_it, prod_right, policy ); \
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
	return renderer->DoRender( &condition, BoostPrecedence(Production::ASSIGN), policy ) + 
		   " ? " +
		   // Middle expression boots parser - so you can't split it up using (), [] etc
		   renderer->DoRender( &expr_then, Production::BOTTOM_EXPR, policy ) + 
		   " : " +
		   renderer->DoRender( &expr_else, Production::ASSIGN, policy );          
}


//////////////////////////// Subscript ///////////////////////////////

Syntax::Production Subscript::GetMyProductionTerminal() const
{ 
	return Production::POSTFIX; 
}

string Subscript::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->DoRender( &destination, Production::POSTFIX, policy ) + 
		   "[" +
		   renderer->DoRender( &index, Production::BOTTOM_EXPR, policy ) + 
		   "]";       
}

//////////////////////////// ArrayInitialiser ///////////////////////////////

Syntax::Production ArrayInitialiser::GetMyProductionTerminal() const
{ 
	// It looks like a Compound. When used as initialsier we want =
	return Production::PARENTHESISED; 
}


string ArrayInitialiser::GetRender( VN::RendererInterface *renderer, Production surround_prod, Policy policy )
{
	(void)surround_prod;
	// Allow render if in an assignment, which will be true in initialiser case due to accomodation
	if( surround_prod != Production::ASSIGN )
		throw RefuseDifficultSyntax(); 
		
	list<string> ls;    	
    for( auto &e : elements )
		ls.push_back( renderer->DoRender( &e, Production::COMMA_SEP, policy ) );

    return Join(ls, ", ", "{", "}"); 
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
	// We may need to convert the argumentation into a suitable form depending on policy.
	// If a conversion occurs, the callee is needed in order to transform the arguments.
	TreePtr<Argumentation> constructor_arg = constructor_argumentation->ConvertToSeqIfPolicyAllows(constructor_id, renderer, policy);

	return string (DynamicTreePtrCast<Global>(global) ? "::" : "") +
		   "new" + placement_argumentation->DirectRenderArgumentation(renderer, policy) +
		   " " +
		   renderer->DoRender( &type, Production::TYPE_IN_NEW, policy ) +
		   constructor_arg->DirectRenderArgumentation(renderer, policy);
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
		   " " + renderer->DoRender( &pointer, Production::PREFIX, policy );
}


//////////////////////////// Lookup ///////////////////////////////

Syntax::Production Lookup::GetMyProductionTerminal() const
{ 
	return Production::POSTFIX; 
}


string Lookup::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return renderer->DoRender(&object, Production::POSTFIX, policy) +
		   "." +
		   renderer->DoRender(&member, Production::PRIMARY_EXPR, policy);
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
		
    return "(" + renderer->DoRender( &type, Production::BOOT_TYPE, policy ) + ")" +
                 renderer->DoRender( &operand, Production::PREFIX, policy );
}

//////////////////////////// Call ///////////////////////////////

Syntax::Production Call::GetMyProductionTerminal() const
{
	return Production::POSTFIX; 	
}


string Call::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{				
	string s = renderer->DoRender( &callee, Production::POSTFIX, policy );
	
	// We may need to convert the argumentation into a suitable form depending on policy.
	// If a conversion occurs, the callee is needed in order to transform the arguments.
	TreePtr<Argumentation> arg = argumentation->ConvertToSeqIfPolicyAllows(callee, renderer, policy);
		
	// Let the SeqArgumentation node do the actual render
	s += arg->DirectRenderArgumentation(renderer, policy);    
	return s;
}

//////////////////////////// ConstructInitialiser ///////////////////////////////

Syntax::Production ConstructInitialiser::GetMyProductionTerminal() const
{
	// Suppress the = when used with an Instance eg GlobalScope globals( "GlobalScope" )
	return Production::DIRECT_INIT; 
}


string ConstructInitialiser::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{		
	if( !policy.detect_and_render_constructor )
		throw RefusedByPolicy(); // TODO find a way of disambiguating from a Call in VN lang, see MemberInitialiser for ideas				

	// We may need to convert the argumentation into a suitable form depending on policy.
	// If a conversion occurs, the callee is needed in order to transform the arguments.
	TreePtr<Argumentation> arg = argumentation->ConvertToSeqIfPolicyAllows(constructor_id, renderer, policy);
			
	// We never render the identifier for constructors - they are "invisible" and represent
	// the choice of which overload we are bound to.		
	return arg->DirectRenderArgumentation(renderer, policy);	
}

//////////////////////////// SizeOf ///////////////////////////////

Syntax::Production SizeOf::GetMyProductionTerminal() const
{ 
	return Production::PREFIX; 	
}


string SizeOf::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "sizeof(" + renderer->DoRender(&argument, Production::BOOT_TYPE, policy ) + ")";
}

//////////////////////////// AlignOf ///////////////////////////////

Syntax::Production AlignOf::GetMyProductionTerminal() const
{ 
	return Production::PREFIX; 
}


string AlignOf::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "alignof(" + renderer->DoRender(&argument, Production::BOOT_TYPE, policy ) + ")";
}

//////////////////////////// Compound ///////////////////////////////

Syntax::Production Compound::GetMyProductionTerminal() const
{ 
	return Production::COMPOUND;
}


string Compound::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	INDENT("C");
    string s = " { ";
 	policy.permit_static_keyword = true; // In a compound, static means global
	policy.cur_access = nullptr; // No access specs here
	
    for( auto &m : members )    
        s += renderer->DoRender( &m, Production::STMT_DECL, policy );    
    if( policy.compound_uses_vn_separator )
		s += "⚬";
    for( auto &st : statements )    
		s += renderer->DoRender( &st, Production::STMT_DECL_LOW, policy );    
    s += " } ";
    return s;
}

//////////////////////////// StatementExpression ///////////////////////////////

Syntax::Production StatementExpression::GetMyProductionTerminal() const
{ 
	return Production::PARENTHESISED; 
}


string StatementExpression::GetRender( VN::RendererInterface *renderer, Production production, Policy policy )
{
	INDENT("S");

    if( policy.refuse_statement_expression )
	{
		// If we can't render syntactially, call RenderNodeExplicit() directly so it
		// gets the updated policy.
		policy.permit_static_keyword = true; // In a compound, static means global
		policy.cur_access = nullptr; // No access specs here
		return renderer->RenderNodeExplicit(shared_from_this(), production, policy);
	}
	    
 	policy.permit_static_keyword = true; // In a compound, static means global
	policy.cur_access = nullptr; // No access specs here
      
    string s = "({ ";
	for( TreePtr<Declaration> m : members )    
		s += renderer->DoRender( &m, Syntax::Production::STMT_DECL, policy );       
	for( TreePtr<Statement> st : statements )    
		s += renderer->DoRender( &st, Syntax::Production::STMT_DECL_LOW, policy );    
	return s + " })";
}

//////////////////////////// Return ///////////////////////////////

Syntax::Production Return::GetMyProductionTerminal() const
{ 
	return Production::BARE_STMT_DECL; 	
}


string Return::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "return " + renderer->DoRender( &return_value, Production::SPACE_SEP_STMT_DECL, policy );
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
	
	string label = renderer->DoRender( &destination, prod, policy );
	
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
		       renderer->DoRender( &condition, Production::BOTTOM_EXPR, policy ) +
		       " )\n" +
		       renderer->DoRender( &body, has_else_clause ? Production::STMT_DECL_HIGH : Production::STMT_DECL_LOW, policy );
		       
    if( has_else_clause )  
        s += "else\n" + renderer->DoRender( &body_else, Production::STMT_DECL_LOW, policy);
        
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
		   renderer->DoRender( &condition, Production::BOTTOM_EXPR, policy ) +
		   " )\n" +
		   renderer->DoRender( &body, Production::STMT_DECL, policy );
}

//////////////////////////// Do ///////////////////////////////

Syntax::Production Do::GetMyProductionTerminal() const
{ 
	return Production::BARE_STMT_DECL; 
}


string Do::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "do\n" +
		   renderer->DoRender( &body, Production::STMT_DECL_LOW, policy ) +
		   "while( " +
		   renderer->DoRender( &condition, Production::BOTTOM_EXPR, policy ) +
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
		   renderer->DoRender( &initialisation, Production::BOTTOM_EXPR, policy ) +
		   "; " +
		   renderer->DoRender( &condition, Production::BOTTOM_EXPR, policy ) +
		   "; " +
		   renderer->DoRender( &increment, Production::BOTTOM_EXPR, policy ) +
		   " )\n" +
		   renderer->DoRender( &body, Production::STMT_DECL, policy );
}

//////////////////////////// Switch ///////////////////////////////

Syntax::Production Switch::GetMyProductionTerminal() const
{ 
	return Production::STMT_DECL; 
}


string Switch::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return "switch( " +
		   renderer->DoRender( &condition, Production::BOTTOM_EXPR, policy ) +
		   " )\n" +
		   renderer->DoRender( &body, Production::STMT_DECL, policy );
}

//////////////////////////// SwitchTarget ///////////////////////////////

Syntax::Production SwitchTarget::GetMyProductionTerminal() const
{ 
	return Production::LABEL; 
}

//////////////////////////// RangeCase //////////////////////////////

string RangeCase::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	// See LabelDeclaration::GetRender() about the ;
	return "case " + 
	       renderer->DoRender( &value_lo, Production::EXPR_CONST, policy) + 
	       ".." +
	       renderer->DoRender( &value_hi, Production::EXPR_CONST, policy) + 
	       ":" + ";";	
}

//////////////////////////// Case //////////////////////////////

string Case::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	// See LabelDeclaration::GetRender() about the ;
	return "case " + renderer->DoRender( &value, Production::EXPR_CONST, policy) + ":" + ";";	
}

//////////////////////////// Default //////////////////////////////

string Default::GetRender( VN::RendererInterface *, Production, Policy )
{
	// See LabelDeclaration::GetRender() about the ;
	return "default:" + string() + ";";	
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
	return Production::PRIMARY_EXPR; 
}

//////////////////////////// MacroField ///////////////////////////////

Syntax::Production MacroField::GetMyProduction(const VN::RendererInterface *, Policy) const
{ 
	if( !DynamicTreePtrCast<Expression>(initialiser) )
		return Production::STMT_DECL;
	else
		return Production::BARE_STMT_DECL;
}


string MacroField::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
    list<string> ls;

	if( policy.refuse_preprocessor )
		throw RefusedByPolicy();

	if( policy.missing_access_to_public )
	    Append( ls, ApplyAndRenderAccessSpec( MakeTreeNode<Public>(), false, renderer, policy ) ); // see #877

    // ---- Proto ----
	ls.push_back( renderer->DoRender( &identifier, Syntax::Production::POSTFIX, policy ) );
	list<string> renders;
	for( TreePtr<Node> node : arguments )
		renders.push_back( renderer->DoRender(&node, Syntax::Production::COMMA_SEP, policy) );
	ls.push_back( Join(renders, ", ", "(", ")") );
	
	// ---- Initialisation ----	    
    Append( ls, RenderMemberInits( renderer, policy ) ); 
	// Use DIRECT_INIT so accomodation maybe adds an = depending on the node
    if( !TreePtr<Uninitialised>::DynamicCast(initialiser) )
		ls.push_back( renderer->DoRender( &initialiser, Syntax::Production::DIRECT_INIT, policy ) );
	return Join( ls, " " );
}

//////////////////////////// MacroStatement ///////////////////////////////

Syntax::Production MacroStatement::GetMyProductionTerminal() const
{ 
	return Production::STMT_DECL; 
}


string MacroStatement::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	if( policy.refuse_preprocessor )
		throw RefusedByPolicy();

	string s = renderer->DoRender( &identifier, Syntax::Production::POSTFIX, policy );
	
    list<string> renders; 
    for( TreePtr<Node> node : arguments )
        renders.push_back( renderer->DoRender( &node, Syntax::Production::COMMA_SEP, policy) );
    s += Join(renders, ", ", "(", ");\n");
    return s;
}

//////////////////////////// PreProcDecl ///////////////////////////////

Syntax::Production PreProcDecl::GetMyProductionTerminal() const
{ 
	return Production::PRE_PROC_DIRECTIVE; 
}

string PreProcDecl::RenderPreProcDirective(string s, Policy policy) const
{
	if( policy.refuse_preprocessor )
		throw RefusedByPolicy();

	return "#" + s + "\n";
}

//////////////////////////// Include ///////////////////////////////

string Include::GetRender( VN::RendererInterface *renderer, Production, Policy policy )
{
	return RenderPreProcDirective("include "+CustomiseFilenameForInclude(filename, renderer, policy), policy);
}

//////////////////////////// SystemInclude ///////////////////////////////

string SystemInclude::CustomiseFilenameForInclude( TreePtr<String> name, VN::RendererInterface *, Policy  )
{
	return "<" + name->GetString() + ">"; 
}

//////////////////////////// LocalInclude ///////////////////////////////             

string LocalInclude::CustomiseFilenameForInclude( TreePtr<String> name, VN::RendererInterface *renderer, Policy policy )
{
	return renderer->DoRender(&name, Syntax::Production::SPACE_SEP_PRE_PROC, policy);
}

