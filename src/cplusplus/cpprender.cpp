#include "tree/cpptree.hpp"
#include "helpers/transformation.hpp"
#include "tree/typeof.hpp"
#include "common/trace.hpp"
#include "common/read_args.hpp"
#include "tree/type_data.hpp"
#include "helpers/walk.hpp"
#include "helpers/simple_duplicate.hpp"
#include "tree/misc.hpp"
#include "tree/scope.hpp"
#include "vn/lang/sort_decls.hpp"
#include "vn/agents/agent.hpp"
#include "clang/Parse/DeclSpec.h"
#include "vn/lang/uniquify_identifiers.hpp"
#include "cpprender.hpp"

using namespace std;
using namespace CPPTree;

// TODO indent back to previous level at end of string
#define ERROR_UNKNOWN(V) \
    ( string( "\n❌" ) + \
      string( V ) + \
      string( " not supported in " ) + \
      string( __func__ ) + \
      string( "()❌\n" ) )

#define ERROR_UNSUPPORTED(P) \
    ERROR_UNKNOWN( P ? TYPE_ID_NAME(*P) : "<nullptr>" )

// Don't like the layout of rendered code?
// There's a .clang-format in repo root.


// For #400 make methods that return strings try-functions
// and use this for the catch clause.
#define DEFAULT_CATCH_CLAUSE \
    catch( const ::Mismatch &e ) \
    { \
        return RenderMismatchException( __func__, e ); \
    } 


CppRender::CppRender( string of ) :
	Render( GetDefaultPolicy(), of ) // default policy when rendering to pure C++
{
}


string CppRender::RenderToString( TreePtr<Node> root )
{       
    // Context is used for various lookups but does not need
    // to be a Scope.
    context = root; 
        
    utils = make_unique<DefaultTransUtils>(context);
    using namespace placeholders;

    // Make the identifiers unique (does its own tree walk). Be strict about undeclared
    // identifiers - to rename them would be unsafe because we assume there's
    // a declaration ouside of our tree. This actually gets other nodes too but 
    // we'll only look up identifiers.
    UniquifyNames::Policy un_policy {
	    .name_getter = &Syntax::GetIdentifierName, 
		.include_single_parent = true,
		.include_multi_parent = true,
		.include_designation_named_identifiers = false,
		.preserve_undeclared_ids = true
	};
	UniquifyNames identifiers_uniqifier(un_policy); 
    trans_kit = TransKit{ utils.get() };
    unique_identifier_names = identifiers_uniqifier.UniquifyAll( trans_kit, context );
    
    Syntax::Policy top_policy = default_policy;
    return DoRender( root, Syntax::Production::PROGRAM, top_policy );
}	


Syntax::Policy CppRender::GetDefaultPolicy()
{
	Syntax::Policy policy;
	
	// Insert braces to disambiguate stratements eg in case of if/else ambiguity
	policy.boot_statements_using_braces = true;
	
	return policy;
}


Syntax::Production CppRender::GetNodeProduction( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy ) const
{
	(void)surround_prod;
	try
	{
		return node->GetMyProduction(this, policy);       
	}
	catch( Syntax::Refusal &r )
	{
		ASSERT(false)(node)(" unknown production");
	}
	ASSERTFAIL();
}


string CppRender::Dispatch( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy ) try 
{ 		
	return node->GetRender( this, surround_prod, policy );		
}
catch( Syntax::Refusal & ) 
{	
	return DispatchInternal( node, surround_prod, policy );
}


