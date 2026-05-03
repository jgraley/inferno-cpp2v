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
    trans_kit = TransKit{ utils.get() };
            
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

    unique_identifier_names = identifiers_uniqifier.UniquifyAll( trans_kit, context );
    
    Syntax::Policy top_policy = default_policy;
    
    nodes_not_rendered_to_c = 0;
    string s = DoRender( &root, Syntax::Production::PROGRAM, top_policy );
    if( nodes_not_rendered_to_c > 0 )
        s = SSPrintf("#error %d nodes could not be rendered to C\n", nodes_not_rendered_to_c) + s;
    return s;
}	


Syntax::Policy CppRender::GetDefaultPolicy()
{
	Syntax::Policy policy;
	
	// Insert braces to disambiguate stratements eg in case of if/else ambiguity
	policy.boot_statements_using_braces = true;
	
	// This can be necessary to resolve dependencies or at least produce readable code
	policy.can_split_instances = true;
		
	// Render constructors and destructors per C++ standard
	policy.use_vn_xstructor_symbol = false;
	
	// Just use type ids directly (aka abstract declarators)
	policy.disambiguate_type_id = false;

	// Allow invisible things to be rendered as empty strings
	policy.refuse_invisibles = false;

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
catch( Syntax::Refusal &ex ) 
{	
	return DispatchInternal( node, surround_prod, policy, ex );
}


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
        return r->identifier->GetRender(this, Syntax::Production::PRIMARY_EXPR, policy) + "::"; 
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


string CppRender::DispatchInternal( TreePtr<Node> node, Syntax::Production surround_prod, Syntax::Policy policy, Syntax::Refusal &ex )
{			
    if( auto make_rec = TreePtr<RecordInitialiser>::DynamicCast(node) )
        return RenderRecordInitialiser( make_rec, surround_prod, policy );
    else if( auto macro_stmt = TreePtr<MacroStatement>::DynamicCast(node) )
        return RenderMacroStatement( macro_stmt, surround_prod, policy );
    else if( auto si = TreePtr<SystemInclude>::DynamicCast(node) )
        return "#include <" + si->filename->GetString() + ">";
    else if( auto si = TreePtr<LocalInclude>::DynamicCast(node) )
        return "#include " + si->filename->GetRender(this, Syntax::Production::SPACE_SEP_PRE_PROC, policy);
    else if( TreePtr<Enum> enum_ = TreePtr<Enum>::DynamicCast(node) )
		return RenderEnum( enum_, policy );        	
    else if( auto t = TreePtr<Typedef>::DynamicCast(node) )
        return RenderTypedef( t, surround_prod, policy );
        
    // Due #969 we might have a standard agent, so fall back to a function that
    // definitely won't call any agent methods.
    nodes_not_rendered_to_c++;
    return ex.What()+RenderNodeExplicit( node, surround_prod, policy );      
}


string CppRender::RenderMacroStatement( TreePtr<MacroStatement> ms, Syntax::Production surround_prod, Syntax::Policy policy ) try
{
	(void)surround_prod;
	string s = DoRender( &ms->identifier, Syntax::Production::POSTFIX, policy );
	
    list<string> renders; // TODO duplicated code, factor out into RenderSeqMacroArgs()
    for( TreePtr<Node> node : ms->arguments )
        renders.push_back( DoRender( &node, Syntax::Production::COMMA_SEP, policy) );
    s += Join(renders, ", ", "(", ");\n");
    return s;
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
        ls.push_back( DoRender( &e, Syntax::Production::COMMA_SEP, policy ) );

    // Do the syntax
    s += "(" + DoRender( &make_rec->type, Syntax::Production::BOTTOM_EXPR, policy ) + ")"; 
    s += Join( ls, ", ", "{", "}" );   // Use of {} in expressions is irregular so handle locally 
    return s;
}
DEFAULT_CATCH_CLAUSE


string CppRender::RenderTypedef( TreePtr<Typedef> t, Syntax::Production surround_prod, Syntax::Policy policy ) try
{
	(void)surround_prod;
    list<string> ls;

	if( policy.missing_access_to_public )
	    Append( ls, t->ApplyAndRenderAccessSpec( MakeTreeNode<Public>(), false, this, policy ) ); // see #877

	Syntax::Policy id_policy = policy;
	id_policy.resolve_identifier_scope = false;
    Syntax::Production starting_declarator_prod = Syntax::Production::PRIMARY_EXPR;
	ls.push_back("typedef");
	string s = Join(ls);
	
    auto id = DoRender( &t->identifier, starting_declarator_prod, id_policy );
    ls.push_back( DoRenderTypeAndDeclarator( &(t->type), id, starting_declarator_prod, Syntax::Production::TYPE_IN_DECLARATION, policy ) );
    return Join(ls);    
}
DEFAULT_CATCH_CLAUSE



string CppRender::RenderEnum( TreePtr<CPPTree::Record> record, Syntax::Policy policy ) try
{
	Syntax::Policy id_policy = policy;
	id_policy.resolve_identifier_scope = false; // Don't want scope resolution when declaring
		
    string s = "enum";
    s += " ";    
    s += DoRender( &record->identifier, Syntax::Production::PRIMARY_EXPR, id_policy); 
	
	if( policy.force_incomplete_records )
		return s;
	
	list<string> ls;		
    for( TreePtr<Declaration> pe : record->members )
    {
		// TODO make a new Enumerator member, derived from Instance, and replace its GetRender()
        auto o = TreePtr<Instance>::DynamicCast(pe);
        if( !o )
        {
			nodes_not_rendered_to_c++;
			ls.push_back( RenderNodeExplicit( pe, Syntax::Production::COMMA_SEP, policy ) );  
			continue;
        }
        
        string s = DoRender( &o->identifier, Syntax::BoostPrecedence(Syntax::Production::ASSIGN), id_policy ); 
        
		// Use DIRECT_INIT so accomodation maybe adds an = depending on the node
        if( !TreePtr<Uninitialised>::DynamicCast(o->initialiser) )      
			s += " " + DoRender( &o->initialiser, Syntax::Production::DIRECT_INIT, policy );
			
		ls.push_back( s );			
    }
    
    return s + "\n" + Join( ls, ",\n", "{\n", "}\n" );
}
DEFAULT_CATCH_CLAUSE
                 