string CppRender::DispatchInternal( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy )
{			
    if( TreePtr<Uninitialised>::DynamicCast(node) )
        return string();  
    else if( auto literal = DynamicTreePtrCast< Literal >(node) )
        return RenderLiteral( literal, surround_prod, policy );
    else if( auto call = TreePtr<Call>::DynamicCast(node) )
        return RenderMapArgsCallAsSeqArg( call, surround_prod, policy ); // Still need this for map args resolution TODO raise/lower steps
    else if( auto make_rec = TreePtr<RecordInitialiser>::DynamicCast(node) )
        return RenderRecordInitialiser( make_rec, surround_prod, policy );
    else if( auto macro_decl = TreePtr<MacroDeclaration>::DynamicCast(node) )
        return RenderMacroDeclaration( macro_decl, surround_prod, policy );
    else if( auto macro_stmt = TreePtr<MacroStatement>::DynamicCast(node) )
        return RenderMacroStatement( macro_stmt, surround_prod, policy );
    else if( auto expression = TreePtr<Expression>::DynamicCast(node) ) // Expression is a kind of Statement
        return RenderExpression( expression, surround_prod, policy );
    else if( auto instance = TreePtr<Instance>::DynamicCast(node) )    // Instance is a kind of Statement and Declaration
        return RenderInstance( instance, surround_prod, policy ); 
    else if( auto ppd = TreePtr<PreProcDecl>::DynamicCast(node) )
        return RenderPreProcDecl(ppd, surround_prod, policy); 
    else if( auto declaration = TreePtr<Declaration>::DynamicCast(node) )
        return RenderDeclaration( declaration, surround_prod, policy );
        
    // Due #969 we might have a standard agent, so fall back to a function that
    // definitely won't call any agent methods.
    return RenderNodeExplicit( node, surround_prod, policy );      
}


string CppRender::RenderLiteral( TreePtr<Literal> sp, Syntax::Production surround_prod, Syntax::Policy policy ) try
{
	(void)surround_prod;
    return Sanitise( sp->GetRender(this, surround_prod, policy) );
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderScopeResolvingPrefix( TreePtr<Node> node, Syntax::Policy policy ) try
{
    TreePtr<Node> scope = TryGetScope(node);
              
    if( !scope )
        return ""; // either we're not in a scope or id is undeclared
    else if( DynamicTreePtrCast<CodeUnit>( scope ) )
        return "";
    else if( auto e = DynamicTreePtrCast<Enum>( scope ) ) // <- for enum
        return RenderScopeResolvingPrefix( e->identifier, policy );    // omit scope for the enum itself
    else if( auto r = DynamicTreePtrCast<Record>( scope ) ) // <- for class, struct, union
        return r->identifier->GetRender(this, Syntax::Production::RESOLVER, policy) + "::";
    else if( DynamicTreePtrCast<CallableParams>( scope ) ||  // <- this is for params
             DynamicTreePtrCast<Compound>( scope ) ||    // <- this is for locals in body
             DynamicTreePtrCast<StatementExpression>( scope ) )    // <- this is for locals in body
        return "";
    else
        return scope->GetTrace()+"::"; // unknown scope
}
DEFAULT_CATCH_CLAUSE


string CppRender::GetUniqueIdentifierName( TreePtr<Node> id ) const 
{
	ASSERT( unique_identifier_names.count(id) > 0 )
	      (id)
	      (" (\"%s\") missing from unique_identifier_names", id->GetIdentifierName().c_str() );
	return unique_identifier_names.at(id);
}


string CppRender::DispatchTypeAndDeclarator( TreePtr<Node> type, string declarator, 
                                             Syntax::Production declarator_prod, Syntax::Production surround_prod, Syntax::Policy policy,
                                             bool constant )
{ 		
	auto type_as_type = TreePtr<CPPTree::Type>::DynamicCast(type);
	ASSERT( type_as_type )(type)(" needs to be a Type"); // Must supply a type

	return type_as_type->GetRenderTypeAndDeclarator( this, declarator, declarator_prod, surround_prod, policy, constant );		
}


// Insert escapes into a string so it can be put in source code
// TODO use \n \r etc and let printable ascii through
string CppRender::Sanitise( string s ) try
{
    string o;
    for( string::size_type i=0; i<s.size(); i++ )
    {
        char c[10];
        if( s[i] < ' ' )
            o += SSPrintf( c, "\\x%02x", s[i] );
        else
            o += s[i];
    }
    return o;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderOperator( TreePtr<Operator> op, Syntax::Production surround_prod, Syntax::Policy policy ) try
{
    (void)surround_prod;
    ASSERT(op);
    
    string s;
    Sequence<Expression> operands;
    if( auto n = DynamicTreePtrCast< New >(op) )
    {
        ASSERTFAIL(); // Hold this spot for resolving the map args to the constructor
    }
    else
        return RenderNodeExplicit( op, surround_prod, policy );
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderMapArgsCallAsSeqArg( TreePtr<Call> call, Syntax::Production surround_prod, Syntax::Policy policy ) try
{
	(void)surround_prod;
	// Note: we need to operate on the call, so that we can use callee to find the declaration and 
	// resolve the map into a sequence.

	auto map_argumentation = TreePtr<MapArgumentation>::DynamicCast( call->argumentation );
	ASSERT( map_argumentation );

    // Render the expression that resolves to the function name unless this is
    // a constructor call in which case just the name of the thing being constructed.
    string s = DoRender( call->callee, Syntax::Production::POSTFIX, policy );

	// A map-args call isn't C++, so lower it to sequential args - requires the function type
    auto callee_type = TypeOf::instance.Get(trans_kit, call->callee).GetTreePtr();
	ASSERT( callee_type );
    
	// Convert f->params from Parameters to Declarations and settle on an arbitrary 
	// ordering. This needs to be the same on each visit with a given callee.
	Sequence<Declaration> decl_sequence;   
	if( auto f = TreePtr<CallableParams>::DynamicCast(callee_type) )  
		for( auto param : f->params )
			decl_sequence.push_back(param); 

	// Determine args sequence using param sequence
	auto sa = MakeTreeNode<SeqArgumentation>();
	sa->arguments = IdValuePair::SortMapById( map_argumentation->arguments, decl_sequence ); // TODO could absorb

	// Let the SeqArgumentation node do the actual render
	s += sa->DirectRenderArgumentation(this, policy);    return s;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderMacroStatement( TreePtr<MacroStatement> ms, Syntax::Production surround_prod, Syntax::Policy policy ) try
{
	(void)surround_prod;
	string s = DoRender( ms->identifier, Syntax::Production::POSTFIX, policy );
	
    list<string> renders; // TODO duplicated code, factor out into RenderSeqMacroArgs()
    for( TreePtr<Node> node : ms->arguments )
        renders.push_back( DoRender( node, Syntax::Production::COMMA_SEP, policy) );
    s += Join(renders, ", ", "(", ");\n");
    return s;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderExpression( TreePtr<Initialiser> expression, Syntax::Production surround_prod, Syntax::Policy policy ) try
{
    (void)surround_prod;
      
    if( auto ce = DynamicTreePtrCast< StatementExpression >(expression) )
    {
        string s = "({ ";
		for( TreePtr<Declaration> m : ce->members )    
			s += DoRender( m, Syntax::Production::STMT_DECL, policy );       
        for( TreePtr<Statement> st : ce->statements )    
            s += DoRender( st, Syntax::Production::STMT_DECL_LOW, policy );    
        return s + " })";
    }
    else
        return Render::Dispatch( expression, surround_prod, policy );

}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderRecordInitialiser( TreePtr<RecordInitialiser> make_rec, Syntax::Production surround_prod, Syntax::Policy policy ) try
{
	(void)surround_prod;
    string s;

    // Get the record
    TreePtr<TypeIdentifier> id = DynamicTreePtrCast<TypeIdentifier>(make_rec->type);
    ASSERT(id);
    
    // We have an interesting problem: there are two collections, but an ordering
    // is implied in the generalised case: the record type has a collection of 
    // fields, but they need to be sorted based on dependency order when rendering
    // the records (we also aim for repeatability here). This ordering must then
    // be applied to the Collection<IdValuePair> in order to get a match-up between sub-expressions
    // and fields. I think C++ side-steps this by diallowing the RecordInitialiser syntax
    // in classes where dependencies might matter.

    TreePtr<Record> r = TryGetRecordDeclaration(trans_kit, id).GetTreePtr();
    // Make sure we have the same ordering as when the record was rendered
    Sequence<Declaration> sorted_members = SortDecls( r->members, true );

    // Determine args sequence using param sequence
    Sequence<Expression> sub_expr_sequence = IdValuePair::SortMapById( make_rec->operands, sorted_members );
    
    // Render to strings
    list<string> ls;
    for( TreePtr<Expression> e : sub_expr_sequence )
        ls.push_back( DoRender( e, Syntax::Production::COMMA_SEP, policy ) );

    // Do the syntax
    s += "(" + DoRender( make_rec->type, Syntax::Production::BOTTOM_EXPR, policy ) + ")"; 
    s += Join( ls, ", ", "{", "}" );   // Use of {} in expressions is irregular so handle locally 
    return s;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderStorage( TreePtr<Instance> st, Syntax::Policy policy ) try
{
	// In VN, Static means in the C++ sense not the C sense
    if( DynamicTreePtrCast<Static>( st ) && policy.permit_static_keyword )
        return "static ";
    else if( DynamicTreePtrCast<LocalVariable>( st ) )
        return ""; // Assume automatic allocation is the default
    else if( DynamicTreePtrCast<Temporary>( st ) )
        return "/*temp*/ ";
    else if( TreePtr<Field> no = DynamicTreePtrCast<Field>( st ) )
    {
        TreePtr<Virtuality> v = no->virt;
        if( DynamicTreePtrCast<Virtual>( v ) )
            return "virtual ";
        else if( DynamicTreePtrCast<NonVirtual>( v ) )
            return "";
        else
            return ERROR_UNKNOWN("virtualness");
    }
    else
        return ""; // eg for Instance as a wildcard
}
DEFAULT_CATCH_CLAUSE


void CppRender::ExtractInits( Sequence<Statement> &body, 
                              Sequence<Statement> &inits, 
                              Sequence<Statement> &remainder )
{
    // Initialisers are just calls to the constructor embedded in the body. In Inferno,
    // we call a constructor by 
    for( TreePtr<Statement> s : body )
    {
        if( auto call = DynamicTreePtrCast< MembInitialisation >(s) ) 
			inits.push_back(s);
		else
		    remainder.push_back(s);
    }
}


string CppRender::RenderInstanceProto( TreePtr<Instance> o, Syntax::Production starting_declarator_prod, Syntax::Policy policy ) try
{
    string s;
    bool constant=false;

    if( TreePtr<Static> st = DynamicTreePtrCast<Static>(o) )
        if( DynamicTreePtrCast<Const>(st->constancy) )
            constant = true;
    if( TreePtr<Field> f = DynamicTreePtrCast<Field>(o) )
        if( DynamicTreePtrCast<Const>(f->constancy) )
            constant = true;
            
    string declarator = DoRender( o->identifier, starting_declarator_prod, policy );
    s += DoRenderTypeAndDeclarator( o->type, declarator, starting_declarator_prod, Syntax::Production::BARE_STMT_DECL, policy, constant );

    return s;
} 
DEFAULT_CATCH_CLAUSE  


string CppRender::RenderInitialisation( TreePtr<Initialiser> init, Syntax::Policy policy ) try
{
	string s;
	if( ReadArgs::use.count("c") )
		s += "/* RenderInitialisation(" + Trace(init) + ") */";
	if( auto stmt = DynamicTreePtrCast<Statement>(init) )
    {
        // Put the contents of the body into a Compound-like form even if there's only one
        // Statement there - this is because we will wrangle with them later
        Sequence<Statement> code;
        Collection<Declaration> members;
        if( TreePtr<Compound> comp = DynamicTreePtrCast<Compound>(stmt) )
        {
            members = comp->members;
            code = comp->statements;
        }
        else 
            code.push_back( stmt );

        // Seperate the statements into constructor initialisers and "other stuff"
        Sequence<Statement> inits;
        Sequence<Statement> remainder;
        ExtractInits( code, inits, remainder );

        // Render the constructor initialisers if there are any
        if( !inits.empty() )
        {
            s += " :\n";
            s += RenderConstructorInitList( inits, policy );

			// Render the other stuff as a Compound so we always get {} in all cases
			auto r = MakeTreeNode<Compound>();
			r->members = members;
			r->statements = remainder;		
			init = r;	
		}	
    }
	
    return s + DoRender( init, Syntax::Production::INITIALISER, policy); 
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderInstance( TreePtr<Instance> o, Syntax::Production surround_prod, Syntax::Policy policy )
{
    string s;
    
	if( policy.force_initialisation )
	{
		if( ReadArgs::use.count("c") )
			s += "/* force init */";   
		// Definition is out-of-line so skip the storage
		policy.force_initialisation = false; // stop at one level
		s += RenderInstanceProto( o, Syntax::Production::RESOLVER, policy );
		s += RenderInitialisation( o->initialiser, policy );	
		s += "\n";		
	}
	else 
	{		
		if( ReadArgs::use.count("c") )
			s += "/* no force init */";   
		s += RenderStorage( o, policy );
		s += RenderInstanceProto( o, Syntax::Production::PURE_IDENTIFIER, policy );
		if( ShouldSplitInstance(o, surround_prod, policy) )
		{
			if( ReadArgs::use.count("c") )
				s += "/* split */";   
			// Emit just a prototype now and request definition later
			// Split out the definition of the instance for rendering later at CodeUnit scope
			if( !TreePtr<Uninitialised>::DynamicCast(o->initialiser) )
			{
				ASSERT(policy.definitions); // Not under a node that can render definitions
				policy.definitions->push(o);
				FTRACE(o)((void *)&policy)("\n");
			}
		}		
		else
		{
			if( ReadArgs::use.count("c") )
				s += "/* no split */";   
			// Emit the whole lot in-line
			s += RenderInitialisation( o->initialiser, policy );							
		}
	}
	return s;
}


// Non-const static objects in records and functions 
// get split into a part that goes into the record (main line of rendering) and
// a part that goes separately (definitions get appended at end of code unit).
// Do all functions, since SortDecls() ignores function bodies for dep analysis
bool CppRender::ShouldSplitInstance( TreePtr<Instance> o, Syntax::Production , Syntax::Policy policy ) 
{
    if( DynamicTreePtrCast<Callable>( o->type ) )
    {
        // ----- functions -----
        if( auto smf = TreePtr<MacroDeclaration>::DynamicCast(o) )
            return false; // don't split these
            
        return true;
    }
    else
    {
        // ----- objects ------ 
        //if( surround_prod==Syntax::Production::STMT_DECL )
	//		return false;
			
        if( policy.split_bulky_statics )
		{
			// we're a field of a record
			if( TreePtr<Static> s = DynamicTreePtrCast<Static>(o) )
			{
				if( DynamicTreePtrCast<Const>(s->constancy) && DynamicTreePtrCast<Numeric>( o->type ) )
					return false;

				return true;                
			}
		}

        return false;
    }
}


string CppRender::RenderMacroDeclaration( TreePtr<MacroDeclaration> md, Syntax::Production surround_prod, Syntax::Policy policy )
{
	(void)surround_prod;	
    // ---- Proto ----
	string s = DoRender( md->identifier, Syntax::Production::POSTFIX, policy );
	list<string> renders;
	for( TreePtr<Node> node : md->arguments )
		renders.push_back( DoRender(node, Syntax::Production::COMMA_SEP, policy) );
	s += Join(renders, ", ", "(", ")");
	
	// ---- Initialisation ----	    
    return s + RenderInitialisation( md->initialiser, policy );
}


string CppRender::RenderEnumHead( TreePtr<Record> record, Syntax::Policy policy )
{
    string s = "enum";
    s += " ";
    s += DoRender( record->identifier, Syntax::Production::PURE_IDENTIFIER, policy); // Don't want scope resolution when declaring
    
    return s;
}


string CppRender::RenderPreProcDecl( TreePtr<PreProcDecl> ppd, Syntax::Production surround_prod, Syntax::Policy policy ) try
{
	(void)surround_prod;
    if( auto si = TreePtr<SystemInclude>::DynamicCast(ppd) )
        return "#include <" + si->filename->GetString() + ">";
    else if( auto si = TreePtr<LocalInclude>::DynamicCast(ppd) )
        return "#include " + si->filename->GetRender(this, Syntax::Production::SPACE_SEP_PRE_PROC, policy);
    else
        return RenderNodeExplicit( ppd, surround_prod, policy );     
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderDeclaration( TreePtr<Declaration> declaration, Syntax::Production surround_prod, Syntax::Policy policy ) try
{
    TRACE();
	(void)surround_prod;

    if( TreePtr<Typedef> t = DynamicTreePtrCast< Typedef >(declaration) )
    {
        Syntax::Production starting_declarator_prod = Syntax::Production::PURE_IDENTIFIER;
        auto id = DoRender( t->identifier, starting_declarator_prod, policy );
        return "typedef " + DoRenderTypeAndDeclarator( t->type, id, starting_declarator_prod, Syntax::Production::TYPE_IN_DECLARATION, policy );
    }
    else if( TreePtr<Enum> enum_ = DynamicTreePtrCast< Enum >(declaration) )
    {
		string s = RenderEnumHead( enum_, policy );        
		if( !policy.force_incomplete_records )
		{
			s += RenderEnumBody( enum_, policy );
			s = '\n' + s + '\n';
		}
		return s;		
    }
    else if( TreePtr<LabelDeclaration> l = DynamicTreePtrCast<LabelDeclaration>(declaration) )
        return DoRender( l->identifier, Syntax::Production::PURE_IDENTIFIER, policy) + ":"; 
    
    return Render::Dispatch( declaration, surround_prod, policy );
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderConstructorInitList( Sequence<Statement> spe, Syntax::Policy policy ) try
{
    list<string> ls; 
    for( TreePtr<Statement> st : spe )
        ls.push_back( "    " + DoRender( st, Syntax::Production::COMMA_SEP, policy ) );		
    return Join(ls, ",\n");
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderEnumBody( TreePtr<CPPTree::Record> record, Syntax::Policy policy ) try
{
    TRACE();   
    string s = "\n{\n";
    bool first = true;
    for( TreePtr<Declaration> pe : record->members )
    {
        if( !first )
            s += ",\n";
            
        auto o = TreePtr<Instance>::DynamicCast(pe);
        if( !o )
        {
            s += RenderNodeExplicit( pe, Syntax::Production::ASSIGN, policy );
            continue;
        }
        // We're really declaring the id, and don't want scope resolution
        s += DoRender( o->identifier, Syntax::Production::PURE_IDENTIFIER, policy ); 
        
        auto ei = TreePtr<Expression>::DynamicCast( o->initialiser );
        if( !ei )
        {
            s += RenderNodeExplicit( o->initialiser, Syntax::Production::INITIALISER, policy );
            continue;
        }       
        s += DoRender( ei, Syntax::Production::INITIALISER, policy );

        first = false;    
    }
    return s + "}";
}
DEFAULT_CATCH_CLAUSE
                 
